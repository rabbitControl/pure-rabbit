#ifndef RCP_PD_SERVER_H
#define RCP_PD_SERVER_H

#include <m_pd.h>

#include <rcp_server_type.h>

namespace rcp {
class ParameterServer;
}

#ifdef __cplusplus
extern "C"{
#endif

typedef struct _rabbit_server_pd
{
    t_object x_obj;

    bool raw;

    rcp::ParameterServer* parameter_server;
    int clients;

    t_outlet* parameter_out;
    t_outlet* parameter_id_out;
    t_outlet* client_count_out;
    t_outlet* info_out;

    t_inlet* raw_in;
    t_outlet* raw_out;

} t_rabbit_server_pd;

void pd_client_connected(t_pd *obj, void *data);
void pd_client_disconnected(t_pd *obj, void *data);

#ifdef __cplusplus
} // extern "C"
#endif


#endif // RCP_PD_SERVER_H
