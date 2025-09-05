#include "GlobalRFFE.hpp"


GlobalRFFE::GlobalRFFE()
    : _le(LE_DAT31R5SP)
    , _clk(CLK_DAT31R5SP)
    , _dtA(DATA_A_DAT31R5SP)
    , _dtB(DATA_B_DAT31R5SP)
    , _dtC(DATA_C_DAT31R5SP)
    , _dtD(DATA_D_DAT31R5SP)
    , _feram_wp(FERAM_WP)
    , _dat31r5sp(&_le, &_clk, &_dtA, &_dtB, &_dtC, &_dtD)
    , _feram(FERAM_SDA, FERAM_SCL, 0xA0, 2048, 2048)
    , _cs_ac(LM71_CS_AC)
    , _cs_bd(LM71_CS_BD)
    , _spi(LM71_MOSI1, LM71_MISO1, LM71_SCLK1)
    , _lm71_ac(_spi, &_cs_ac)
    , _lm71_bd(_spi, &_cs_bd)
{
}

static inline int hex_nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

/**
 * Parse network textual address.
 * - len==4  -> dotted-quad IPv4 "a.b.c.d"
 * - len==6  -> MAC "xx:xx:xx:xx:xx:xx" (hex pairs, ':' separator)
 */
static bool parse_netaddr(const char* s, uint8_t* out, size_t len) {
    if (!s || !out) return false;

    // IPv4, Mask, Gateway
    if (len == 4) {
        unsigned acc = 0;
        int dots = 0;
        bool have_digit = false;

        for (const char* p = s; ; ++p) {
            const char c = *p;

            if (c >= '0' && c <= '9') {
                have_digit = true;
                acc = acc * 10u + unsigned(c - '0');
                if (acc > 255u) return false;
            } else if (c == '.') {
                if (!have_digit || dots >= 3) return false; // empty or dots > 3
                out[dots++] = static_cast<uint8_t>(acc);
                acc = 0;
                have_digit = false;
            } else if (c == '\0') {
                if (dots != 3 || !have_digit) return false;
                out[3] = static_cast<uint8_t>(acc);
                return true;
            } else {
                return false;
            }
        }
    }

    // MAC address
    if (len == 6) {
        uint8_t acc = 0;
        int digits = 0;
        int octets = 0;
        char sep = 0;

        for (const char* p = s; ; ++p) {
            const char c = *p;

            if (c == ':') {
                if (digits == 0) return false;
                if (sep == 0) sep = c;
                else if (c != sep) return false;

                out[octets++] = acc;
                if (octets >= 6) return false;

                acc = 0;
                digits = 0;
                continue;
            }

            if (c == '\0') {
                if (digits == 0) return false;
                out[octets++] = acc;
                return (octets == 6);
            }

            const int h = hex_nibble(c);
            if (h < 0) return false;

            if (digits == 2) return false;
            acc = static_cast<uint8_t>((acc << 4) | h);
            ++digits;
        }
    }

    return false;
}


int GlobalRFFE::stream_feram_raw() {
    constexpr size_t kCap  = 2048;
    constexpr size_t kLine = 16;
    unsigned char buf[kLine];

    debug("\n");
    for (size_t off = 0; off < kCap; off += kLine) {
        ssize_t n = _feram.read(off, reinterpret_cast<char*>(buf), kLine);
        if (n != static_cast<ssize_t>(kLine)) {
            printf("[FERAM] Falha de leitura @0x%04X (ret=%d)\n",
                   static_cast<unsigned>(off), static_cast<int>(n));
            return -1;
        }

        // Offset
        debug("%04X: ", static_cast<unsigned>(off));

        // Hex
        for (size_t i = 0; i < kLine; ++i) {
            debug("%02X ", static_cast<unsigned>(buf[i]));
        }

        debug("|\n");
    }
    return static_cast<int>(kCap);
}

int GlobalRFFE::get_measure_temp_ac(float &value)
{
    _lm71_ac.lm71_mode(0);

    _lm71_ac.lm71_readtemp(value);

    auto in_range = [](float x) { return (x >= -55.0f && x <= 150.0f); };

    if(!in_range(value))
    {
        ThisThread::sleep_for(270ms);
        _lm71_ac.lm71_readtemp(value);
        if(!in_range(value))
        {
            return -1;
        }
    }
    return 0;
}

int GlobalRFFE::get_measure_temp_bd(float &value)
{
    _lm71_bd.lm71_mode(0);

    _lm71_bd.lm71_readtemp(value);

    auto in_range = [](float x) { return (x >= -55.0f && x <= 150.0f); };

    if(!in_range(value))
    {
        ThisThread::sleep_for(270ms);
        _lm71_bd.lm71_readtemp(value);
        if(!in_range(value))
        {
            return -1;
        }
    }
    return 0;
}

