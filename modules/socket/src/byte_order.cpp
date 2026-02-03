#include "oink_judge/socket/byte_order.h"

#include <arpa/inet.h>

uint64_t hton64(uint64_t val) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return static_cast<int64_t>(htonl(val & 0xFFFFFFFF)) << 32 | htonl(val >> 32 & 0xFFFFFFFF); // NOLINT
#else
    return val;
#endif
}

uint64_t ntoh64(uint64_t val) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return static_cast<uint64_t>(ntohl(val & 0xFFFFFFFF)) << 32 | ntohl(val >> 32 & 0xFFFFFFFF); // NOLINT
#else
    return val;
#endif
}
