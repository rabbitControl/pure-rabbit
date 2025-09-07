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

#include "rabbit.server.h"

#include <rcp_server.h>

#include "ParameterServer.h"
#include "PdMaxUtils.h"
#include "PdRcp.h"

using namespace std;
using namespace rcp;
using namespace PdMaxUtils;

#ifdef __cplusplus
extern "C"{
#endif

static t_class *rcp_server_pd_class;


// synchronized from threaded transporters

void pd_client_connected(t_pd *obj, void *data)
{
    if (obj != NULL)
    {
        t_rabbit_server_pd* x = (t_rabbit_server_pd*)obj;
        x->clients++;

        outlet_float(x->client_count_out, x->clients);
    }
}

void pd_client_disconnected(t_pd *obj, void *data)
{
    if (obj != NULL)
    {
        t_rabbit_server_pd* x = (t_rabbit_server_pd*)obj;

        x->clients--;

        if (x->clients < 0)
        {
            pd_error(x, "invalid client count");
            x->clients = 0;
        }

        outlet_float(x->client_count_out, x->clients);
    }
}


// pd interface

void rcpserver_bang(t_rabbit_server_pd *x)
{
    if (x->parameter_server)
    {
        x->parameter_server->bang();
    }
}

void rcpserver_list(t_rabbit_server_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->parameter_server)
    {
        x->parameter_server->list(argc, argv);
    }
}

void rcpserver_any(t_rabbit_server_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->parameter_server)
    {
        x->parameter_server->any(s, argc, argv);
    }
}

void rcpserver_listen(t_rabbit_server_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    if (!x->parameter_server)
    {
        return;
    }

    int port = 0;

    if (argc &&
        argv->a_type == A_FLOAT)
    {
        port = atom_getintarg(0, argc, argv);
    }

    x->parameter_server->listen(port);
}

void rcpserver_getport(t_rabbit_server_pd *x)
{
    if (x->parameter_server)
    {
        auto port = x->parameter_server->port();

        t_atom a;
        setInt(a, port);

        outlet_anything(x->info_out, gensym("port"), 1, &a);
    }
}

void rcpserver_expose_parameter(t_rabbit_server_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->parameter_server)
    {
        x->parameter_server->exposeParameter(argc, argv);
    }
}

void rcpserver_remove_parameter(t_rabbit_server_pd *x, float id)
{
    if (x->parameter_server)
    {
        x->parameter_server->removeParameter(int(id));
    }
}

void rcpserver_remove_parameter_sym(t_rabbit_server_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->parameter_server)
    {
        x->parameter_server->removeParameterList(argc, argv);
    }
}

//
void rcpserver_set_rabbithole(t_rabbit_server_pd *x, t_symbol* sym, int argc, t_atom *argv)
{
    if (!x->parameter_server)
    {
        return;
    }

    if (argc > 0)
    {
        if (argv[0].a_type == A_SYMBOL)
        {
            x->parameter_server->setRabbithole(argv[0].a_w.w_symbol->s_name);
        }
        else
        {
            pd_error(x, "Invalid argument for rabbithole");
        }
    }
    else
    {
        x->parameter_server->setRabbithole("");
    }
}

void rcpserver_set_rabbithole_interval(t_rabbit_server_pd *x, float interval)
{
    if (x->parameter_server)
    {
        x->parameter_server->setRabbitholeInterval(interval);
    }
}

void rcpserver_parameter_set_readonly(t_rabbit_server_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->parameter_server)
    {
        x->parameter_server->parameterSetReadonly(argc, argv);
    }
}

void rcpserver_parameter_set_order(t_rabbit_server_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->parameter_server)
    {
        x->parameter_server->parameterSetOrder(argc, argv);
    }
}

void rcpserver_parameter_set_min(t_rabbit_server_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->parameter_server)
    {
        x->parameter_server->parameterSetMin(argc, argv);
    }
}

void rcpserver_parameter_set_max(t_rabbit_server_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->parameter_server)
    {
        x->parameter_server->parameterSetMax(argc, argv);
    }
}

void rcpserver_parameter_set_minmax(t_rabbit_server_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->parameter_server)
    {
        x->parameter_server->parameterSetMinMax(argc, argv);
    }
}

void post_rcp_version(t_rabbit_server_pd *x)
{
    PdRcp::postRabbitcontrolInit();
    post("");
    PdRcp::postVersion();
}

// new

void *rcpserver_pd_new(t_symbol *s, int argc, t_atom *argv)
{
    t_rabbit_server_pd *x = (t_rabbit_server_pd *)pd_new(rcp_server_pd_class);

    x->parameter_server = new ParameterServer(x, argc, argv);

    return (void *)x;
}


void rcpserver_pd_free(t_rabbit_server_pd *x)
{
    if (x->parameter_server)
    {
        delete x->parameter_server;
    }
}


void setup_rabbit0x2eserver(void) {
    rcp_server_pd_class = class_new(gensym("rabbit.server"),
                                   (t_newmethod)rcpserver_pd_new,
                                   (t_method)rcpserver_pd_free,
                                   sizeof(t_rabbit_server_pd),
                                   CLASS_DEFAULT,
                                   A_GIMME,
                                   0);


    // parameter client/server
    class_addbang(rcp_server_pd_class, (t_method)rcpserver_bang);
    class_addlist(rcp_server_pd_class, (t_method)rcpserver_list);
    class_addanything(rcp_server_pd_class, (t_method)rcpserver_any);

    class_addmethod(rcp_server_pd_class, (t_method)post_rcp_version, gensym("getrcpversion"), A_NULL);


    // NOTE: getter are handled with inlet anything

    // parameter server
    class_addmethod(rcp_server_pd_class, (t_method)rcpserver_listen, gensym("listen"), A_GIMME, A_NULL);
    class_addmethod(rcp_server_pd_class, (t_method)rcpserver_getport, gensym("getport"), A_NULL);

    class_addmethod(rcp_server_pd_class, (t_method)rcpserver_expose_parameter, gensym("expose"), A_GIMME, A_NULL);
    class_addmethod(rcp_server_pd_class, (t_method)rcpserver_remove_parameter, gensym("remove"), A_FLOAT, A_NULL);
    class_addmethod(rcp_server_pd_class, (t_method)rcpserver_remove_parameter_sym, gensym("remove"), A_GIMME, A_NULL);

    class_addmethod(rcp_server_pd_class, (t_method)rcpserver_parameter_set_readonly, gensym("setreadonly"), A_GIMME, A_NULL);
    class_addmethod(rcp_server_pd_class, (t_method)rcpserver_parameter_set_order, gensym("setorder"), A_GIMME, A_NULL);
    class_addmethod(rcp_server_pd_class, (t_method)rcpserver_parameter_set_min, gensym("setmin"), A_GIMME, A_NULL);
    class_addmethod(rcp_server_pd_class, (t_method)rcpserver_parameter_set_max, gensym("setmax"), A_GIMME, A_NULL);
    class_addmethod(rcp_server_pd_class, (t_method)rcpserver_parameter_set_minmax, gensym("setminmax"), A_GIMME, A_NULL);

    // rabbithole
    class_addmethod(rcp_server_pd_class, (t_method)rcpserver_set_rabbithole, gensym("rabbithole"), A_GIMME, A_NULL);
    class_addmethod(rcp_server_pd_class, (t_method)rcpserver_set_rabbithole_interval, gensym("rabbithole_interval"), A_FLOAT, A_NULL);

    // raw
    // input is handled by inlet anything
}

#ifdef __cplusplus
} // extern "C"
#endif
