#ifndef RCP_CLIENT_PD_H
#define RCP_CLIENT_PD_H

#include <m_pd.h>

#include <rcp_server_type.h>

namespace rcp {
class ParameterClient;
}

#ifdef __cplusplus
extern "C"{
#endif

typedef struct _rabbit_client_pd
{
    t_object x_obj;

    rcp::ParameterClient* parameter_client;

    t_outlet* parameter_out;
    t_outlet* parameter_id_out;
    t_outlet* client_connected_out;
    t_outlet* info_out;

    t_inlet* raw_in;
    t_outlet* raw_out;

} t_rabbit_client_pd;

#ifdef __cplusplus
} // extern "C"
#endif


#endif // RCP_CLIENT_PD_H
