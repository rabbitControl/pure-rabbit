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
    void handle_raw_data(char* data, size_t size) override;

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
