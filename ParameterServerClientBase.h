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
    virtual void handle_raw_data(char* data, size_t size) = 0;

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
    void raw_data_list(int argc, t_atom* argv);

private:
    void* m_obj{nullptr};
};

} // namespace rcp


#endif // PARAMETERSERVERCLIENTBASE_H
