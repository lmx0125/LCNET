#pragma once

#ifndef _NET_P2P_
#define _NET_P2P_

#include <vector>
#include <thread>
#include <mutex>

#include "../encoder/encoder.h"
#include "../UDP/udp.h"
#include "../net.h"

#ifndef _P2P_DATA_STRUCT_
#define _P2P_DATA_STRUCT_
struct p2p_data_struct {
	std::vector<device_struct> p2p_network_device_list;
	char* data = nullptr;
	UINT package_type;
	UINT status;
	unsigned short flags;

	~p2p_data_struct() {
		//delete[] data;
	}
};
#endif

#ifndef _P2P_PACKAGE_TYPE_
#define _P2P_PACKAGE_TYPE_
#define _P2P_NETWORK_DEVICE_LIST_PACKAGE_ 1
#define _P2P_NETWORK_DATA_PACKAGE_ 2
#define _P2P_HEART_BEAT_PACKAGE_ 3
#define _P2P_NETWORK_BOARDCAST_PACKAGE_ 4
#define _P2P_NETWORK_STATUS_PACKAGE_ 5
#define _P2P_NETWORK_CONNECTION_PACKAGE_ 6
#endif // !_P2P_PACKAGE_TYPE_

#ifndef _P2P_CONNECTION_FLAGS_
#define _P2P_CONNECTION_FLAGS_
#define _P2P_CONNECT_FLAG_ 1
#define _P2P_DISCONNECT_FLAG_ 2
#endif // !_P2P_CONNECTION_FLAGS_

class P2P : LOG
{
#define _P2P "P2P"
public:
	P2P(NET* net);
	void p2p_core_service();
	void register_p2p_network(const char* addr = "127.0.0.1", int port = 0);
	void join_p2p_network(const char* addr = "127.0.0.1", int port = 0);
	void quit_p2p_network(ul p2p_network_ID);
	static void p2p_package_recv_callback(char* data, device_struct* device, int status, UDP* udp, std::vector<void*> pass_va);
	void p2p_send(p2p_data_struct p2p_data,ul ID);
	static void p2p_register_device(char* data, device_struct* device, int status, UDP* udp);
	static void register_p2p_device(ul ID, device_struct* device, UDP* udp, P2P* p2p);

	//p2p func
	void add_verify_server(const char* addr, int port);
	void p2p_send_join_package();

	void send_add_p2p_device_request(device_struct device, ul ID = 0);
	device_struct* get_p2p_device_by_id(ul ID);

	//p2p connection func
	static void on_recv_connect_request(char* data, device_struct* device, int status, UDP* udp);

	template<typename T,typename T2>	
	static bool is_id_in_list(T ID,std::vector<T2> list);
	
	//pack func
	static std::string encode_p2p_package(p2p_data_struct pack);
	static p2p_data_struct decode_p2p_package(std::string& pack_str);
	static std::string encode_device_struct(device_struct device);
	static device_struct decode_device_struct(std::string& device_str);
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
};

#endif