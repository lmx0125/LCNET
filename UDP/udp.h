#ifndef _LC_UDP_
#define _LC_UDP_

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
#include "../log.h"

#ifndef _DEVICE_STRUCT_
#define _DEVICE_STRUCT_
struct device_struct {
	SOCKET sock;
	sockaddr_in sock_addr;
	unsigned long ID;
};
#endif

class UDP : LOG {
public:
	unsigned long register_new_device(const char* addr, UINT port);
	void delete_device(unsigned long ID);
	
	std::mutex mtx;
	std::vector<device_struct*> device_list;
};

#endif