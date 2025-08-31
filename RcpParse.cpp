/*
********************************************************************
* rabbitcontrol - a protocol and data-format for remote control.
*
* https://rabbitcontrol.cc
* https://github.com/rabbitcontrol/rcp-pdmaxutils
*
* This file is part of rabbitcontrol for Pd and Max.
*
* Written by Ingo Randolf, 2021 - 2022
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, version 3.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*********************************************************************
*/

#include "RcpParse.h"

#include <vector>

#include <m_pd.h>

#include <rcp_packet.h>
#include <rcp_parameter.h>
#include <rcp_typedefinition.h>
#include <rcp_logging.h>

#include "PdMaxUtils.h"

using namespace PdMaxUtils;


RcpParse::RcpParse(t_rabbit_parse_pd* x)
    : m_x(x)
{
    m_x->parameter_out = outlet_new(&m_x->x_obj, &s_list);
    m_x->parameter_id_out = outlet_new(&m_x->x_obj, &s_float);
}

RcpParse::~RcpParse()
{
    outlet_free(m_x->parameter_out);
    outlet_free(m_x->parameter_id_out);
}

void RcpParse::handleList(int argc, t_atom* argv)
{
    std::vector<char> data(argc);

    for (int i=0; i<argc; i++)
    {
        if (canBeInt(argv[i]))
        {
            unsigned int di = getInt(argv[i]);
            if (di > 255)
            {
                pd_error(m_x, "invalid data in packet");
                return;
            }

            data[i] = (char)di;
        }
        else
        {
            pd_error(m_x, "malformed data");
            return;
        }
    }

    rcp_packet* packet = NULL;
    size_t data_size = argc;
    const char* data_p = data.data();

    while (data_p != NULL
           && data_size > 0)
    {
        data_p = rcp_packet_parse(data_p, data_size, &packet, &data_size);
        if (data_p && packet)
        {
            rcp_packet_command command = rcp_packet_get_command(packet);
            switch (command)
            {
            case COMMAND_INFO:
            {
                // NOTE: packet owns infodata - no transfer of ownership
                rcp_infodata* info_data = rcp_packet_get_infodata(packet);

                if (info_data)
                {
                    const char* version = rcp_infodata_get_version(info_data);
                    const char* app_id = rcp_infodata_get_application_id(info_data);

                    // info version (app)
                    int len = 2 + (app_id != NULL ? 1 : 0);
                    std::vector<t_atom> list(len);

                    setSymbol(list[0], gensym("info"));
                    setSymbol(list[1], gensym(version));

                    if (app_id != NULL)
                    {
                        setSymbol(list[2], gensym(app_id));
                    }

                    outlet_list(m_x->parameter_out, NULL, len, list.data());
                }
                else
                {
                    t_atom list[1];
                    setSymbol(list[0], gensym("info"));
                    outlet_list(m_x->parameter_out, NULL, 1, list);
                }

                break;
            }

            case COMMAND_INITIALIZE:
            {
                int16_t id = rcp_packet_get_iddata(packet);
                outputList("initialize", id);
                break;
            }

            case COMMAND_DISCOVER:
            {
                int16_t id = rcp_packet_get_iddata(packet);
                outputList("discover", id);
                break;
            }

            case COMMAND_UPDATE:
            case COMMAND_UPDATEVALUE:
            {
                // update parameter
                // NOTE: packet owns infodata - no transfer of ownership
                rcp_parameter* param = rcp_packet_get_parameter(packet);
                if (param)
                {
                    parameterUpdate(param);
                }
                break;
            }

            case COMMAND_REMOVE:
            {
                int16_t id = rcp_packet_get_iddata(packet);
                outputList("remove", id);
                break;
            }

            case COMMAND_INVALID:
            case COMMAND_MAX_:
                // nop
                break;
            }

            //
            rcp_packet_free(packet);
            packet = NULL;
        }
    }


}

void RcpParse::outputList(const char* str, int16_t id)
{
    // discover
    t_atom list[1];
    setSymbol(list[0], gensym(str));

    outlet_float(m_x->parameter_id_out, id);
    outlet_list(m_x->parameter_out, NULL, 1, list);
}

void RcpParse::parameterUpdate(rcp_parameter* parameter)
{
    const char* label = rcp_parameter_get_label(parameter);
    int16_t id = rcp_parameter_get_id(parameter);
    rcp_datatype type = rcp_typedefinition_get_type_id(rcp_parameter_get_typedefinition(parameter));

    // output
    // list update label value

    int len = 3 + (label != NULL ? 1 : 0);
    std::vector<t_atom> list(len);

    int i=0;
    setSymbol(list[i], gensym("update"));
    i++;

    if (label)
    {
        setSymbol(list[i], gensym(label));
        i++;
    }

    if (type == DATATYPE_BOOLEAN)
    {
        setInt(list[i], rcp_parameter_get_value_bool(RCP_VALUE_PARAMETER(parameter)) ? 1 : 0);
        i++;
    }
    else if (type == DATATYPE_INT32)
    {
        setInt(list[i], rcp_parameter_get_value_int32(RCP_VALUE_PARAMETER(parameter)));
        i++;
    }
    else if (type == DATATYPE_FLOAT32)
    {
        setFloat(list[i], rcp_parameter_get_value_float(RCP_VALUE_PARAMETER(parameter)));
        i++;
    }
    else if (type == DATATYPE_STRING)
    {
        setString(list[i], rcp_parameter_get_value_string(RCP_VALUE_PARAMETER(parameter)));
        i++;
    }

    outlet_float(m_x->parameter_id_out, id);
    outlet_list(m_x->parameter_out, NULL, i, list.data());
}

