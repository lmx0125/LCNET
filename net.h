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
#include "log.h"

struct device {
	SOCKET sock;
	sockaddr_in sock_addr;
	unsigned long ID;
};

struct msg {
	UINT type;
	CString data_cs;
	char data[4096];
};
 
struct recv_async_struct {
	unsigned long ID;
	std::future<void> futu;
};

class NET : LOG {
public:
	NET();
	static void Cleanup(int signum);
	void service();
	void send(CString message, unsigned long ID);
	void auto_print_recv(device this_device);
	void connect(char *addr, UINT port);
	
	std::vector<recv_async_struct *> auto_receive_list;
	std::vector<device> connect_devices;
	device newDevice;
	UINT PORT, err;
	SOCKET sock;
	sockaddr_in sock_addr;
private:
	WSADATA wsaData;
	bool is_service_on = true;
	std::mutex mtx;
};

#endif