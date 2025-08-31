#ifndef WEBSOCKETCLIENTTRANSPORTER_H
#define WEBSOCKETCLIENTTRANSPORTER_H

#include <m_pd.h>

#include <rcp_client_transporter.h>

#include <WebsocketClient.h>

#include "IClientTransporter.h"

using namespace scaryws;

namespace rcp
{

class WebsocketClientTransporter
    : public WebsocketClient
    , public IClientTransporter
{
public:
    WebsocketClientTransporter(t_pd* x);
    ~WebsocketClientTransporter();

    void send(const char* data, size_t size);

public:
    // IClientTransporter
    rcp_client_transporter* transporter() const override;
    void connect(const std::string& address) override;
    void disconnect() override;

public:
    // IClientSessionListener
    void connected() override;
    // void error(int code, const std::string& message) override;
    void disconnected(uint16_t code) override;
    void received(const char* data, size_t size) override;

private:
    t_pd* m_x{nullptr};
    rcp_client_transporter* m_transporter{nullptr};
};

} // namespace rcp


#endif // WEBSOCKETCLIENTTRANSPORTER_H
