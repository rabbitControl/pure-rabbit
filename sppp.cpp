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

#include <string.h>

#include <m_pd.h>
#include <rcp_sppp.h>

#include "PdMaxUtils.h"

using namespace PdMaxUtils;

#ifdef __cplusplus
extern "C"{
#endif


typedef struct _sppp
{
    t_object x_obj;

    rcp_sppp* parser;

    t_outlet* list_out;

} t_sppp;

static t_class *sppp_class;



static void packet_cb(const char* data, size_t data_size, void* user)
{
    if (user == NULL)
    {
        return;
    }

    t_sppp* x = (t_sppp*)user;

    t_atom* atoms = new t_atom[data_size];

    for (size_t i=0; i<data_size; i++)
    {
        setInt(atoms[i], data[i]);
    }

    outlet_list(x->list_out, NULL, data_size, atoms);

    delete [] atoms;
}


void sppp_float(t_sppp *x, float f)
{
    int data = (int)f;
    if (data < 256 && data >= 0)
    {
        char d = (char)data;
        rcp_sppp_data(x->parser, &d, 1);
    }
}


void sppp_list(t_sppp *x, t_symbol *s, int argc, t_atom *argv)
{
    char* data = new char[argc];
    int offset = 0;

    for (int i=0; i<argc; i++)
    {
        if (canBeInt(argv[i]))
        {
            int id = (int)getAInt(argv[i], -1);
            if (id >= 0 &&
                id < 256)
            {
                data[i-offset] = id;
            }
            else
            {
                offset++;
            }
        }
        else
        {
            offset++;
        }
    }

    rcp_sppp_data(x->parser, data, argc - offset);

    delete [] data;
}


void sppp_reset(t_sppp* x)
{
    rcp_sppp_reset(x->parser);
}


void *sppp_new(t_symbol *s, int argc, t_atom *argv)
{
    t_sppp *x = (t_sppp *)pd_new(sppp_class);

    x->list_out = outlet_new(&x->x_obj, &s_list);


    int buffer_size = 1024;

    for (int i=0; i<argc; i++)
    {
        if (canBeInt(argv[i]))
        {
            buffer_size = getAInt(argv[i], 1024);
        }
    }

    if (buffer_size <= 0)
    {
        pd_error(x, "please provide a valid buffersize");
        buffer_size = 1024;
    }

    // create parser
    x->parser = rcp_sppp_create(buffer_size, packet_cb, x);


    return (void *)x;
}


void sppp_free(t_sppp *x)
{
    if (x->parser)
    {
        rcp_sppp_free(x->parser);
    }

    outlet_free(x->list_out);
}


void sppp_setup(void) {
    sppp_class = class_new(gensym("sppp"),
                           (t_newmethod)sppp_new,
                           (t_method)sppp_free,
                           sizeof(t_sppp),
                           CLASS_DEFAULT,
                           A_GIMME,
                           0);


    class_addfloat(sppp_class, (t_method)sppp_float);
    class_addlist(sppp_class, (t_method)sppp_list);

    class_addmethod(sppp_class, (t_method)sppp_reset, gensym("reset"), A_NULL, A_NULL);
}


#ifdef __cplusplus
} // extern "C"
#endif
