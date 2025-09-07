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

#ifndef RCP_PARAMETERCLIENT_H
#define RCP_PARAMETERCLIENT_H

#include <string>

#include <rcp_client.h>

#include "rabbit.client.h"
#include "IClientTransporter.h"
#include "ParameterServerClientBase.h"

using namespace std;

namespace rcp
{

class ParameterClient
    : public ParameterServerClientBase
{
public:
    ParameterClient(t_rabbit_client_pd* x, int argc, t_atom *argv);
    ~ParameterClient();

    void connect(string url);
    void disconnect();

    void parameterAddedThreaded(rcp_parameter* parameter);
    void parameterRemovedThreaded(rcp_parameter* parameter);

private:
    //
    void outputIdParameterList(std::vector<t_atom>* list) override;
    void handle_raw_data(char* data, size_t size) override;

private:
    rcp_group_parameter* createGroups(int argc, t_atom* argv, std::string& outLabel);
    void setupValueParameter(rcp_value_parameter* parameter);

private:
    t_rabbit_client_pd* m_x{nullptr};

    IClientTransporter* m_transporter{nullptr};
    rcp_client* m_client{nullptr};
};

} // namespace rcp

#endif // RCP_PARAMETERCLIENT_H
