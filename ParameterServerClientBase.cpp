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

#include "ParameterServerClientBase.h"

#include <m_pd.h>

#include <rcp_parameter.h>
#include <rcp_typedefinition.h>

#include "PdMaxUtils.h"
#include "Threading.h"

using namespace PdMaxUtils;

static void postAtoms(int argc, const t_atom* argv)
{
    for (int i=0; i<argc; i++)
    {
        if (argv[i].a_type == A_FLOAT)
        {
            post("float: %f", argv[i].a_w.w_float);
        }
#ifndef PD_MAJOR_VERSION
        else if (argv[i].a_type == A_INT)
        {
            post("int: %d", GetInt(argv[i]));
        }
#endif
        else if (argv[i].a_type == A_SYMBOL)
        {
            post("sym: %s", argv[i].a_w.w_symbol->s_name);
        }
        else
        {
            post("other");
        }
    }
}

static bool setAtomValue(rcp_parameter* param, const t_atom& atom)
{
    if (param == nullptr) return false;

    rcp_datatype type = rcp_typedefinition_get_type_id(rcp_parameter_get_typedefinition(param));
    bool was_set = false;

    if (rcp_parameter_is_value(param))
    {
        if (type == DATATYPE_BOOLEAN)
        {
            if (canBeInt(atom))
            {
                rcp_parameter_set_value_bool(RCP_VALUE_PARAMETER(param), getInt(atom) > 0);
                was_set = true;
            }
        }
        else if (type == DATATYPE_INT32)
        {
            if (canBeInt(atom))
            {
                rcp_parameter_set_value_int32(RCP_VALUE_PARAMETER(param), getInt(atom));
                was_set = true;
            }
        }
        else if (type == DATATYPE_FLOAT32)
        {
            if (canBeFloat(atom))
            {
                rcp_parameter_set_value_float(RCP_VALUE_PARAMETER(param), atom.a_w.w_float);
                was_set = true;
            }
        }
        else if (type == DATATYPE_STRING)
        {
            if (atom.a_type == A_SYMBOL)
            {
                rcp_parameter_set_value_string(RCP_VALUE_PARAMETER(param), atom.a_w.w_symbol->s_name);
                was_set = true;
            }
        }
    }
    else if (type == DATATYPE_BANG)
    {
    }
    else if (type == DATATYPE_GROUP)
    {
    }

    return was_set;
}


static std::string typeToString(rcp_datatype type)
{
    switch(type)
    {
    case DATATYPE_FLOAT32:
        return "float";
    case DATATYPE_INT32:
        return "int";
    case DATATYPE_BOOLEAN:
        return "toggle";
    case DATATYPE_BANG:
        return "bang";
    case DATATYPE_STRING:
        return "string";
    case DATATYPE_GROUP:
        return "group";
    }

    return "unknown";
}


