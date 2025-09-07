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

#ifndef RCPFORMAT_H
#define RCPFORMAT_H

#include "rabbit.format.h"
#include <string>
#include <vector>

#include <m_pd.h>

#include <rcp.h>

class RcpFormat
{
public:
    RcpFormat(t_rabbit_format_pd* x, int argc, t_atom *argv);
    ~RcpFormat();

public:
    void handleBang();
    void handleInt(int v);
    void handleFloat(float v);
    void handleSymbol(t_symbol* v);

    void setId(int v);

    // type
    void setType(const t_symbol* p);
    const t_symbol* getType() const;
    // label
    void setLabel(const t_symbol* p);
    const t_symbol* getLabel() const;
    void clearLabel();

private:
    t_rabbit_format_pd* m_x{nullptr};

    int16_t m_id;
    rcp_datatype m_type;
    std::string m_label;

    // update start
    // 4 18 id id t 0 32 value 33 any len label 0 0 0

    std::vector<char> labelOption;
};

#endif // RCPFORMAT_H
