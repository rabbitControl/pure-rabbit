#ifndef RCP_PD_PARSE_H
#define RCP_PD_PARSE_H

#include <m_pd.h>

class RcpParse;

#ifdef __cplusplus
extern "C"{
#endif

typedef struct _rabbit_parse_pd
{
    t_object x_obj;

    RcpParse* parse;

    t_outlet* parameter_out;
    t_outlet* parameter_id_out;

} t_rabbit_parse_pd;

#ifdef __cplusplus
} // extern "C"
#endif


#endif // RCP_PD_PARSE_H
