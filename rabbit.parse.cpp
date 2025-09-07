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

#include "rabbit.parse.h"

#include "RcpParse.h"

#ifdef __cplusplus
extern "C"{
#endif

static t_class *rcp_parse_class;


void rcpparse_list(t_rabbit_parse_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->parse)
    {
        x->parse->handleList(argc, argv);
    }
}


void *rcpparse_pd_new(t_symbol *s)
{
    t_rabbit_parse_pd *x = (t_rabbit_parse_pd *)pd_new(rcp_parse_class);

    x->parse = new RcpParse(x);

    return (void *)x;
}


void rcpparse_pd_free(t_rabbit_parse_pd *x)
{
    if (x->parse)
    {
        delete x->parse;
    }
}


void setup_rabbit0x2eparse(void) {
    rcp_parse_class = class_new(gensym("rabbit.parse"),
                                   (t_newmethod)rcpparse_pd_new,
                                   (t_method)rcpparse_pd_free,
                                   sizeof(t_rabbit_parse_pd),
                                   CLASS_DEFAULT,
                                   A_NULL,
                                   0);


    class_addlist(rcp_parse_class, (t_method)rcpparse_list);
}

#ifdef __cplusplus
} // extern "C"
#endif
