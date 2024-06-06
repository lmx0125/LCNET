#include "udp.h"

unsigned long UDP::register_new_device(const char* addr, UINT port) {
	//create a new device
	device_struct* device = new device_struct;
	device->ID =
		(rand() % 10000) * 100000000 +
		(rand() % 10000) * 10000 +
		(rand() % 10000) * 1;

	device->sock = socket(
		AF_INET,
		SOCK_DGRAM,
		IPPROTO_UDP
	);

	device->sock_addr.sin_port = htons(port);
	device->sock_addr.sin_family = AF_INET;
	device->sock_addr.sin_addr.S_un.S_addr = inet_addr(addr);
	
	device_list.push_back(device);

	CString str;
	str.Format("add a new device [ ID > %ul ]", device->ID);
	Show_log(_MSG, str);
	
	return device->ID;
}

void UDP::delete_device(unsigned long ID) {
	mtx.lock();

	int device_no = 0;
	for ( auto device : device_list) {
		if (device->ID == ID) {
			delete device;
			device_list.erase(
				device_list.begin() + device_no
			);
			mtx.unlock();
			return;
		}
		device_no++;
	}

	mtx.unlock();
}

void UDP::send(CString msg, unsigned long ID) {
	device_struct* device = nullptr;
	for (auto temp : device_list)
		if (temp->ID == ID) {
			device = temp;
			break;
		}
	
	sendto(
		device->sock,
		msg,
		sizeof(msg),
		0,
		(sockaddr*)&device->sock_addr,
		sizeof(device->sock_addr)
	);
}