nsapi_error_t GlobalRFFE::config_get_eth_mode(EthernetInterface & net)
{
    eth_addr_mode_t mode = ETH_ADDR_MODE_DHCP;
    config_get_eth_addressing(&mode);

    auto try_static = [this](EthernetInterface & n) -> nsapi_error_t
    {
        char ip[16] = {};
        char mask[16] = {};
        char gw[16]= {};
        uint8_t mac[6] = {};

        int nip = config_get_ipv4_addr(ip, sizeof(ip));
        int nmask = config_get_mask_addr(mask, sizeof(mask));
        int ngate = config_get_gateway_addr(gw, sizeof(gw));
        int nmac = config_get_mac_addr(mac, sizeof(mac));

        const bool static_ip = (nip > 0 && nmask > 0 && ngate > 0 && nmac > 0);
        if(!static_ip) return NSAPI_ERROR_PARAMETER;

        n.set_dhcp(false);
        nsapi_error_t st = n.set_network(ip, mask, gw);
        if(st != NSAPI_ERROR_OK) return st;

        st = n.set_mac_address(mac, sizeof(mac));
        if (st != NSAPI_ERROR_OK) return st;

        return NSAPI_ERROR_OK;
    };

    nsapi_error_t st = NSAPI_ERROR_OK;

    switch(mode)
    {
        case ETH_ADDR_MODE_STATIC:
            st = try_static(net);
            if(st != NSAPI_ERROR_OK)
            {
                debug("[GlobalRFFE][WARN] Static failed (%d); falling back to DHCP.\n", st);
                net.set_dhcp(true);
            }
            break;

        case ETH_ADDR_MODE_DHCP:
            net.set_dhcp(true);
            break;

        case ETH_ADDR_MODE_NONE:
        default:
            st = try_static(net);
            if(st != NSAPI_ERROR_OK)
            {
                net.set_dhcp(true);
                st = NSAPI_ERROR_OK;
            }
            break;
    }

    return st;
}


int GlobalRFFE::config_get_mac_addr(uint8_t * mac, size_t len)
{
    if (!mac || len == 0) return -1;

    uint8_t data[6] = {};
    if (_feram.read(_mac_addr_offset, reinterpret_cast<char*>(data), 6) != 6) {
        debug("[ERROR][GlobalRFFE] Reading IPv4 from FRAM failed.\n");
        mac[0] = '\0';
        return -1;
    }
    memcpy(mac, data, len);

    return len;
}

int GlobalRFFE::config_set_mac_addr(const char * mac_str)
{
    if (!mac_str) return -1;

    uint8_t mac[6];
    if (!parse_netaddr(mac_str, mac, 6)) {
        debug("[ERROR][GlobalRFFE] Invalid IPv4 string.\n");
        return -1;
    }

    if (_feram.write(_mac_addr_offset, reinterpret_cast<const char*>(mac), 6) != 6) {
        debug("[ERROR][GlobalRFFE] Writing IPv4 failed.\n");
        return -1;
    }

    return 6;
}

int GlobalRFFE::config_get_eth_addressing(eth_addr_mode_t* addr_mode)
{
    uint8_t buf = 0;

    if(_feram.read(_eth_addr_offset, reinterpret_cast<char*>(&buf), 1) != 1)
    {
        debug("[ERROR][GlobalRFFE] Reading Version from FRAM failed.\n");
        return -1;
    }

    switch (buf)
    {
    case 0:
        *addr_mode = ETH_ADDR_MODE_STATIC;
        break;
    case 1:
        *addr_mode = ETH_ADDR_MODE_DHCP;
        break;
    default:
        *addr_mode = ETH_ADDR_MODE_NONE;
        break;
    }

    return 0;
}

int GlobalRFFE::config_set_eth_addressing(eth_addr_mode_t addr_mode)
{
    uint8_t buf = 0;

    switch (addr_mode)
    {
        case ETH_ADDR_MODE_STATIC:
            buf = 0;
            break;
        case ETH_ADDR_MODE_DHCP:
            buf = 1;
            break;
        default:
            buf = 2;
            break;
    }

    if(_feram.write(_eth_addr_offset, reinterpret_cast<char*>(&buf), 1) != 1)
    {
        debug("[ERROR][GlobalRFFE] Reading Version from FRAM failed.\n");
        return -1;
    }

    return 0;
}

int GlobalRFFE::config_get_version()
{
    char version[1];

    if(_feram.read(_config_version_offset, reinterpret_cast<char*>(&version),  1) != 1){
        debug("[ERROR][GlobalRFFE] Reading Version from FRAM failed.\n");
        return -1;
    }

    return 0;
}


