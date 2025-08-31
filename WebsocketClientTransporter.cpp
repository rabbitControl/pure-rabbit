#include "WebsocketClientTransporter.h"

#include <vector>

#include <rcp_memory.h>

#include "Threading.h"
#include "rabbit.client.h"

//
static void _pd_websocket_client_transporter_send(rcp_client_transporter* transporter, const char* data, size_t size)
{
    if (transporter &&
        transporter->user)
    {
        ((rcp::WebsocketClientTransporter*)transporter->user)->send(data, size);
    }
}

// synchronized from threaded transporter

static void pd_client_connected(t_pd *obj, void *data)
{
    if (obj != NULL)
    {
        t_rabbit_client_pd* x = (t_rabbit_client_pd*)obj;

        outlet_float(x->client_connected_out, 1);
    }
}

static void pd_client_disconnected(t_pd *obj, void *data)
{
    if (obj != NULL)
    {
        t_rabbit_client_pd* x = (t_rabbit_client_pd*)obj;

        outlet_float(x->client_connected_out, 0);
    }
}



namespace rcp
{

WebsocketClientTransporter::WebsocketClientTransporter(t_pd* x)
    : WebsocketClient()
    , m_x(x)
{
    binary(true);

    m_transporter = (rcp_client_transporter*)RCP_CALLOC(1, sizeof(rcp_client_transporter));

    if (m_transporter)
    {
        rcp_client_transporter_setup(m_transporter,
                                     _pd_websocket_client_transporter_send);

        m_transporter->user = this;
    }
}

WebsocketClientTransporter::~WebsocketClientTransporter()
{
    if (m_transporter)
    {
        RCP_FREE(m_transporter);
        m_transporter = nullptr;
    }
}

void WebsocketClientTransporter::send(const char *data, size_t size)
{
    std::vector<char> d(size);

    for (int i = 0; i < size; ++i)
    {
        d[i] = data[i];
    }

    WebsocketClient::send(d);
}

// IClientTransporter
rcp_client_transporter* WebsocketClientTransporter::transporter() const
{
    return m_transporter;
}

void WebsocketClientTransporter::connect(const std::string& address)
{
    WebsocketClient::connect(address);
}

void WebsocketClientTransporter::disconnect()
{
    WebsocketClient::disconnect();
}


// threaded
void WebsocketClientTransporter::connected()
{
    rcp_client_transporter_call_connected_cb(m_transporter);

    pd_queue_mess(&pd_maininstance, (t_pd*)m_x, NULL, pd_client_connected);
}

void WebsocketClientTransporter::disconnected(uint16_t code)
{
    rcp_client_transporter_call_disconnected_cb(m_transporter);

    pd_queue_mess(&pd_maininstance, (t_pd*)m_x, NULL, pd_client_disconnected);
}

void WebsocketClientTransporter::received(const char* data, size_t size)
{
    // handle binary data

    if (m_transporter &&
        data  &&
        size > 0)
    {
        std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

        rcp_client_transporter_call_recv_cb(m_transporter, data, size);
    }
}


} // namespace rcp
