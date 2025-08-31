#include "PdRcp.h"

#include <cstring>

#include <m_pd.h>
#include <rcp.h>

bool PdRcp::toasted = false;


void PdRcp::postRabbitcontrolInit()
{
    if (toasted) return;

    post("");
#ifdef PD_MAJOR_VERSION
    PdRcp::rabbitPost("RabbitControl for Pd");
#else
    PdRcp::rabbitPost("RabbitControl for Max");
#endif

    toasted = true;
}

void PdRcp::postVersion()
{
#ifdef PD_MAJOR_VERSION
    post("RCP Pd version: %s", RCP_PD_VERSION);
#else
    post("RCP Max version: %s", RCP_PD_VERSION);
#endif

    post("RCP version: %s", RCP_VERSION);
}

void PdRcp::rabbitPost(const char* msg)
{
    post("()()");
    (msg != NULL && strlen(msg) > 0 ) ? post(" oO    %s", msg) : post(" oO");
    post("  x");
}

void PdRcp::rabbitPostOneline(const char* msg)
{
    (msg != NULL && strlen(msg) > 0 ) ? post("()()    %s", msg) : post("()()");
}
