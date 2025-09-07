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

#ifndef WEBSOCKETSERVERTRANSPORTER_H
#define WEBSOCKETSERVERTRANSPORTER_H

#include <m_pd.h>

#include <WebsocketServer.h>

#include <rcp_server_transporter.h>

#include "IServerTransporter.h"

using namespace scaryws;

namespace rcp
{

class WebsocketServerTransporter
    : public WebsocketServer
    , public IServerTransporter
{
public:
    WebsocketServerTransporter(t_pd* x);
    ~WebsocketServerTransporter();

    void sendToOne(const char* data, size_t size, void* id);
    void sendToAll(const char* data, size_t size, void* excludeId);

public:
    // IServerTransporter
    rcp_server_transporter* transporter() const override;
    void bind(uint16_t port) override;
    void unbind() override;
    uint16_t port() const override;
    bool isListening() const override;
    size_t clientCount() const override;

public:
    // IServerSessionListener
    virtual void listening() override;
    virtual void closed() override;
    virtual void clientConnected(void* client) override;
    virtual void clientDisconnected(void* client) override;
    virtual void received(const char* data, size_t size, void* client) override;
    // virtual void received(const std::string& msg, void* client) override; // ignore text data

private:
    t_pd* m_x{nullptr};
    rcp_server_transporter* m_transporter{nullptr};
};

} // namespace rcp


#endif // WEBSOCKETSERVERTRANSPORTER_H
