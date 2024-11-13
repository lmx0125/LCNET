#ifndef _STUN_H_
#define _STUN_H_

#include "../net.h"
#include "../UDP/udp.h"
#include <iomanip>

const uint32_t MAGIC_COOKIE = 0x2112A442;
#define STUN_HEADER_SIZE 20

#ifndef ARRAYSIZE
#define ARRAYSIZE(arr) (sizeof(arr)/sizeof((arr)[0]))  // Macro to calculate number of elements in array
#endif

enum message_type {
    Binding_Request = 0x0001,
    Binding_Response = 0x0101,
    Binding_Error_Response = 0x0111,
    Shared_Secret_Request = 0x0002,
    Shared_Secret_Response = 0x0102,
    Shared_Secret_Error_Response = 0x0112
};

enum attribute_type {
    mapped_address = 0x0001,
    response_address = 0x0002,
    change_request = 0x0003,
    source_address = 0x0004,
    changed_address = 0x0005,
    username = 0x0006,
    password = 0x0007,
    message_integrity = 0x0008,
    error_code = 0x0009,
    unknown_attributes = 0x000a,
    reflected_from = 0x000b
};

class STUN {
public:
    STUN(NET* net);
    ~STUN();
    void build_stun_package_header(uint16_t msg_type, uint16_t att_type);
    void gen_transaction_id();
    void send_stun_bind_request(const char* addr, UINT port);
    static void stun_recv_callback(char* data, long buffer_size, device_struct*, int status, UDP* udp, std::vector<void*>);
    void stun_callback_clear();

    uint16_t get_end_port();
    std::string get_end_IP();

    uint8_t* recv_data = nullptr;
    NET* net;
    uint8_t header[STUN_HEADER_SIZE];
    uint8_t transaction_id[12] = { 0 };
    ul STUN_SERVER_ID = 0;
    std::vector<void*> old_parameter;
    package_recv_callback_func old_callback_func = nullptr;

};

#endif