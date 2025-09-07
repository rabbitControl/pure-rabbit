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

#include "WebsocketServerTransporter.h"

#include <vector>

#include <rcp_memory.h>
#include <rcp_server_transporter.h>

#include "Threading.h"
#include "rabbit.server.h"

//
static void _pd_websocket_server_transporter_sendToOne(rcp_server_transporter* transporter, const char* data, size_t data_size, void* id)
{
    if (transporter &&
        transporter->user)
    {
        ((rcp::WebsocketServerTransporter*)transporter->user)->sendToOne(data, data_size, id);
    }
}

static void _pd_websocket_server_transporter_sendToAll(rcp_server_transporter* transporter, const char* data, size_t data_size, void* excludeId)
{
    if (transporter &&
        transporter->user)
    {
        ((rcp::WebsocketServerTransporter*)transporter->user)->sendToAll(data, data_size, excludeId);
    }
}


namespace rcp
{

WebsocketServerTransporter::WebsocketServerTransporter(t_pd* x)
    : WebsocketServer()
    , m_x(x)
{
    binary(true);

    m_transporter = (rcp_server_transporter*)RCP_CALLOC(1, sizeof(rcp_server_transporter));

    if (m_transporter)
    {
        rcp_server_transporter_setup(m_transporter,
                                     _pd_websocket_server_transporter_sendToOne,
                                     _pd_websocket_server_transporter_sendToAll);

        m_transporter->user = this;
    }
}

WebsocketServerTransporter::~WebsocketServerTransporter()
{
    if (m_transporter)
    {
        RCP_FREE(m_transporter);
        m_transporter = nullptr;
    }
}

void WebsocketServerTransporter::sendToOne(const char *data, size_t size, void *id)
{
    std::vector<char> d(size);

    for (int i = 0; i < size; ++i)
    {
        d[i] = data[i];
    }

    WebsocketServer::sendTo(d, id);
}

void WebsocketServerTransporter::sendToAll(const char *data, size_t size, void *excludeId)
{
    std::vector<char> d(size);

    for (int i = 0; i < size; ++i) {
        d[i] = data[i];
    }

    WebsocketServer::sendToAll(d, excludeId);
}

// IServerTransporter
rcp_server_transporter* WebsocketServerTransporter::transporter() const
{
    return m_transporter;
}

void WebsocketServerTransporter::bind(uint16_t port)
{
    WebsocketServer::listen(port);
}

void WebsocketServerTransporter::unbind()
{
    WebsocketServer::close();
}

bool WebsocketServerTransporter::isListening() const
{
    return WebsocketServer::isListening();
}

uint16_t WebsocketServerTransporter::port() const
{
    return WebsocketServer::port();
}

size_t WebsocketServerTransporter::clientCount() const
{
    return WebsocketServer::clientCount();
}


// threaded
void WebsocketServerTransporter::listening()
{
}

void WebsocketServerTransporter::closed()
{
}

void WebsocketServerTransporter::clientConnected(void* client)
{
    pd_queue_mess(&pd_maininstance, (t_pd*)m_x, NULL, pd_client_connected);
}

void WebsocketServerTransporter::clientDisconnected(void* client)
{
    pd_queue_mess(&pd_maininstance, (t_pd*)m_x, NULL, pd_client_disconnected);
}

void WebsocketServerTransporter::received(const char* data, size_t size, void* client)
{
    // handle binary data

    if (m_transporter &&
        data  &&
        size > 0)
    {
        if (m_transporter->received)
        {
            std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

            rcp_server_transporter_call_recv_cb(m_transporter, data, size, client);
        }
    }
}

} // namespace rcp

