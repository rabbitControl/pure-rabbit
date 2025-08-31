#ifndef WEBSOCKETSERVERTRANSPORTER_H
#define WEBSOCKETSERVERTRANSPORTER_H

#include <m_pd.h>

#include <WebsocketServer.h>

#include <rcp_server_transporter.h>

#include "IServerTransporter.h"

using namespace scaryws;

namespace rcp
{

class WebsocketServerTransporter
    : public WebsocketServer
    , public IServerTransporter
{
public:
    WebsocketServerTransporter(t_pd* x);
    ~WebsocketServerTransporter();

    void sendToOne(const char* data, size_t size, void* id);
    void sendToAll(const char* data, size_t size, void* excludeId);

public:
    // IServerTransporter
    rcp_server_transporter* transporter() const override;
    void bind(uint16_t port) override;
    void unbind() override;
    uint16_t port() const override;
    bool isListening() const override;
    size_t clientCount() const override;

public:
    // IServerSessionListener
    virtual void listening() override;
    virtual void closed() override;
    virtual void clientConnected(void* client) override;
    virtual void clientDisconnected(void* client) override;
    virtual void received(const char* data, size_t size, void* client) override;
    // virtual void received(const std::string& msg, void* client) override; // ignore text data

private:
    t_pd* m_x{nullptr};
    rcp_server_transporter* m_transporter{nullptr};
};

} // namespace rcp


#endif // WEBSOCKETSERVERTRANSPORTER_H
