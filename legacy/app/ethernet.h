#if 0
#ifndef ETHERNET_APP
#define ETHERNET_APP

#define MODULE_STR "[ETHERNET]"

enum {
    FIX_IP,
    DHCP
};

class RFFEServer {
public:
    RFFEServer(CDCE906 &pll, int addressing, int port);
    void eth_init();
    void eth_connect();


    const char* get_ip_address();
    const char* get_gateway();
    const char* get_netmask();
private:
    CDCE906 _pll;

    EthernetInterface _net;
    TCPSocket _client;
    SocketAddress _client_addr;
    TCPServer _server;
    Ethernet _cable_status;

    int _port;
    int _addressing;
};

#endif
#endif
