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

#include "PdRcp.h"

#include <cstring>

#include <m_pd.h>
#include <rcp.h>

bool PdRcp::toasted = false;


void PdRcp::postRabbitcontrolInit()
{
    if (toasted) return;

    post("");
#ifdef PD_MAJOR_VERSION
    PdRcp::rabbitPost("RabbitControl for Pd");
#else
    PdRcp::rabbitPost("RabbitControl for Max");
#endif

    toasted = true;
}

void PdRcp::postVersion()
{
#ifdef PD_MAJOR_VERSION
    post("RCP Pd version: %s", RCP_PD_VERSION);
#else
    post("RCP Max version: %s", RCP_PD_VERSION);
#endif

    post("RCP version: %s", RCP_VERSION);
}

void PdRcp::rabbitPost(const char* msg)
{
    post("()()");
    (msg != NULL && strlen(msg) > 0 ) ? post(" oO    %s", msg) : post(" oO");
    post("  x");
}

void PdRcp::rabbitPostOneline(const char* msg)
{
    (msg != NULL && strlen(msg) > 0 ) ? post("()()    %s", msg) : post("()()");
}
