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

#include "rabbit.client.h"

#include <rcp_client.h>

#include "ParameterClient.h"
#include "PdRcp.h"

using namespace std;
using namespace rcp;

#ifdef __cplusplus
extern "C"{
#endif

static t_class *rcp_client_pd_class;


void rcpclient_bang(t_rabbit_client_pd *x)
{
    if (x->parameter_client)
    {
        x->parameter_client->bang();
    }
}

void rcpclient_list(t_rabbit_client_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->parameter_client)
    {
        x->parameter_client->list(argc, argv);
    }
}

void rcpclient_any(t_rabbit_client_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->parameter_client)
    {
        x->parameter_client->any(s, argc, argv);
    }
}


void rcpclient_connect(t_rabbit_client_pd *x, t_symbol *s)
{
    if (x->parameter_client)
    {
        x->parameter_client->connect(s->s_name);
    }
}

void rcpclient_disconnect(t_rabbit_client_pd *x)
{
    if (x->parameter_client)
    {
        x->parameter_client->disconnect();
    }
}

void post_rcp_version(t_rabbit_client_pd *x)
{
    PdRcp::postRabbitcontrolInit();
    post("");
    PdRcp::postVersion();
}


void *rcpclient_pd_new(t_symbol *s, int argc, t_atom *argv)
{
    t_rabbit_client_pd *x = (t_rabbit_client_pd *)pd_new(rcp_client_pd_class);

    x->parameter_client = new ParameterClient(x, argc, argv);

    return (void *)x;
}


void rcpclient_pd_free(t_rabbit_client_pd *x)
{
    if (x->parameter_client)
    {
        delete x->parameter_client;
    }
}


void setup_rabbit0x2eclient(void) {
    rcp_client_pd_class = class_new(gensym("rabbit.client"),
                                   (t_newmethod)rcpclient_pd_new,
                                   (t_method)rcpclient_pd_free,
                                   sizeof(t_rabbit_client_pd),
                                   CLASS_DEFAULT,
                                   A_GIMME,
                                   0);

    // parameter client/server
    class_addbang(rcp_client_pd_class, (t_method)rcpclient_bang);
    class_addlist(rcp_client_pd_class, (t_method)rcpclient_list);
    class_addanything(rcp_client_pd_class, (t_method)rcpclient_any);

    class_addmethod(rcp_client_pd_class, (t_method)post_rcp_version, gensym("getrcpversion"), A_NULL);

    // NOTE: getter are handled with inlet anything

    // parameter client
    class_addmethod(rcp_client_pd_class, (t_method)rcpclient_connect, gensym("connect"), A_SYMBOL, A_NULL);
    class_addmethod(rcp_client_pd_class, (t_method)rcpclient_disconnect, gensym("disconnect"), A_NULL);

    // keep these around for backward compatibility
    class_addmethod(rcp_client_pd_class, (t_method)rcpclient_connect, gensym("open"), A_SYMBOL, A_NULL);
    class_addmethod(rcp_client_pd_class, (t_method)rcpclient_disconnect, gensym("close"), A_NULL);

    // raw
    // input is handled by inlet anything
}

#ifdef __cplusplus
} // extern "C"
#endif
