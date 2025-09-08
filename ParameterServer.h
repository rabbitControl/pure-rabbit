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

#ifndef RCP_PARAMETERSERVER_H
#define RCP_PARAMETERSERVER_H

#include <rcp_server.h>

#include "IServerTransporter.h"
#include "ParameterServerClientBase.h"
#include "RabbitHoleServerTransporter.h"
#include "rabbit.server.h"

namespace rcp
{

class ParameterServer
    : public ParameterServerClientBase
{
public:
    ParameterServer(t_rabbit_server_pd* x, int argc, t_atom *argv);
    ~ParameterServer();

    // port
    int port() const;
    void listen(int port);

    size_t clientCount() const;

public:
    // parameter
    void exposeParameter(int argc, t_atom* argv);
    void removeParameter(int id);
    void removeParameterList(int argc, t_atom* argv);
    // parameter options
    void parameterSetReadonly(int argc, t_atom* argv);
    void parameterSetOrder(int argc, t_atom* argv);
    // min max
    void parameterSetMin(int argc, t_atom* argv);
    void parameterSetMax(int argc, t_atom* argv);
    void parameterSetMinMax(int argc, t_atom* argv);

    // rabbithole
    void setRabbithole(const std::string& uri);
    void setRabbitholeInterval(const int i);

private:
    void outputIdParameterList(std::vector<t_atom>* list) override;
    void handleRawData(char* data, size_t size) override;

private:
    rcp_group_parameter* createGroups(int argc, t_atom* argv, std::string& outLabel);
    void setupValueParameter(rcp_value_parameter* parameter);

private:
    t_rabbit_server_pd* m_x{nullptr};

    IServerTransporter* m_transporter{nullptr};
    rcp_server* m_server{nullptr};

    std::shared_ptr<RabbitHoleServerTransporter> m_rabbitholeTransporter;
};

} // namespace rcp


#endif // RCP_PARAMETERSERVER_H
