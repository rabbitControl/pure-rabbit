#ifndef RCP_PD_FORMAT_H
#define RCP_PD_FORMAT_H

#include <m_pd.h>

#include <rcp_typedefinition.h>

class RcpFormat;

#ifdef __cplusplus
extern "C"{
#endif

typedef struct _rabbit_format_pd
{
    t_object x_obj;

    RcpFormat* format;

    t_outlet* list_out;
    t_outlet* info_out;

} t_rabbit_format_pd;

#ifdef __cplusplus
} // extern "C"
#endif


#endif // RCP_PD_FORMAT_H
