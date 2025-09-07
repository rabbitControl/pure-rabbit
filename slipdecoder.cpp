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

#include <stdbool.h>

#include <m_pd.h>

#include <rcp_slip.h>

#include "PdMaxUtils.h"

using namespace PdMaxUtils;


#ifdef __cplusplus
extern "C"{
#endif

typedef struct _slipdecoder
{
    t_object x_obj;

    rcp_slip* slip;

    t_outlet* list_out;

} t_slipdecoder;

static t_class *slipdecoder_class;


static void packet_cb(char* data, size_t data_size, void* user)
{
    if (user == NULL)
    {
        return;
    }

    t_slipdecoder* x = (t_slipdecoder*)user;

    t_atom* atoms = new t_atom[data_size];

    for (size_t i=0; i<data_size; i++)
    {
        setInt(atoms[i], (unsigned char)data[i]);
    }

    outlet_list(x->list_out, NULL, data_size, atoms);

    delete [] atoms;
}


void slipdecoder_float(t_slipdecoder *x, float f)
{
    int data = (int)f;
    if (data < 256 && data >= 0)
    {
        char d = (char)data;
        rcp_slip_append(x->slip, d);
    }
}


void slipdecoder_list(t_slipdecoder *x, t_symbol *s, int argc, t_atom *argv)
{
    for (int i=0; i<argc; i++)
    {
        if (canBeInt(argv[i]))
        {
            int id = getAInt(argv[i], -1);
            if (id >= 0 &&
                id < 256)
            {
                rcp_slip_append(x->slip, (char)id);
            }
        }
    }
}


void *slipdecoder_new(t_symbol *s, int argc, t_atom *argv)
{
    t_slipdecoder *x = (t_slipdecoder *)pd_new(slipdecoder_class);

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
    x->slip = rcp_slip_create(buffer_size);
    if (x->slip)
    {
        rcp_slip_set_user(x->slip, x);
        rcp_slip_set_packet_cb(x->slip, packet_cb);
    }

    return (void *)x;
}


void slipdecoder_free(t_slipdecoder *x)
{
    if (x->slip)
    {
        rcp_slip_free(x->slip);
    }

    outlet_free(x->list_out);
}


void slipdecoder_setup(void) {
    slipdecoder_class = class_new(gensym("slipdecoder"),
                           (t_newmethod)slipdecoder_new,
                           (t_method)slipdecoder_free,
                           sizeof(t_slipdecoder),
                           CLASS_DEFAULT,
                           A_GIMME,
                           0);


    class_addfloat(slipdecoder_class, (t_method)slipdecoder_float);
    class_addlist(slipdecoder_class, (t_method)slipdecoder_list);
}


#ifdef __cplusplus
} // extern "C"
#endif
