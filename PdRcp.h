#ifndef PDRCP_H
#define PDRCP_H

#define RCP_PD_VERSION "2.0.0"

/*
 Version changes:

 2.0.0:
    - handle threading (needs Pd >= 0.56.0)
    - rename rcp externals to rabbit (e.g.: rcp.server -> rabbit.server)
    - add backward compatibility patches (breaks raw server and client)
    - remove flext
    - use boost beast for websockets (remove old websocketpp)
    - boost version 1.88.0
    - individual externals
    - statically link dependecies (boost_url, OpenSSL)
    - increase OpenSSL to 3.5.2
    - prefix "get" for getter (e.g.: order -> getorder)
    - rabbit.client to prefer "connect" and "disconnect"
    - info outlet to output anything (instead of list)    
    - remove rcp.debug
    - remove ws.client / ws.server (moved to seperate project)

 */

class PdRcp
{
public:
    static void postVersion();
    static void rabbitPost(const char* msg);
    static void rabbitPostOneline(const char* msg);
    static void postRabbitcontrolInit();

private:
    static bool toasted;

};

#endif // PARAMETERCLIENT_H
