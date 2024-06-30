#ifndef _NET_P2P_
#define _NET_P2P_

#include <vector>
#include <thread>
#include <mutex>

#include "../UDP/udp.h"
#include "../net.h"

#ifndef _P2P_DATA_STRUCT_
#define _P2P_DATA_STRUCT_
struct p2p_data_struct {
	std::vector<device_struct> p2p_network_device_list;
	char* data;
	UINT package_type;
	UINT status;
};
#endif

#ifndef _P2P_PACKAGE_TYPE_
#define _P2P_PACKAGE_TYPE_
#define _P2P_NETWORK_DEVICE_LIST_PACKAGE_ 1
#define _P2P_NETWORK_DATA_PACKAGE_ 2
#define _P2P_HEART_BEAT_PACKAGE_ 3
#define _P2P_NETWORK_BOARDCAST_PACKAGE_ 4
#define _P2P_NETWORK_STATUS_PACKAGE_ 5
#endif // !_P2P_PACKAGE_TYPE_


class P2P : LOG
{
#define _P2P "P2P"
public:
	P2P(NET* net);
	void p2p_core_service();
	void register_p2p_network(const char* addr = "127.0.0.1", int port = 0);
	void join_p2p_network(const char* addr = "127.0.0.1", int port = 0);
	void quit_p2p_network(ul p2p_network_ID);
	static void p2p_package_recv_callback(char* data, device_struct* device, int status);
	void p2p_send(p2p_data_struct p2p_data);

	void add_p2p_device(device_struct device, ul ID = 0);
	device_struct* get_p2p_device_by_id(ul ID);

	//this func will delete the mem of data 
	static p2p_data_struct char_2_p2p_data(char* data);
	static std::shared_ptr<char[]> p2p_data_2_char(p2p_data_struct data);
	
private:
	std::mutex mtx;
	ul p2p_network_ID;
	sockaddr_in public_IP;
	std::vector<ul> p2p_device_list;
	NET* net{ nullptr };
	std::thread core_service;
};

#endif