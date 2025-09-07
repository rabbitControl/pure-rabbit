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

#include "PdClientTransporter.h"

#include <rcp_memory.h>
#include <rcp_client.h>

#include "ParameterClient.h"


static void pd_client_transporter_send(rcp_client_transporter* transporter, const char* data, size_t size)
{
    if (transporter &&
            transporter->user)
    {
        ((rcp::PdClientTransporter*)transporter->user)->pdClient()->dataOut(data, size);
    }
}


namespace rcp
{

PdClientTransporter::PdClientTransporter(ParameterClient* client)
    : m_pdClient(client)
{
    m_transporter = (rcp_client_transporter*)RCP_CALLOC(1, sizeof(rcp_client_transporter));

    if (m_transporter)
    {
        rcp_client_transporter_setup(m_transporter,
                                     pd_client_transporter_send);

        m_transporter->user = this;
    }
}

PdClientTransporter::~PdClientTransporter()
{
    if (m_transporter)
    {
        RCP_FREE(m_transporter);
    }
}


rcp_client_transporter* PdClientTransporter::transporter() const
{
    return m_transporter;
}

ParameterClient* PdClientTransporter::pdClient() const
{
    return m_pdClient;
}

void PdClientTransporter::pushData(const char* data, size_t size) const
{
    if (m_transporter &&
        data &&
        size > 0)
    {
        // NOTE: no need to lock

        rcp_client_transporter_call_recv_cb(m_transporter, data, size);
    }
}

} // namespace rcp



