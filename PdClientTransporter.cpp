#include "PdClientTransporter.h"

#include <rcp_memory.h>
#include <rcp_client.h>

#include "ParameterClient.h"


static void pd_client_transporter_send(rcp_client_transporter* transporter, const char* data, size_t size)
{
    if (transporter &&
            transporter->user)
    {
        ((rcp::PdClientTransporter*)transporter->user)->pdClient()->dataOut(data, size);
    }
}


namespace rcp
{

PdClientTransporter::PdClientTransporter(ParameterClient* client)
    : m_pdClient(client)
{
    m_transporter = (rcp_client_transporter*)RCP_CALLOC(1, sizeof(rcp_client_transporter));

    if (m_transporter)
    {
        rcp_client_transporter_setup(m_transporter,
                                     pd_client_transporter_send);

        m_transporter->user = this;
    }
}

PdClientTransporter::~PdClientTransporter()
{
    if (m_transporter)
    {
        RCP_FREE(m_transporter);
    }
}


rcp_client_transporter* PdClientTransporter::transporter() const
{
    return m_transporter;
}

ParameterClient* PdClientTransporter::pdClient() const
{
    return m_pdClient;
}

void PdClientTransporter::pushData(const char* data, size_t size) const
{
    if (m_transporter &&
        data &&
        size > 0)
    {
        // NOTE: no need to lock

        rcp_client_transporter_call_recv_cb(m_transporter, data, size);
    }
}

} // namespace rcp



