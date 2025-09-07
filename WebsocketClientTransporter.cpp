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

#include "WebsocketClientTransporter.h"

#include <vector>

#include <rcp_memory.h>

#include "Threading.h"
#include "rabbit.client.h"

//
static void _pd_websocket_client_transporter_send(rcp_client_transporter* transporter, const char* data, size_t size)
{
    if (transporter &&
        transporter->user)
    {
        ((rcp::WebsocketClientTransporter*)transporter->user)->send(data, size);
    }
}

// synchronized from threaded transporter

static void pd_client_connected(t_pd *obj, void *data)
{
    if (obj != NULL)
    {
        t_rabbit_client_pd* x = (t_rabbit_client_pd*)obj;

        outlet_float(x->client_connected_out, 1);
    }
}

static void pd_client_disconnected(t_pd *obj, void *data)
{
    if (obj != NULL)
    {
        t_rabbit_client_pd* x = (t_rabbit_client_pd*)obj;

        outlet_float(x->client_connected_out, 0);
    }
}



namespace rcp
{

WebsocketClientTransporter::WebsocketClientTransporter(t_pd* x)
    : WebsocketClient()
    , m_x(x)
{
    binary(true);

    m_transporter = (rcp_client_transporter*)RCP_CALLOC(1, sizeof(rcp_client_transporter));

    if (m_transporter)
    {
        rcp_client_transporter_setup(m_transporter,
                                     _pd_websocket_client_transporter_send);

        m_transporter->user = this;
    }
}

WebsocketClientTransporter::~WebsocketClientTransporter()
{
    if (m_transporter)
    {
        RCP_FREE(m_transporter);
        m_transporter = nullptr;
    }
}

void WebsocketClientTransporter::send(const char *data, size_t size)
{
    std::vector<char> d(size);

    for (int i = 0; i < size; ++i)
    {
        d[i] = data[i];
    }

    WebsocketClient::send(d);
}

// IClientTransporter
rcp_client_transporter* WebsocketClientTransporter::transporter() const
{
    return m_transporter;
}

void WebsocketClientTransporter::connect(const std::string& address)
{
    WebsocketClient::connect(address);
}

void WebsocketClientTransporter::disconnect()
{
    WebsocketClient::disconnect();
}


// threaded
void WebsocketClientTransporter::connected()
{
    rcp_client_transporter_call_connected_cb(m_transporter);

    pd_queue_mess(&pd_maininstance, (t_pd*)m_x, NULL, pd_client_connected);
}

void WebsocketClientTransporter::disconnected(uint16_t code)
{
    rcp_client_transporter_call_disconnected_cb(m_transporter);

    pd_queue_mess(&pd_maininstance, (t_pd*)m_x, NULL, pd_client_disconnected);
}

void WebsocketClientTransporter::received(const char* data, size_t size)
{
    // handle binary data

    if (m_transporter &&
        data  &&
        size > 0)
    {
        std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

        rcp_client_transporter_call_recv_cb(m_transporter, data, size);
    }
}


} // namespace rcp
