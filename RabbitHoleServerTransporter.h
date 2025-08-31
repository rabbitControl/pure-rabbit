#ifndef RABBITHOLESERVERTRANSPORTER_H
#define RABBITHOLESERVERTRANSPORTER_H

#include <m_pd.h>

#include <WebsocketClient.h>

#include <rcp_server_transporter.h>

using namespace scaryws;

namespace rcp {

class RabbitHoleServerTransporter
    : public WebsocketClient
{
public:
    RabbitHoleServerTransporter(t_pd* x, rcp_server* server);
    ~RabbitHoleServerTransporter();

    rcp_server_transporter* transporter() const;

    void send(const char* data, size_t data_size);

    void setInterval(int i);

public:
    // IClientSessionListener
    void connected() override;
    // void error(int code, const std::string& message) override;
    void disconnected(uint16_t code) override;
    void received(const char* data, size_t size) override;

private:
    t_pd* m_x{nullptr};
    rcp_server* m_rcpServer{nullptr};

    rcp_server_transporter* m_transporter{nullptr};

    // connection timer
    t_clock* m_connectionTimer{nullptr};
    int m_connectionInverval{2000};
    bool m_connected{false};
};

} // namespace rcp

#endif // RABBITHOLESERVERTRANSPORTER_H
