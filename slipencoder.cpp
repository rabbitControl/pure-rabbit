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

#include <vector>

#include <m_pd.h>

#include <rcp_slip.h>

#include "PdMaxUtils.h"

using namespace PdMaxUtils;

#ifdef __cplusplus
extern "C"{
#endif

typedef struct _slipencoder
{
    t_object x_obj;

    std::vector<char> m_data;

    t_outlet* list_out;

} t_slipencoder;

static t_class *slipencoder_class;



static void data_out(char data, void* user)
{
    if (user)
    {
        t_slipencoder* x = (t_slipencoder*)user;
        x->m_data.push_back(data);
    }
}


void slipencoder_list(t_slipencoder *x, t_symbol *s, int argc, t_atom *argv)
{
    std::vector<char> data(argc);
    int offset = 0;

    for (int i=0; i<argc; i++)
    {
        if (canBeInt(argv[i]))
        {
            int id = (int)getInt(argv[i]);
            if (id >= 0 &&
                id < 256)
            {
                data[i - offset] = id;
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

    x->m_data.clear();
    rcp_slip_encode(data.data(), argc - offset, data_out, x);

    // output data
    std::vector<t_atom> atoms(x->m_data.size());
    for (size_t i=0; i<x->m_data.size(); i++)
    {
        setInt(atoms[i], (unsigned char)x->m_data[i]);
    }

    outlet_list(x->list_out, NULL, atoms.size(), atoms.data());
}


void *slipencoder_new(t_symbol *s)
{
    t_slipencoder *x = (t_slipencoder *)pd_new(slipencoder_class);

    x->list_out = outlet_new(&x->x_obj, &s_list);

    return (void *)x;
}


void slipencoder_free(t_slipencoder *x)
{
    outlet_free(x->list_out);
}


void slipencoder_setup(void) {
    slipencoder_class = class_new(gensym("slipencoder"),
                           (t_newmethod)slipencoder_new,
                           (t_method)slipencoder_free,
                           sizeof(t_slipencoder),
                           CLASS_DEFAULT,
                           A_NULL,
                           0);


    class_addlist(slipencoder_class, (t_method)slipencoder_list);
}


#ifdef __cplusplus
} // extern "C"
#endif
