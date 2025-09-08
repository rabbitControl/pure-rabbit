/*
********************************************************************
* rabbitcontrol - a protocol and data-format for remote control.
*
* https://rabbitcontrol.cc
* https://github.com/rabbitControl/pure-rabbit
*
* This file is part of rabbitcontrol for Pd and Max.
*
* Written by Ingo Randolf, 2025
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

#include "ParameterClient.h"

#include <rcp_parameter.h>
#include <rcp_typedefinition.h>
#include <rcp_logging.h>

#include <m_pd.h>

#include "PdMaxUtils.h"
#include "PdClientTransporter.h"
#include "WebsocketClientTransporter.h"
#include "Threading.h"

using namespace PdMaxUtils;

static void client_parameter_added_cb(rcp_parameter* parameter, void* user)
{
    if (user)
    {
        rcp::ParameterClient* client = static_cast<rcp::ParameterClient*>(user);
        client->parameterAddedThreaded(parameter);
    }
}

static void client_parameter_removed_cb(rcp_parameter* parameter, void* user)
{
    if (user)
    {
        rcp::ParameterClient* client = static_cast<rcp::ParameterClient*>(user);
        client->parameterRemovedThreaded(parameter);
    }
}

// parameter updates
static void parameterValueUpdatedCb(rcp_value_parameter* parameter, void* user)
{
    if (user)
    {
        rcp::ParameterClient* client = static_cast<rcp::ParameterClient*>(user);
        client->parameterUpdate(RCP_PARAMETER(parameter));
    }
}
static void bangCb(rcp_bang_parameter* param, void* user)
{
    if (user)
    {
        rcp::ParameterClient* client = static_cast<rcp::ParameterClient*>(user);
        client->parameterUpdate(RCP_PARAMETER(param));
    }
}


// synchronized from threaded transporter

static void pd_id_parameter_output(t_pd *obj, void *data)
{
    std::vector<t_atom>* _list = (std::vector<t_atom>*)data;

    if (obj != NULL)
    {
        t_rabbit_client_pd* x = (t_rabbit_client_pd*)obj;

        if (_list &&
            _list->size() > 1 &&
            (_list->data()+1)->a_type == A_SYMBOL)
        {
            outlet_float(x->parameter_id_out, (*_list)[0].a_w.w_float);
            outlet_anything(x->parameter_out, (_list->data()+1)->a_w.w_symbol, _list->size()-2, _list->data()+2);
        }
    }

    if (_list)
    {
        delete _list;
    }
}


namespace rcp
{

ParameterClient::ParameterClient(t_rabbit_client_pd* x, int argc, t_atom *argv)
    : ParameterServerClientBase(x)
    , m_x(x)
{
    // init pd struct
    m_x->parameter_out = outlet_new(&m_x->x_obj, &s_list);
    m_x->parameter_id_out = outlet_new(&m_x->x_obj, &s_float);
    m_x->client_connected_out = outlet_new(&m_x->x_obj, &s_float);
    m_x->info_out = outlet_new(&m_x->x_obj, &s_list);

    setOutlets(m_x->parameter_out,
               m_x->parameter_id_out,
               m_x->info_out);


    // check arguments
    for (int i = 0; i < argc; ++i)
    {
        if (argv[i].a_type == A_SYMBOL)
        {
            if (strcmp(argv[i].a_w.w_symbol->s_name, "-raw") == 0)
            {
                m_raw = true;
            }

            // other arguments?
        }
    }

    if (m_raw)
    {
        m_x->raw_in = inlet_new(&m_x->x_obj, &m_x->x_obj.ob_pd, &s_list, gensym("__raw_input"));
        m_x->raw_out = outlet_new(&m_x->x_obj, &s_list);

        setRawOutlet(m_x->raw_out);

        m_transporter = new PdClientTransporter(this);
    }
    else
    {
        m_transporter = new WebsocketClientTransporter((t_pd*)x);
    }

    if (!m_transporter)
    {
        throw std::runtime_error("could not create rcp client transporter");
    }

    // create client
    m_client = rcp_client_create(m_transporter->transporter());

    if (m_client == nullptr)
    {
        throw std::runtime_error("could not create rcp client");
    }

    rcp_client_set_id(m_client, "pd rcp client");

    rcp_client_set_user(m_client, this);
    rcp_client_set_parameter_added_cb(m_client, client_parameter_added_cb);
    rcp_client_set_parameter_removed_cb(m_client, client_parameter_removed_cb);

    m_manager = rcp_client_get_manager(m_client);
}

ParameterClient::~ParameterClient()
{
    if (m_client)
    {
        rcp_client_free(m_client);
        m_client = nullptr;
    }

    if (m_transporter)
    {
        m_transporter->disconnect();
        delete m_transporter;
        m_transporter = nullptr;
    }

    // cleanup pd struct
    outlet_free(m_x->parameter_out);
    outlet_free(m_x->parameter_id_out);
    outlet_free(m_x->client_connected_out);
    outlet_free(m_x->info_out);

    if (m_x->raw_out)
    {
        outlet_free(m_x->raw_out);
    }

    if (m_x->raw_in)
    {
        inlet_free(m_x->raw_in);
    }
}

void ParameterClient::connect(string url)
{
    if (m_transporter)
    {
        std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

        m_transporter->connect(url);
    }
}

void ParameterClient::disconnect()
{
    if (m_transporter)
    {
        std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

        m_transporter->disconnect();
    }
}


// threaded - called from transporter thread
void ParameterClient::parameterAddedThreaded(rcp_parameter* parameter)
{
    const char* label = rcp_parameter_get_label(parameter);
    uint16_t id = rcp_parameter_get_id(parameter);

    // get the parents
    std::vector<std::string> groups = getParents(parameter);

    // cout << "add - parents: " << groups.size() << "\n";

    // output [list]
    // add group1 groupN... label value

    // TODO: append userid?

    int len = 1 + 3 + groups.size();
    std::vector<t_atom>* _list = new std::vector<t_atom>(len);
    std::vector<t_atom>& list = *_list;

    int i=0;

    setFloat(list[i], id);
    i++;

    setSymbol(list[i], gensym("add"));
    i++;

    for (std::vector<std::string>::reverse_iterator rit = groups.rbegin();
         rit != groups.rend(); ++rit)
    {
        setString(list[i], rit->c_str());
        i++;
    }

    setSymbol(list[i], gensym(label != NULL ? label : "<nolabel>"));
    i++;


    // set this as user
    rcp_parameter_set_user(parameter, this);

    // get type
    rcp_datatype type = rcp_typedefinition_get_type_id(rcp_parameter_get_typedefinition(parameter));

    if (rcp_parameter_is_value(parameter))
    {
        //
        rcp_parameter_set_value_updated_cb(RCP_VALUE_PARAMETER(parameter), parameterValueUpdatedCb);

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
            const char* value = rcp_parameter_get_value_string(RCP_VALUE_PARAMETER(parameter));
            setString(list[i], (value != NULL ? value : ""));
            i++;
        }
    }
    else if (type == DATATYPE_BANG)
    {
        rcp_bang_parameter_set_bang_cb(RCP_BANG_PARAMETER(parameter), bangCb);

        //
        setSymbol(list[i], gensym("[bang]"));
        i++;
    }
    else if (type == DATATYPE_GROUP)
    {
        //
        setSymbol(list[i], gensym("[group]"));
        i++;
    }

    // output list
    pd_queue_mess(&pd_maininstance, (t_pd*)m_x, _list, pd_id_parameter_output);
}

void ParameterClient::parameterRemovedThreaded(rcp_parameter* parameter)
{
    std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

    const char* label = rcp_parameter_get_label(parameter);
    uint16_t id = rcp_parameter_get_id(parameter);

    // get the parents
    std::vector<std::string> groups = getParents(parameter);

    // output [list]
    // remove group1 groupN... label

    int len = 1 + 2 + groups.size();
    std::vector<t_atom>* _list = new std::vector<t_atom>(len);
    std::vector<t_atom>& list = *_list;

    int i=0;

    setFloat(list[i], id);
    i++;

    setSymbol(list[i], gensym("remove"));
    i++;

    for (std::vector<std::string>::reverse_iterator rit = groups.rbegin();
         rit != groups.rend(); ++rit)
    {
        setString(list[i], rit->c_str());
        i++;
    }

    setSymbol(list[i], gensym(label != NULL ? label : "<nolabel>"));
    i++;

    // output list
    pd_queue_mess(&pd_maininstance, (t_pd*)m_x, _list, pd_id_parameter_output);
}

void ParameterClient::outputIdParameterList(std::vector<t_atom>* list)
{
    // output list
    pd_queue_mess(&pd_maininstance, (t_pd*)m_x, list, pd_id_parameter_output);
}

void ParameterClient::handleRawData(char* data, size_t size)
{
    if (m_transporter)
    {
        m_transporter->pushData(data, size);
    }
}

} // namespace rcp

