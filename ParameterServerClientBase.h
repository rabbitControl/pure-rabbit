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

#ifndef PARAMETERSERVERCLIENTBASE_H
#define PARAMETERSERVERCLIENTBASE_H

#include <string>
#include <vector>

#include <rcp_manager.h>

#include <m_pd.h>

namespace rcp
{

class ParameterServerClientBase
{
public:
    ParameterServerClientBase(void* obj);

    void parameterUpdate(rcp_parameter* parameter);

    void any(t_symbol* sym, int argc, t_atom* argv);
    void list(int argc, t_atom* argv);
    void bang() const;

    void dataOut(const char* data, size_t size) const;

public:
    void parameterInfo(int argc, t_atom* argv);
    void parameterId(int argc, t_atom* argv);
    void parameterType(int argc, t_atom* argv);
    void parameterReadonly(int argc, t_atom* argv);
    void parameterOrder(int argc, t_atom* argv);
    void parameterValue(int argc, t_atom* argv);
    void parameterMin(int argc, t_atom* argv);
    void parameterMax(int argc, t_atom* argv);

    std::string GetAsString(const t_atom &a);
    rcp_parameter* getParameter(int argc, t_atom* argv, rcp_group_parameter* group = NULL);
    std::vector<std::string> getParents(rcp_parameter* parameter);

protected:
    // ParameterServerClientBase
    virtual void outputIdParameterList(std::vector<t_atom>* list) = 0;
    virtual void handleRawData(char* data, size_t size) = 0;

protected:
    void setOutlets(t_outlet* parameterOutlet,
                    t_outlet* parameterIdOutlet,
                    t_outlet* infoOutlet);
    void setRawOutlet(t_outlet* outlet);

protected:
    bool m_raw{false};

    rcp_manager* m_manager{nullptr};

    t_outlet* m_parameterOutlet{nullptr};
    t_outlet* m_parameterIdOutlet{nullptr};
    t_outlet* m_infoOutlet{nullptr};
    t_outlet* m_rawDataOutlet{nullptr};

private:
    void _outputInfo(rcp_parameter* parameter, int argc, t_atom* argv);
    void _input(rcp_parameter* parameter, int argc, t_atom* argv);
    void _rawDataList(int argc, t_atom* argv);

private:
    void* m_obj{nullptr};
};

} // namespace rcp


#endif // PARAMETERSERVERCLIENTBASE_H
