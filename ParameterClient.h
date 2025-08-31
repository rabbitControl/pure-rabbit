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
