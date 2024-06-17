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

	std::string str;
	str = "UDP port > " + std::to_string(port);
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

	std::string str;
	str = "add a new device [ ID > " + std::to_string(device->ID) + "]";
	Show_log(_MSG, str);

	std::thread auto_cleanup_thread(&UDP::package_auto_cleanup, this, device->ID);
	auto_cleanup_thread.detach();
	
	return device->ID;
}

UDP& UDP::delete_device(unsigned long ID) {
	mtx.lock();

	int device_no = 0;
	for ( auto device : device_list) {
		if (device->ID == ID) {
			Show_log(_DEBU, "delete a device");
			delete device;
			device_list.erase(
				device_list.begin() + device_no
			);
			mtx.unlock();
			return *this;
		}
		device_no++;
	}
	mtx.unlock();
	return *this;
}

UDP& UDP::send(std::string msg, unsigned long ID) {
	device_struct* device = nullptr;
	for (auto temp : device_list)
		if (temp->ID == ID) {
			device = temp;
			break;
		}
	
	int err = sendto(
		device->sock,
		msg.c_str(),
		strlen(msg.c_str()),
		0,
		(sockaddr*)&device->sock_addr,
		sizeof(device->sock_addr)
	);
	return *this;
}

void UDP::recv_service() {
	std::string str;
	
	Show_log(_DEBU, "recv_service_udp is running");

	char* buffer = new char[4097];
	int sockaddr_size = sizeof(sockaddr), err;
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
#ifdef _WIN32
			str = "recv error , error code is " + std::to_string(WSAGetLastError());
			Show_log(_ERROR, str);
#else
			Show_log(_ERROR, "Socket error (UDP > auto recv)");
#endif
			return;
		}

		buffer[err] = '\0';

		//str = "recv msg > ";
		//str += buffer;
		//Show_log(_MSG, str);

		if (!is_device_in_device_list(device->sock_addr)) {
			register_new_device(
				inet_ntoa(device->sock_addr.sin_addr),
				ntohs(device->sock_addr.sin_port)
			);
			str = "add a new device | ip > " + std::to_string(
				device->sock_addr.sin_addr.S_un.S_addr
			) + " | port > " + std::to_string(
				device->sock_addr.sin_port
			);
			Show_log(_MSG, str);
		}
		else {
			int device_no = get_device_no_from_addr(device->sock_addr);
			delete device;
			device = device_list[device_no];
		}
		mtx.lock();
		device = device_list[get_device_no_from_addr(device->sock_addr)];
		
		device->data.data_CS.push_back(buffer);
		device->status.last_recv_time = time(nullptr);

		mtx.unlock();
	}
}

int UDP::get_device_no_from_id(unsigned long ID) {
	int device_no = 0;
	for (auto device : device_list) {
		if (device->ID == ID)
			return device_no;
		device_no++;
	}
	Show_log(_ERROR, "invalid id");
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
	Show_log(_ERROR, "invalid addr");
	return -1;
}

//int UDP::get_data_list_no_from_id(unsigned long ID) {
//	int device_no = 0;
//	for (auto device : device_list) {
//		if (device->device.ID == ID)
//			return device_no;
//		device_no++;
//	}
//	return -1;
//}
//
//int UDP::get_data_list_no_from_addr(sockaddr_in addr) {
//	int device_no = 0;
//	for (auto device : device_list) {
//		if (device->device.sock_addr.sin_addr.S_un.S_addr == addr.sin_addr.S_un.S_addr &&
//			device->device.sock_addr.sin_port == addr.sin_port)
//			return device_no;
//		device_no++;
//	}
//	return -1;
//}

bool UDP::is_device_in_device_list(sockaddr_in addr) {
	for (auto device : device_list) {
		if (device->sock_addr.sin_addr.S_un.S_addr == addr.sin_addr.S_un.S_addr &&
			device->sock_addr.sin_port == addr.sin_port) {
			return true;
		}
	}
	return false;
}

void UDP::package_auto_cleanup(ul ID) {
	this->mtx.lock();
	device_struct* device = device_list[get_device_no_from_id(ID)];
	this->mtx.unlock();

	while (true) {
		Sleep(3000);
		if (device->status.last_recv_time < time(nullptr) - 5)
			break;
	}
	package_cleanup(ID);
	delete_device(ID);
}

void UDP::package_cleanup(ul ID) {
	this->mtx.lock();
	device_struct* device = device_list[get_device_no_from_id(ID)];
	this->mtx.unlock();

	device->data.data_bin.erase(device->data.data_bin.begin(), device->data.data_bin.end());
	device->data.data_CS.erase(device->data.data_CS.begin(), device->data.data_CS.end());
}