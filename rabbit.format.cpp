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

#include "rabbit.format.h"

#include "RcpFormat.h"
#include "PdMaxUtils.h"


using namespace PdMaxUtils;

#ifdef __cplusplus
extern "C"{
#endif

static t_class *rcp_format_class;


void rcpformat_bang(t_rabbit_format_pd *x)
{
    if (x->format)
    {
        x->format->handleBang();
    }
}


void rcpformat_float(t_rabbit_format_pd *x, float f)
{
    if (x->format)
    {
        x->format->handleFloat(f);
    }
}


void rcpformat_sym(t_rabbit_format_pd *x, t_symbol *s)
{
    if (x->format)
    {
        x->format->handleSymbol(s);
    }
}


void rcpformat_list(t_rabbit_format_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->format)
    {
    }
}


void rcpformat_set_id(t_rabbit_format_pd *x, float f)
{
    if (x->format)
    {
        x->format->setId(int(f));
    }
}


void rcpformat_set_type(t_rabbit_format_pd *x, t_symbol* s)
{
    if (x->format)
    {
        x->format->setType(s);
    }
}


void rcpformat_get_type(t_rabbit_format_pd *x)
{
    if (x->format)
    {
        const t_symbol* s = x->format->getType();

        t_atom a;
        setSymbol(a, s);

        outlet_anything(x->info_out, gensym("type"), 1, &a);
    }
}


void rcpformat_set_label(t_rabbit_format_pd *x, t_symbol* s)
{
    if (x->format)
    {
        x->format->setLabel(s);
    }
}


void rcpformat_get_label(t_rabbit_format_pd *x)
{
    if (x->format)
    {
        const t_symbol* s = x->format->getLabel();

        t_atom a;
        setSymbol(a, s);

        outlet_anything(x->info_out, gensym("label"), 1, &a);
    }
}


void rcpformat_clear_label(t_rabbit_format_pd *x)
{
    if (x->format)
    {
        x->format->clearLabel();
    }
}


void *rcpformat_pd_new(t_symbol *s, int argc, t_atom *argv)
{
    t_rabbit_format_pd *x = (t_rabbit_format_pd *)pd_new(rcp_format_class);

    x->format = new RcpFormat(x, argc, argv);

    return (void *)x;
}


void rcpformat_pd_free(t_rabbit_format_pd *x)
{
    if (x->format)
    {
        delete x->format;
    }
}


void setup_rabbit0x2eformat(void) {
    rcp_format_class = class_new(gensym("rabbit.format"),
                                   (t_newmethod)rcpformat_pd_new,
                                   (t_method)rcpformat_pd_free,
                                   sizeof(t_rabbit_format_pd),
                                   CLASS_DEFAULT,
                                   A_GIMME,
                                   0);


    class_addbang(rcp_format_class, (t_method)rcpformat_bang);
    class_addfloat(rcp_format_class, (t_method)rcpformat_float);
    class_addsymbol(rcp_format_class, (t_method)rcpformat_sym);
    class_addlist(rcp_format_class, (t_method)rcpformat_list);
    // class_addanything(rcp_format_class, (t_method)rcpformat_any);

    //
    class_addmethod(rcp_format_class, (t_method)rcpformat_set_id, gensym("set"), A_FLOAT, A_NULL);

    // type
    class_addmethod(rcp_format_class, (t_method)rcpformat_set_type, gensym("type"), A_SYMBOL, A_NULL);
    class_addmethod(rcp_format_class, (t_method)rcpformat_get_type, gensym("gettype"), A_NULL, A_NULL);

    // label
    class_addmethod(rcp_format_class, (t_method)rcpformat_set_label, gensym("label"), A_SYMBOL, A_NULL);
    class_addmethod(rcp_format_class, (t_method)rcpformat_get_label, gensym("getlabel"), A_NULL, A_NULL);
    class_addmethod(rcp_format_class, (t_method)rcpformat_clear_label, gensym("clearlabel"), A_NULL, A_NULL);
}

#ifdef __cplusplus
} // extern "C"
#endif
