#ifndef _LC_UDP_
#define _LC_UDP_

#ifdef _WIN32
#include <WinSock2.h> 
#else
#include <sys/socket.h>  // Linux cpp headers
#include <netinet/in.h>  // Linux cpp headers
#include <arpa/inet.h>   // Linux cpp headers
#endif

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <vector>
#include <signal.h>
#include <random>
#include <time.h>
#include <future>
#include <mutex>
#include <memory>
#include <thread>
#include <string>
#include "../log.h"

#ifndef _NET_TYPE_
#define _NET_TYPE_
typedef unsigned long ul;

#ifndef _WIN32
typedef int SOCKET;
typedef unsigned int UINT;
#endif

#endif

#ifndef _NET_DEVICE_STATUS_
#define _NET_DEVICE_STATUS_
struct device_status_struct {
	time_t last_recv_time;
};
#endif // _NET_DEVICE_STATUS_


#ifndef _DEVICE_RECV_DATA_STRUCT_
#define _DEVICE_RECV_DATA_STRUCT_
struct device_recv_data_struct {
	std::vector<std::string> data_CS;
	std::vector<std::vector<unsigned char>> data_bin;
};
#endif

#ifndef _DEVICE_STRUCT_
#define _DEVICE_STRUCT_
struct device_struct {
	SOCKET sock;
	sockaddr_in sock_addr;
	unsigned long ID;
	device_recv_data_struct data; 
	device_status_struct status;
};
#endif

class UDP : LOG {
public:
	UDP() = default;
	void up(UINT port);
	unsigned long register_new_device(const char* addr, UINT port);
	UDP& delete_device(unsigned long ID);
	UDP& send(std::string msg, unsigned long ID);
	void recv_service();
	int get_device_no_from_id(unsigned long ID);
	int get_device_no_from_addr(sockaddr_in addr);
	//int get_data_list_no_from_id(unsigned long ID);
	//int get_data_list_no_from_addr(sockaddr_in addr);
	bool is_device_in_device_list(sockaddr_in addr);
	
	void package_auto_cleanup(ul ID);
	void package_cleanup(ul ID);
	sockaddr_in listen_addr;
	SOCKET sock;
	//std::vector<device_recv_data_struct*> device_recv_data;
	std::mutex mtx;
	std::vector<device_struct*> device_list;
	bool recv_service_status = true;
};

#endif