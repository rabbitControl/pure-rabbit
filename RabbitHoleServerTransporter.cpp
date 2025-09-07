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

#include "RabbitHoleServerTransporter.h"

#include <rcp_memory.h>
#include <rcp_server.h>

#include "Threading.h"
#include "rabbit.server.h"

// callbacks
static void _pd_rabbithole_server_transporter_sendToOne(rcp_server_transporter* transporter, const char* data, size_t data_size, void* /*id*/)
{
    if (transporter &&
        transporter->user)
    {
        ((rcp::RabbitHoleServerTransporter*)transporter->user)->send(data, data_size);
    }
}

static void _pd_rabbithole_server_transporter_sendToAll(rcp_server_transporter* transporter, const char* data, size_t data_size, void* /*excludeId*/)
{
    if (transporter &&
        transporter->user)
    {
        ((rcp::RabbitHoleServerTransporter*)transporter->user)->send(data, data_size);
    }
}

static void _rhl_connection_timer(rcp::RabbitHoleServerTransporter* transporter)
{
    transporter->reconnect();
}


namespace rcp
{

RabbitHoleServerTransporter::RabbitHoleServerTransporter(t_pd* x, rcp_server* server)
    : WebsocketClient()
    , m_x(x)
    , m_rcpServer(server)
{
    // make sure it sends in binary
    binary(true);
    verifyPeer(false);

    m_transporter = (rcp_server_transporter*)RCP_CALLOC(1, sizeof(rcp_server_transporter));

    if (m_transporter)
    {
        rcp_server_transporter_setup(m_transporter,
                                     _pd_rabbithole_server_transporter_sendToOne,
                                     _pd_rabbithole_server_transporter_sendToAll);

        rcp_server_add_transporter(m_rcpServer, m_transporter);

        m_transporter->user = this;

        m_connectionTimer = clock_new(this, (t_method)_rhl_connection_timer);
    }
}


RabbitHoleServerTransporter::~RabbitHoleServerTransporter()
{
    if (m_connectionTimer)
    {
        clock_free(m_connectionTimer);
        m_connectionTimer = nullptr;
    }

    if (m_transporter)
    {
        rcp_server_remove_transporter(m_rcpServer, m_transporter);

        RCP_FREE(m_transporter);
        m_transporter = nullptr;
    }
}

void RabbitHoleServerTransporter::send(const char* data, size_t data_size)
{
    std::vector<char> d;

    for (size_t i=0; i<data_size; i++)
    {
        d.push_back(data[i]);
    }

    WebsocketClient::send(d);
}

void RabbitHoleServerTransporter::connected()
{
    clock_unset(m_connectionTimer);

    pd_queue_mess(&pd_maininstance, (t_pd*)m_x, NULL, pd_client_connected);

    m_connected = true;
}

void RabbitHoleServerTransporter::disconnected(uint16_t code)
{
    if (m_connected)
    {
        pd_queue_mess(&pd_maininstance, (t_pd*)m_x, NULL, pd_client_disconnected);

        m_connected = false;
    }

    // start reconnect
    clock_delay(m_connectionTimer, m_connectionInverval);
}

void RabbitHoleServerTransporter::received(const char* data, size_t size)
{
    if (m_transporter &&
        data  &&
        size > 0)
    {
        if (m_transporter->received)
        {
            std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

            rcp_server_transporter_call_recv_cb(m_transporter, data, size, NULL);
        }
    }
}

void RabbitHoleServerTransporter::setInterval(int i)
{
    m_connectionInverval = i;
}

} // namespace rcp