namespace rcp
{

ParameterServerClientBase::ParameterServerClientBase(void* obj)
    : m_obj(obj)
{
}

void ParameterServerClientBase::_input(rcp_parameter* parameter, int argc, t_atom* argv)
{
    if (parameter)
    {
        if (rcp_parameter_is_group(parameter))
        {
            if (argc > 1)
            {
                // look for parameter in group
                parameter = getParameter(argc-1, argv, RCP_GROUP_PARAMETER(parameter));
                if (parameter == NULL)
                {
                    pd_error(m_obj, "parameter not found");
                    return;
                }

                if (rcp_parameter_is_group(parameter))
                {
                    pd_error(m_obj, "can not set value for group parameter");
                    return;
                }
            }
            else
            {
                // can not set value for group parameter
                pd_error(m_obj, "can not set value for group parameter");
                return;
            }
        }
        else if (rcp_parameter_is_type(parameter, DATATYPE_BANG))
        {
            rcp_manager_set_dirty(m_manager, parameter);
            rcp_manager_update(m_manager);
            return;
        }

        // set value
        if (setAtomValue(parameter, argv[argc-1]))
        {
            rcp_manager_update(m_manager);
        }
    }
}

void ParameterServerClientBase::list(int argc, t_atom* argv)
{
    // <id> <value>
    // <group> <group> ... <label> <value>

    //        postAtoms(argc, argv);

    int16_t id = 0;
    if (canBeInt(argv[0]))
    {
        id = getInt(argv[0]);
    }

    std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

    if (id != 0)
    {
        rcp_parameter* p = rcp_manager_get_parameter(m_manager, id);

        if (setAtomValue(p, argv[argc-1]))
        {
            rcp_manager_update(m_manager);
            return;
        }
    }

    if (argv[0].a_type == A_SYMBOL)
    {
        rcp_parameter* param = rcp_manager_find_parameter(m_manager, argv[0].a_w.w_symbol->s_name, NULL);
        _input(param, argc-1, argv+1);
    }
}

void ParameterServerClientBase::any(t_symbol* sym, int argc, t_atom* argv)
{
    if (sym == gensym("__raw_input"))
    {
        if (m_raw)
        {
            _rawDataList(argc, argv);
            return;
        }
    }

    if (strcmp(sym->s_name, "getinfo") == 0)
    {
        parameterInfo(argc, argv);
    }
    else if (strcmp(sym->s_name, "getid") == 0)
    {
        parameterId(argc, argv);
    }
    else if (strcmp(sym->s_name, "gettype") == 0)
    {
        parameterType(argc, argv);
    }
    else if (strcmp(sym->s_name, "getvalue") == 0)
    {
        parameterValue(argc, argv);
    }
    else if (strcmp(sym->s_name, "getreadonly") == 0)
    {
        parameterReadonly(argc, argv);
    }
    else if (strcmp(sym->s_name, "getorder") == 0)
    {
        parameterOrder(argc, argv);
    }
    else if (strcmp(sym->s_name, "getmin") == 0)
    {
        parameterMin(argc, argv);
    }
    else if (strcmp(sym->s_name, "getmax") == 0)
    {
        parameterMax(argc, argv);
    }
    else
    {
        std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

        rcp_parameter* param = rcp_manager_find_parameter(m_manager, sym->s_name, NULL);
        _input(param, argc, argv);
    }
}


void ParameterServerClientBase::_rawDataList(int argc, t_atom* argv)
{
    std::vector<char> data(argc);
    int offset = 0;
    int value = -1;

    for (int i=0; i<argc; i++)
    {
        if (canBeInt(argv[i]))
        {
            value = getInt(argv[i]);
            if (value < 0 || value > 255)
            {
                pd_error(m_obj, "invalid data in list");
                return;
            }

            data[i-offset] = value;
        }
        else
        {
            offset++;
        }
    }

    handleRawData(data.data(), argc-offset);
}


void ParameterServerClientBase::bang() const
{
    std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

    rcp_parameter_list* list = rcp_manager_get_paramter_list(m_manager);
    post("---- parameter ----");
    while (list != NULL)
    {
        rcp_datatype type = RCP_TYPE_ID(list->parameter);
        std::string ts = typeToString(type);
        const char* label = rcp_parameter_get_label(list->parameter);
        post("%s\tid: %d\ttype: %s", label, rcp_parameter_get_id(list->parameter), ts.c_str());
        list = list->next;
    }
}

void ParameterServerClientBase::dataOut(const char* data, size_t size) const
{
    if (m_rawDataOutlet)
    {
        std::vector<t_atom> atoms(size);

        for (size_t i=0; i<size; i++)
        {
            setInt(atoms[i], data[i]);
        }

        outlet_list(m_rawDataOutlet, &s_list, size, atoms.data());
    }
}


void ParameterServerClientBase::_outputInfo(rcp_parameter* parameter, int argc, t_atom* argv)
{
    if (parameter)
    {
        rcp_typedefinition* td = rcp_parameter_get_typedefinition(parameter);

        int16_t id = rcp_parameter_get_id(parameter);
        rcp_datatype type = RCP_TYPE_ID(parameter);
        std::string ts = typeToString(type);

        // info <group-label-list> <value> <min> <max> <id> <type>

        int len = 2 + argc;
        bool has_min = false;
        bool has_max = false;

        if (rcp_parameter_is_value(parameter))
        {
            len++;

            if (type == DATATYPE_FLOAT32 || type == DATATYPE_INT32)
            {
                // check if we have min and max
                if (rcp_typedefinition_has_option(td, NUMBER_OPTIONS_MINIMUM))
                {
                    has_min = true;
                    len++;
                }

                if (rcp_typedefinition_has_option(td, NUMBER_OPTIONS_MAXIMUM))
                {
                    has_max = true;
                    len++;
                }
            }
        }


        std::vector<t_atom> list(len);

        int i=0;
        for (int j=0; j<argc; j++, i++) {
            list[i] = argv[j];
        }

        // set value
        if (type == DATATYPE_BOOLEAN)
        {
            setInt(list[i], rcp_parameter_get_value_bool(RCP_VALUE_PARAMETER(parameter)) ? 1 : 0);
            i++;
        }
        else if (type == DATATYPE_INT32)
        {
            setInt(list[i], rcp_parameter_get_value_int32(RCP_VALUE_PARAMETER(parameter)));
            i++;

            if (has_min)
            {
                // add min
                setInt(list[i], rcp_typedefinition_get_option_i32(td, NUMBER_OPTIONS_MINIMUM, 0));
                i++;
            }

            if (has_max)
            {
                // add max
                setInt(list[i], rcp_typedefinition_get_option_i32(td, NUMBER_OPTIONS_MAXIMUM, 0));
                i++;
            }
        }
        else if (type == DATATYPE_FLOAT32)
        {
            setFloat(list[i], rcp_parameter_get_value_float(RCP_VALUE_PARAMETER(parameter)));
            i++;

            if (has_min)
            {
                // add min
                setFloat(list[i], rcp_typedefinition_get_option_f32(td, NUMBER_OPTIONS_MINIMUM, 0));
                i++;
            }

            if (has_max)
            {
                // add max
                setFloat(list[i], rcp_typedefinition_get_option_f32(td, NUMBER_OPTIONS_MAXIMUM, 0));
                i++;
            }
        }
        else if (type == DATATYPE_STRING)
        {
            setString(list[i], rcp_parameter_get_value_string(RCP_VALUE_PARAMETER(parameter)));
            i++;
        }

        setInt(list[i], id);
        i++;

        setString(list[i], ts.c_str());
        i++;

        outlet_anything(m_infoOutlet, gensym("info"), list.size(), list.data());
    }
}

void ParameterServerClientBase::parameterInfo(int argc, t_atom* argv)
{
    std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

    if (argc == 0)
    {
        // output all
        rcp_parameter_list* list = rcp_manager_get_paramter_list(m_manager);
        while (list != NULL)
        {
            std::vector<std::string> groups = getParents(list->parameter);

            std::vector<t_atom> groups_a(groups.size());

            int i = 0;
            for(std::vector<std::string>::reverse_iterator rit = groups.rbegin();
                 rit != groups.rend(); ++rit)
            {
                setString(groups_a[i], rit->c_str());
                i++;
            }

            _outputInfo(list->parameter, groups.size(), groups_a.data());

            list = list->next;
        }
    }
    else
    {
        rcp_parameter* parameter = getParameter(argc, argv);
        _outputInfo(parameter, argc, argv);
    }
}

void ParameterServerClientBase::parameterId(int argc, t_atom* argv)
{
    std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

    rcp_parameter* parameter = getParameter(argc, argv);
    if (parameter)
    {
        int16_t id = rcp_parameter_get_id(parameter);

        // id <group-label-list> <id>
        int len = 1 + argc;
        std::vector<t_atom> list(len);

        int i=0;
        for (int j=0; j<argc; j++, i++) {
            list[i] = argv[j];
        }

        setInt(list[i], id);
        i++;

        outlet_anything(m_infoOutlet, gensym("id"), list.size(), list.data());
    }
}

void ParameterServerClientBase::parameterType(int argc, t_atom* argv)
{
    std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

    rcp_parameter* parameter = getParameter(argc, argv);
    if (parameter)
    {
        rcp_typedefinition* td = rcp_parameter_get_typedefinition(parameter);
        rcp_datatype type = RCP_TYPE_ID(parameter);
        std::string ts = typeToString(type);


        // id <group-label-list> <type>
        int len = 1 + argc;
        std::vector<t_atom> list(len);

        int i=0;
        for (int j=0; j<argc; j++, i++) {
            list[i] = argv[j];
        }

        setString(list[i], ts.c_str());
        i++;

        outlet_anything(m_infoOutlet, gensym("type"), list.size(), list.data());
    }
}

void ParameterServerClientBase::parameterReadonly(int argc, t_atom* argv)
{
    std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

    rcp_parameter* parameter = getParameter(argc, argv);
    if (parameter)
    {
        bool ro = rcp_parameter_get_readonly(parameter);

        // readonly <group-label-list> <ro>
        int len = 1 + argc;
        std::vector<t_atom> list(len);

        int i=0;
        for (int j=0; j<argc; j++, i++) {
            list[i] = argv[j];
        }

        setInt(list[i], ro ? 1 : 0);
        i++;

        outlet_anything(m_infoOutlet, gensym("readonly"), list.size(), list.data());
    }
}

void ParameterServerClientBase::parameterOrder(int argc, t_atom* argv)
{
    std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

    rcp_parameter* parameter = getParameter(argc, argv);
    if (parameter)
    {
        int32_t order = rcp_parameter_get_order(parameter);

        // order <group-label-list> <order>
        int len = 1 + argc;
        std::vector<t_atom> list(len);

        int i=0;
        for (int j=0; j<argc; j++, i++) {
            list[i] = argv[j];
        }

        setInt(list[i], order);
        i++;

        outlet_anything(m_infoOutlet, gensym("order"), list.size(), list.data());
    }
}

void ParameterServerClientBase::parameterValue(int argc, t_atom* argv)
{
    std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

    rcp_parameter* parameter = getParameter(argc, argv);
    if (parameter)
    {
        rcp_datatype type = RCP_TYPE_ID(parameter);

        // value <group-label-list> <value>
        int len = argc + (rcp_parameter_is_value(parameter) ? 1 : 0);
        std::vector<t_atom> list(len);

        int i=0;
        for (int j=0; j<argc; j++, i++) {
            list[i] = argv[j];
        }

        if (type == DATATYPE_BOOLEAN)
        {
            setInt(list[i], rcp_parameter_get_value_bool(RCP_VALUE_PARAMETER(parameter)) ? 1 : 0);
            i++;
        }
        else if (type == DATATYPE_INT32)
        {
            setInt(list[i], rcp_parameter_get_value_int32(RCP_VALUE_PARAMETER(parameter)));
            i++;
        }
        else if (type == DATATYPE_FLOAT32)
        {
            setFloat(list[i], rcp_parameter_get_value_float(RCP_VALUE_PARAMETER(parameter)));
            i++;
        }
        else if (type == DATATYPE_STRING)
        {
            setString(list[i], rcp_parameter_get_value_string(RCP_VALUE_PARAMETER(parameter)));
            i++;
        }

        outlet_anything(m_infoOutlet, gensym("value"), list.size(), list.data());
    }
}

void ParameterServerClientBase::parameterMin(int argc, t_atom* argv)
{
    std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

    rcp_parameter* parameter = getParameter(argc, argv);
    if (parameter)
    {
        rcp_typedefinition* td = rcp_parameter_get_typedefinition(parameter);
        rcp_datatype type = RCP_TYPE_ID(parameter);

        if (rcp_typedefinition_has_option(td, NUMBER_OPTIONS_MINIMUM))
        {
            // min <group-label-list> <min>
            int len = argc + 1;
            std::vector<t_atom> list(len);

            int i=0;
            for (int j=0; j<argc; j++, i++) {
                list[i] = argv[j];
            }

            if (type == DATATYPE_INT32)
            {
                setInt(list[i], rcp_parameter_get_min_int32(RCP_VALUE_PARAMETER(parameter)));
                i++;
            }
            else if (type == DATATYPE_FLOAT32)
            {
                setFloat(list[i], rcp_parameter_get_min_float(RCP_VALUE_PARAMETER(parameter)));
                i++;
            }

            outlet_anything(m_infoOutlet, gensym("min"), list.size(), list.data());
        }
    }
}

void ParameterServerClientBase::parameterMax(int argc, t_atom* argv)
{
    std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

    rcp_parameter* parameter = getParameter(argc, argv);
    if (parameter)
    {
        rcp_typedefinition* td = rcp_parameter_get_typedefinition(parameter);
        rcp_datatype type = RCP_TYPE_ID(parameter);

        if (rcp_typedefinition_has_option(td, NUMBER_OPTIONS_MAXIMUM))
        {
            // max <group-label-list> <max>
            int len = argc + 1;
            std::vector<t_atom> list(len);

            int i=0;
            for (int j=0; j<argc; j++, i++) {
                list[i] = argv[j];
            }

            if (type == DATATYPE_INT32)
            {
                setInt(list[i], rcp_parameter_get_max_int32(RCP_VALUE_PARAMETER(parameter)));
                i++;
            }
            else if (type == DATATYPE_FLOAT32)
            {
                setFloat(list[i], rcp_parameter_get_max_float(RCP_VALUE_PARAMETER(parameter)));
                i++;
            }

            outlet_anything(m_infoOutlet, gensym("max"), list.size(), list.data());
        }
    }
}

std::string ParameterServerClientBase::GetAsString(const t_atom &a)
{
    if (a.a_type == A_SYMBOL)
    {
        return a.a_w.w_symbol->s_name;
    }

    if (isInt(a))
    {
        int i = getInt(a);
        return std::to_string(i);
    }

    if (a.a_type == A_FLOAT)
    {
        return std::to_string(a.a_w.w_float);
    }

    return "";
}

rcp_parameter* ParameterServerClientBase::getParameter(int argc, t_atom* argv, rcp_group_parameter* group)
{
    rcp_parameter* param = NULL;
    rcp_group_parameter* last_group = group;

    for (int i=0; i<argc; i++)
    {
        if (argv[i].a_type != A_SYMBOL)
        {
            // not a string!
            return NULL;
        }

        param = rcp_manager_find_parameter(m_manager, argv[i].a_w.w_symbol->s_name, last_group);
        if (param == NULL)
        {
            // not found
            return NULL;
        }

        if (rcp_parameter_is_group(param))
        {
            last_group = RCP_GROUP_PARAMETER(param);
        }
    }

    return param;
}

std::vector<std::string> ParameterServerClientBase::getParents(rcp_parameter* parameter)
{
    std::vector<std::string> groups;

    rcp_group_parameter* last_group = rcp_parameter_get_parent(RCP_PARAMETER(parameter));
    while (last_group != NULL)
    {
        const char* label = rcp_parameter_get_label(RCP_PARAMETER(last_group));
        groups.push_back(label != NULL ? label : "null");

        last_group = rcp_parameter_get_parent(RCP_PARAMETER(last_group));
    }

    return groups;
}


void ParameterServerClientBase::parameterUpdate(rcp_parameter* parameter)
{
    std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

    const char* label = rcp_parameter_get_label(parameter);
    int16_t id = rcp_parameter_get_id(parameter);
    rcp_datatype type = rcp_typedefinition_get_type_id(rcp_parameter_get_typedefinition(parameter));

    // get the parents
    std::vector<std::string> groups = getParents(parameter);


    // output [list]
    // update group1 groupN... label value

    int len = 1 + 3 + groups.size();
    std::vector<t_atom>* _list = new std::vector<t_atom>(len);
    std::vector<t_atom>& list = *_list;

    int i=0;

    setFloat(list[i], id);
    i++;

    setSymbol(list[i], gensym("update"));
    i++;

    for (std::vector<std::string>::reverse_iterator rit = groups.rbegin();
         rit != groups.rend(); ++rit)
    {
        setString(list[i], rit->c_str());
        i++;
    }

    setSymbol(list[i], gensym(label != NULL ? label : "<nolabel>"));
    i++;

    if (type == DATATYPE_BOOLEAN)
    {
        setInt(list[i], rcp_parameter_get_value_bool(RCP_VALUE_PARAMETER(parameter)) ? 1 : 0);
        i++;
    }
    else if (type == DATATYPE_INT32)
    {
        setInt(list[i], rcp_parameter_get_value_int32(RCP_VALUE_PARAMETER(parameter)));
        i++;
    }
    else if (type == DATATYPE_FLOAT32)
    {
        setFloat(list[i], rcp_parameter_get_value_float(RCP_VALUE_PARAMETER(parameter)));
        i++;
    }
    else if (type == DATATYPE_STRING)
    {
        setString(list[i], rcp_parameter_get_value_string(RCP_VALUE_PARAMETER(parameter)));
        i++;
    }


    outputIdParameterList(_list);
}

void ParameterServerClientBase::setOutlets(t_outlet* parameterOutlet,
                                           t_outlet* parameterIdOutlet,
                                           t_outlet* infoOutlet)
{
    m_parameterOutlet = parameterOutlet;
    m_parameterIdOutlet = parameterIdOutlet;
    m_infoOutlet = infoOutlet;
}

void ParameterServerClientBase::setRawOutlet(t_outlet* outlet)
{
    m_rawDataOutlet = outlet;
}

} // namespace rcp

