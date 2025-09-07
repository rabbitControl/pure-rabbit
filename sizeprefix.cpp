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

#if defined(__APPLE__)
#include <arpa/inet.h>
#elif defined(__linux__)
#include <endian.h>
#elif defined(_WIN32)
#include <winsock.h>
#endif

#include <m_pd.h>


#ifdef __cplusplus
extern "C"{
#endif


typedef struct _sizeprefix
{
    t_object x_obj;

    t_outlet* list_out;
    t_outlet* prefix_out;

} t_sizeprefix;

static t_class *sizeprefix_class;


void sizeprefix_list(t_sizeprefix *x, t_symbol *s, int argc, t_atom *argv)
{
    t_atom size_a[4];
    char size_c[4];

#if defined(__APPLE__)
    uint32_t n = htonl(argc);
#elif defined(__linux__)
    uint32_t n = htobe32(argc);
#elif defined(_WIN32)
    uint32_t n = htonl(argc);
#endif

    memcpy(size_c, &n, sizeof(uint32_t));

    for (int i=0; i<4; i++)
    {
#ifdef PD_MAJOR_VERSION
        size_a[i].a_type = A_FLOAT;
        size_a[i].a_w.w_float = (float)size_c[i];
#else
        size_a[i].a_type = A_INT;
        size_a[i].a_w.w_long = size_c[i];
#endif
    }

    outlet_list(x->prefix_out, NULL, 4, size_a);
    outlet_list(x->list_out, NULL, argc, argv);
}


void *sizeprefix_new(t_symbol *s)
{
    t_sizeprefix *x = (t_sizeprefix *)pd_new(sizeprefix_class);

    x->list_out = outlet_new(&x->x_obj, &s_list);
    x->prefix_out = outlet_new(&x->x_obj, &s_list);

    return (void *)x;
}


void sizeprefix_free(t_sizeprefix *x)
{
    outlet_free(x->list_out);
    outlet_free(x->prefix_out);
}


void sizeprefix_setup(void) {
    sizeprefix_class = class_new(gensym("sizeprefix"),
                                   (t_newmethod)sizeprefix_new,
                                   (t_method)sizeprefix_free,
                                   sizeof(t_sizeprefix),
                                   CLASS_DEFAULT,
                                   A_NULL,
                                   0);


    class_addlist(sizeprefix_class, (t_method)sizeprefix_list);
}


#ifdef __cplusplus
} // extern "C"
#endif
