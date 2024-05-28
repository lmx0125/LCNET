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

struct connect_device {
	SOCKET sock;
	sockaddr_in sock_addr;
	unsigned long ID;
};

struct recv_device {
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
	void auto_print_recv(recv_device this_device);
	unsigned long connect(char *addr, UINT port);


	std::vector<connect_device *> connect_devices;
	std::vector<recv_async_struct *> auto_receive_list;
	std::vector<recv_device> recv_devices;
	recv_device newDevice;
	UINT PORT, err;
	SOCKET sock;
	sockaddr_in sock_addr;
private:
	WSADATA wsaData;
	bool is_service_on = true;
	std::mutex mtx;
};

#endif