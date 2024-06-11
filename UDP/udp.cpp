#include "udp.h"

void UDP::up(UINT port) {
	sock = socket(
		AF_INET,
		SOCK_DGRAM,
		IPPROTO_UDP
	);

	listen_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_port = htons(port);

	bind(
		sock,
		(sockaddr*)&listen_addr,
		sizeof(listen_addr)
	);

	CString str;
	str.Format("UDP port is > %d", port);
	Show_log(_MSG, str);

	std::thread recv_thread(&UDP::recv_service,this);
	recv_thread.detach();
}

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
	
	int err = sendto(
		device->sock,
		msg,
		sizeof(msg),
		0,
		(sockaddr*)&device->sock_addr,
		sizeof(device->sock_addr)
	);
}

void UDP::recv_service() {
	CString str;
	
	Show_log(_DEBU, "recv_service_udp is running");

	char* buffer = new char[4097];
	int sockaddr_size = sizeof(sockaddr), err, device_no;
	while (recv_service_status) {
		device_struct* device = new device_struct;
		err = recvfrom(
			sock,
			buffer,
			4097,
			0,
			(sockaddr*)&device->sock_addr,
			&sockaddr_size
		);
		
		if (err == SOCKET_ERROR) {
			str.Format("WSA error , error code is %d ", WSAGetLastError());
			Show_log(_ERROR, str);
			return;
		}

		buffer[err] = '\0';

		str.Format("recv msg > %s", buffer);
		Show_log(_MSG, str);

		mtx.lock();

		if (!is_device_in_recv_list(device->sock_addr)) {
			device_recv_data_struct* ptr = new device_recv_data_struct;
			ptr->device.ID =
				(rand() % 10000) * 100000000 +
				(rand() % 10000) * 10000 +
				(rand() % 10000) * 1;
			memcpy(&ptr->device, device, sizeof(*device));
			device_recv_data.push_back(ptr);
			str.Format("add a new device | ip > %ul | port > %d",
				device->sock_addr.sin_addr.S_un.S_addr, 
				device->sock_addr.sin_port
			);
			Show_log(_MSG, str);
		}

		device_recv_data[
			get_data_list_no_from_addr(device->sock_addr)
		]->data_CS.push_back(buffer);

		mtx.unlock();

		delete device;
	}
}

int UDP::get_device_no_from_id(unsigned long ID) {
	int device_no = 0;
	for (auto device : device_list) {
		if (device->ID == ID)
			return device_no;
		device_no++;
	}
	return -1;
}

int UDP::get_device_no_from_addr(sockaddr_in addr) {
	int device_no = 0;
	for (auto device : device_list) {
		if (device->sock_addr.sin_addr.S_un.S_addr == addr.sin_addr.S_un.S_addr &&
			device->sock_addr.sin_port == addr.sin_port)
			return device_no;
		device_no++;
	}
	return -1;
}

int UDP::get_data_list_no_from_id(unsigned long ID) {
	int device_no = 0;
	for (auto device : device_recv_data) {
		if (device->device.ID == ID)
			return device_no;
		device_no++;
	}
	return -1;
}

int UDP::get_data_list_no_from_addr(sockaddr_in addr) {
	int device_no = 0;
	for (auto device : device_recv_data) {
		if (device->device.sock_addr.sin_addr.S_un.S_addr == addr.sin_addr.S_un.S_addr &&
			device->device.sock_addr.sin_port == addr.sin_port)
			return device_no;
		device_no++;
	}
	return -1;
}

bool UDP::is_device_in_recv_list(sockaddr_in addr) {
	for (auto device : device_recv_data) {
		if (device->device.sock_addr.sin_addr.S_un.S_addr == addr.sin_addr.S_un.S_addr &&
			device->device.sock_addr.sin_port == addr.sin_port) {
			return true;
		}
	}
	return false;
}