#pragma once
#ifndef _NET_
#define _NET_

#include <WinSock2.h> 
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
#include <atlstr.h>
#include "log.h"
#include "UDP/udp.h"

#ifndef _NET_TYPE_
#define _NET_TYPE_
typedef unsigned long ul;
#endif

#ifndef _DEVICE_RECV_DATA_STRUCT_
#define _DEVICE_RECV_DATA_STRUCT_
struct device_recv_data_struct {
	std::vector<CString> data_CS;
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
};
#endif

typedef device_struct connect_device;
typedef device_struct recv_device;


struct msg {
	UINT type;
	CString data_cs;
	char data[4096];
};
 
struct recv_async_struct {
	device_struct* device;
	std::future<void> futu;
};

class NET : LOG {
public:
	NET();
	static void Cleanup(int signum);
	void service();
	NET& send(CString message, unsigned long ID);
	void auto_print_recv(recv_device this_device);
	unsigned long connect(const char* addr, UINT port);
	void disconnect(unsigned long ID);
	int find_device(unsigned long ID, std::vector<device_struct*> list);
	int find_device(unsigned long ID, std::vector<recv_async_struct*> list);


	std::vector<device_struct *> device_list;
	std::vector<recv_async_struct *> auto_receive_list;
	UINT PORT, err;
	SOCKET sock;
	sockaddr_in sock_addr;

	UDP udp;
private:
	WSADATA wsaData;
	bool is_service_on = true;
	std::mutex mtx;
};

#endif