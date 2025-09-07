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

#ifndef WEBSOCKETCLIENTTRANSPORTER_H
#define WEBSOCKETCLIENTTRANSPORTER_H

#include <m_pd.h>

#include <rcp_client_transporter.h>

#include <WebsocketClient.h>

#include "IClientTransporter.h"

using namespace scaryws;

namespace rcp
{

class WebsocketClientTransporter
    : public WebsocketClient
    , public IClientTransporter
{
public:
    WebsocketClientTransporter(t_pd* x);
    ~WebsocketClientTransporter();

    void send(const char* data, size_t size);

public:
    // IClientTransporter
    rcp_client_transporter* transporter() const override;
    void connect(const std::string& address) override;
    void disconnect() override;

public:
    // IClientSessionListener
    void connected() override;
    // void error(int code, const std::string& message) override;
    void disconnected(uint16_t code) override;
    void received(const char* data, size_t size) override;

private:
    t_pd* m_x{nullptr};
    rcp_client_transporter* m_transporter{nullptr};
};

} // namespace rcp


#endif // WEBSOCKETCLIENTTRANSPORTER_H