float GlobalRFFE::config_get_attenuation()
{
    int32_t raw = 0;
    ssize_t n = _feram.read(_attenuation_offset,
                            reinterpret_cast<char*>(&raw),
                            sizeof(raw));
    if (n != static_cast<ssize_t>(sizeof(raw))) {
        debug("[ERROR][GlobalRFFE] Reading Attenuation from FRAM failed.\n");
        return -1;
    }

    float att = static_cast<float>(raw) / 65536.0f;

    return att;
}

int GlobalRFFE::config_set_attenuation(float att)
{
    _att = att;
    if(!_dat31r5sp.is_valid()){
        debug("[GlobalRFFE][ERROR] DAT31R5SP pin validation failed. Check DigitalOut initialization.\n");
    };
    _dat31r5sp.set_attenuation(att);

    const double scaled = static_cast<double>(att) * 65536.0;
    int32_t raw = static_cast<int32_t>(std::llrint(scaled));

    ssize_t n = _feram.write(_attenuation_offset,
                            reinterpret_cast<char*>(&raw),
                            sizeof(raw));
    if (n != static_cast<ssize_t>(sizeof(raw))) {
        debug("[ERROR][GlobalRFFE] Writing Attenuation FRAM failed.\n");
        return -1;
    }

    return 0;
}


int GlobalRFFE::config_get_ipv4_addr(char * ip, size_t len)
{
    if (!ip || len == 0) return -1;

    uint8_t data[4] = {};
    if (_feram.read(_ip_addr_offset, reinterpret_cast<char*>(data), 4) != 4) {
        debug("[ERROR][GlobalRFFE] Reading IPv4 from FRAM failed.\n");
        if (len) ip[0] = '\0';
        return -1;
    }
    int n = std::snprintf(ip, len, "%u.%u.%u.%u", data[0], data[1], data[2], data[3]);

    return n;
}


int GlobalRFFE::config_set_ipv4_addr(const char * ip_str)
{

    if (!ip_str) return -1;

    uint8_t ip[4];
    if (!parse_netaddr(ip_str, ip, 4)) {
        debug("[ERROR][GlobalRFFE] Invalid IPv4 string.\n");
        return -1;
    }


    if (_feram.write(_ip_addr_offset, reinterpret_cast<const char*>(ip), 4) != 4) {
        debug("[ERROR][GlobalRFFE] Writing IPv4 failed.\n");
        return -1;
    }

    return 0;
}

int GlobalRFFE::config_set_mask_addr(const char * mask_str)
{

    if (!mask_str) return -1;

    uint8_t mask[4];
    if (!parse_netaddr(mask_str, mask, 4)) {
        debug("[ERROR][GlobalRFFE] Invalid Mask string.\n");
        return -1;
    }


    if (_feram.write(_ip_addr_offset, reinterpret_cast<const char*>(mask), 4) != 4) {
        debug("[ERROR][GlobalRFFE] Writing Mask failed.\n");
        return -1;
    }
    return 4;
}

int GlobalRFFE::config_set_gateway_addr(const char * gt_str)
{

    if (!gt_str) return -1;

    uint8_t gt[4];
    if (!parse_netaddr(gt_str, gt, 4)) {
        debug("[ERROR][GlobalRFFE] Invalid Gateway string.\n");
        return -1;
    }


    if (_feram.write(_ip_addr_offset, reinterpret_cast<const char*>(gt), 4) != 4) {
        debug("[ERROR][GlobalRFFE] Writing Gateway failed.\n");
        return -1;
    }
    return 4;
}

int GlobalRFFE::config_get_mask_addr(char * mask, size_t len)
{
    if (!mask || len == 0) return -1;

    char data[4] = {};
    if (_feram.read(_mask_addr_offset, reinterpret_cast<char*>(data), 4) != 4) {
        debug("[ERROR][GlobalRFFE] Reading Mask from FRAM failed.\n");
        if (len) mask[0] = '\0';
        return -1;
    }

    int n = std::snprintf(mask, len, "%u.%u.%u.%u", data[0], data[1], data[2], data[3]);

    return n;
}


int GlobalRFFE::config_get_gateway_addr(char * gateway, size_t len)
{
    if (!gateway || len == 0) return -1;

    uint8_t data[4] = {};
    if (_feram.read(_gateway_addr_offset, reinterpret_cast<char*>(data), 4) != 4) {
        debug("[ERROR][GlobalRFFE] Reading Gateway from FRAM failed.\n");
        if (len) gateway[0] = '\0';
        return -1;
    }

    int n = std::snprintf(gateway, len, "%u.%u.%u.%u",
                          data[0], data[1], data[2], data[3]);

    return n;
}

