#pragma once

#ifndef _P2P_H_
#define _P2P_H_

#include <vector>
#include <thread>
#include <mutex>
#include <climits>

#include "../encoder/encoder.h"
#include "../UDP/udp.h"
#include "../net.h"
#include "../IDgen/IDgen.h"

#ifndef _P2P_DATA_STRUCT_
#define _P2P_DATA_STRUCT_

#define P2P_DATA_DEFAULT_SIZE 1480

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

#ifndef _P2P_PACKAGE_TYPE_
#define _P2P_PACKAGE_TYPE_
#define _P2P_INVALID_PACKAGE_ 0
#define _P2P_NETWORK_DEVICE_LIST_PACKAGE_ 1
#define _P2P_NETWORK_DATA_PACKAGE_ 2
#define _P2P_HEART_BEAT_PACKAGE_ 3
#define _P2P_NETWORK_BOARDCAST_PACKAGE_ 4
#define _P2P_NETWORK_STATUS_PACKAGE_ 5
#define _P2P_NETWORK_CONNECTION_PACKAGE_ 6
#define _P2P_HOLE_PUNCHING_PACKAGE_ 7
#define _P2P_REGISTRY_PACKAGE_ 8
#endif // !_P2P_PACKAGE_TYPE_

#ifndef _P2P_FLAGS_
#define _P2P_FLAGS_
#define _P2P_INVALID_FLAG_ 0
#define _P2P_CONNECT_FLAG_ 1
#define _P2P_DISCONNECT_FLAG_ 2
#define _P2P_CONNECT_INFORMATION_FLAG_ 3
#define _P2P_HOLE_PUNCHING_FLAG_ 4
#define _P2P_FROM_SERVER_FLAG_ 5
#define _P2P_FROM_CLIENT_FLAG_ 6
#define _P2P_UNKNOWN_DEVICE_FLAG_ 7
#define _P2P_DEVICE_AVAILABLE_FLAG_ 8
#endif // !_P2P_FLAGS_

#ifndef _P2P_PACKAGE_STATUS_
#define _P2P_PACKAGE_STATUS_
#define _DROP_ 1
#endif

struct p2p_data_struct {
	std::vector<device_struct> p2p_network_device_list;
	char* data = nullptr;
	UINT package_type = _P2P_INVALID_PACKAGE_;
	UINT status = 0;
	unsigned short flags = _P2P_INVALID_FLAG_;

	p2p_data_struct(size_t data_size = 0) {
		if (data_size == 0)
			data_size = P2P_DATA_DEFAULT_SIZE;
		data = (char*)malloc(data_size * sizeof(char));
		memset(data, '\0', data_size);
	}

	~p2p_data_struct() {
		if (data != nullptr) {
			free(data);
			data = nullptr;
		}
	}

	void set_data_size(size_t data_size) {
		if (data != nullptr)
			delete data;
		data = (char*)malloc(data_size * sizeof(char));
		memset(data, '\0', data_size);
	}
};
#endif

class P2P : LOG
{
#define _P2P "P2P"
public:
	P2P(NET* net);
	void p2p_core_service();
	void register_p2p_network(const char* addr = "127.0.0.1", int port = 0);
	void join_p2p_network_by_addr(const char* addr = "127.0.0.1", int port = 0);
	void quit_p2p_network();
	static void p2p_package_recv_callback(char* data, long buffer_size, device_struct* device, int status, UDP* udp, std::vector<void*> pass_va);
	void p2p_send(p2p_data_struct* p2p_data, ul ID);
	void p2p_send(p2p_data_struct* p2p_data, const char* addr = "127.0.0.1", int port = 0);

	//p2p func
	void add_verify_server(const char* addr, int port);
	void p2p_send_join_package();

	void send_add_p2p_device_request(device_struct device, ul ID = 0);
	device_struct* get_p2p_device_by_id(ul ID);

	//p2p connection func
	//recv
	static void on_recv_connect_request(char* data, device_struct* device, int status, UDP* udp, P2P* p2p);
	/* recv hole punching request from client ( or other server ) */
	static void on_recv_hole_punching_request(char* data, device_struct* device, int status, UDP* udp, P2P* p2p);
	/* recv hole punching response from client ( or other server ) */
	static void on_recv_hole_punching_response(char* data, device_struct* device, int status, UDP* udp, P2P* p2p);
	/* recv registry requrest from client */
	static void on_recv_registry_request(char* data, device_struct* device, int status, UDP* udp, P2P* p2p);
	/* recv registry response from server */
	static void on_recv_registry_response(char* data, device_struct* device, int status, UDP* udp, P2P* p2p);
	//send
	/* send registry request ( to server ) */
	void send_registry_request();
	/* send hole punching request to server */
	void send_hole_punching_request(const char* addr = nullptr, int port = 0);

	template<typename T,typename T2>	
	static bool is_id_in_list(T ID, std::vector<T2> list);
	
	//pack func
	static std::string encode_p2p_package(p2p_data_struct* pack);
	static p2p_data_struct* decode_p2p_package(std::string& pack_str);
	static std::string encode_device_struct(device_struct* device);
	static device_struct* decode_device_struct(std::string& device_str);
	static std::string encode_sockaddr_in(sockaddr_in addr);
	static sockaddr_in decode_sockaddr_in(std::string& addr);
private:
	std::vector<std::promise<void>> p2p_device_auto_heart_beat;
	std::mutex mtx;
	ul p2p_network_ID;
	sockaddr_in public_IP;
	std::vector<ul> p2p_device_list;
	NET* net{ nullptr };
	std::thread core_service;
	char* verify_server;
	unsigned short verify_server_port;
	ul verify_server_ID;
	IDgen IDgenerator;
};

#endif