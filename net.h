#pragma once
#ifndef _NET_H_
#define _NET_H_

#ifdef _WIN32
#include <WinSock2.h> 
#include <atlstr.h>
#include <WS2tcpip.h>
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
#include <cstring>
#include "log/log.h"
#include "UDP/udp.h"
#include "IDgen/IDgen.h"

#ifndef _NET_TYPE_
#define _NET_TYPE_
typedef unsigned long ul;

#ifndef _WIN32
typedef int SOCKET;
typedef unsigned int UINT;
#endif

#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

#ifndef _NET_DEVICE_STATUS_
#define _NET_DEVICE_STATUS_
struct device_status_struct {
	bool to_cleanup;
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
	device_struct() {
		//LOG log;
		//log.Show_log(_DEBU, "created a device_struct");
		//printf("[CREATE] address > %p\n", this);
	}

	~device_struct() {
		//LOG log;
		this->data.data_bin.erase(
			this->data.data_bin.begin(),
			this->data.data_bin.end()
		);
		this->data.data_CS.erase(
			this->data.data_CS.begin(),
			this->data.data_CS.end()
		);
		//log.Show_log(_DEBU, "device_struct release");
		//printf("[DELETE] address > %p\n", this);
	}
};
#endif

typedef device_struct connect_device;
typedef device_struct recv_device;


struct msg {
	UINT type;
	std::string data_s;
	char data[4096];
};
 
struct recv_async_struct {
	device_struct* device;
	std::future<void> futu;
};

class NET : LOG {
public:
	NET(int port = 0,int udp_port = 0);
	~NET();
	static void Cleanup(int signum);
	void service();
	NET& send(const char* message, unsigned long ID);
	void auto_print_recv(recv_device* this_device);
	unsigned long connect(const char* addr, UINT port);
	void disconnect(unsigned long ID);
	int find_device(unsigned long ID, std::vector<device_struct*> list);
	int find_device(unsigned long ID, std::vector<recv_async_struct*> list);

	char* domain_to_IP(char* domain);
	char* domain_to_IP(std::string domain);

	std::vector<device_struct *> device_list;
	std::vector<recv_async_struct *> auto_receive_list;
	UINT PORT, err;
	SOCKET sock;
	sockaddr_in sock_addr;
	UDP udp;
#ifdef _WIN32
	WSADATA wsaData;
#endif
	bool is_service_on = true;
	std::mutex mtx;
	IDgen IDgenerator;
};

#endif