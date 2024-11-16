#include "udp.h"

UDP::~UDP() {
	//disconnect (delete) all device
	while (device_list.size()) {
		Show_log(_DEBU, "is destory device | ID > " + std::to_string(device_list[0]->ID));
		delete_device(device_list[0]->ID);
	}

	//wait for all thread determinated
	should_recv_service_terminate = true;
	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	while (clean_up_threads.size())
		clean_up_threads[0].join();

	delete buffer;
}

void UDP::up(UINT port) {
	sock = socket(
		AF_INET,
		SOCK_DGRAM,
		IPPROTO_UDP
	);

	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
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

	//else init
	callback_func = default_recv_callback_func;
	set_auto_disconnect_time(5);

#ifdef _WIN32
	unsigned long on = 1;
	if (ioctlsocket(sock, FIONBIO, &on) != 0)
		Show_log(_ERROR, "can not set non-blocking mode");
#else
	int flag = fcntl(sock, F_GETFL, 0);
	if (flag < 0) {
		perror("fcntl F_GETFL fail");
		return;
	}
	if (fcntl(sock, F_SETFL, flag | SOCK_NONBLOCK) < 0) {
		perror("fcntl F_SETFL fail");
	}
#endif
}

unsigned long UDP::register_new_device(const char* addr, UINT port) {
	//check is device in this list
	unsigned long target_device_inet_addr = inet_addr(addr);
	ul target_device_port = htons(port);
	for (auto dev : device_list) {
		if (dev->sock_addr.sin_addr.s_addr == target_device_inet_addr &&
			dev->sock_addr.sin_port == target_device_port) {
			//Show_log(_MSG, "this device has been registered");
			return dev->ID;
		}
	}
	
	//create a new device
	device_struct* device = new device_struct;
	device->ID = IDgenerator.generateID();

	device->sock_addr.sin_port = (unsigned short)target_device_port;
	device->sock_addr.sin_family = AF_INET;
	device->sock_addr.sin_addr.s_addr = target_device_inet_addr;
	
	device_list.push_back(device);

	std::string str;
	str = "add a new device | ID > " + std::to_string(device->ID);
	Show_log(_MSG, str);

	clean_up_threads.push_back(std::thread(&UDP::package_auto_cleanup, this, device->ID));
	device->is_main_thread_terminated = false;
	
	return device->ID;
}

UDP& UDP::delete_device(unsigned long ID) {
	int device_no = 0;
	mtx.lock();
	for ( auto device : device_list) {
		if (device->ID == ID) {
			Show_log(_DEBU, "delete a device | ID > " + std::to_string(device->ID));
			IDgenerator.deleteID(device->ID);
			delete device;
			device = nullptr;
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

UDP& UDP::send(std::string msg, unsigned long ID, long data_size) {
	const char* char_msg = msg.c_str();
	return send(const_cast<char*>(char_msg), ID, data_size);
}

UDP& UDP::send(char* msg, unsigned long ID, long data_size) {
	device_struct* device = nullptr;

	this->mtx.lock();
	long long device_no = get_device_no_from_id(ID);
	if (device_no == -1) {
		this->mtx.unlock();
		return *this;
	}
	device = device_list[device_no];
	this->mtx.unlock();

	if (!data_size) {
		int err = sendto(
			sock,
			msg,
			(int)strlen(msg),
			0,
			(sockaddr*)&device->sock_addr,
			sizeof(device->sock_addr)
		);
	}else {
		int err = sendto(
			sock,
			msg,
			data_size,
			0,
			(sockaddr*)&device->sock_addr,
			sizeof(device->sock_addr)
		);
	}

	device->status.last_recv_time = time(nullptr);
	return *this;
}

void UDP::recv_service() {
	std::string str;
	
	Show_log(_DEBU, "recv_service_udp is running");

#ifdef _WIN32
	int sockaddr_size = sizeof(sockaddr), err = 0, device_no = 0;
#else
	socklen_t sockaddr_size = sizeof(sockaddr), err = 0, device_no = 0;
#endif

	timeval timeout_time;
	timeout_time.tv_sec = 1;
	timeout_time.tv_usec = 0;

	if (setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout_time,sizeof(timeout_time)) != 0) {
		Show_log(_ERROR, "can not set socket recv mode");
	}

	while (recv_service_status) {
		while (true) {
			err = recvfrom(
				sock,
				buffer,
				4096,
				0,
				(sockaddr*)&device->sock_addr,
				&sockaddr_size
			);

			if (should_recv_service_terminate) {
				Show_log(_DEBU, "auto recv thread is terminating");
				return;
			}
			if ((int)err > 0)
				break;
		}

		buffer[err] = '\0';
		
		if (err == SOCKET_ERROR) {
#ifdef _WIN32
			str = "recv error , error code is " + std::to_string(WSAGetLastError());
			Show_log(_ERROR, str);
#else
			Show_log(_ERROR, "Socket error (UDP > auto recv)");
#endif
			return;
		}

		if (!is_device_in_device_list(device->sock_addr)) {
			register_new_device(
				inet_ntoa(device->sock_addr.sin_addr),
				ntohs(device->sock_addr.sin_port)
			);

			//show a log
			std::string IP(inet_ntoa(device->sock_addr.sin_addr));
			str = "add a new device | ip > " + IP + " | port > " + std::to_string(ntohs(device->sock_addr.sin_port));
			Show_log(_MSG, str);
		} else {
			mtx.lock();
			device = std::make_shared<device_struct>(*device_list[get_device_no_from_addr(device->sock_addr)]);
			mtx.unlock();
			device->status.last_recv_time = time(nullptr);
		}
		device->data.data_CS.push_back(buffer);
		device->status.last_recv_time = time(nullptr);

		//invoke the callback func
		this->callback_func(buffer, err, device.get(), 0, this, pass_parameter);
	}
}

int UDP::get_device_no_from_id(unsigned long ID) {
	int device_no = 0;
	for (auto device : device_list) {
		if (device->ID == ID)
			return device_no;
		device_no++;
	}
	Show_log(_ERROR, "invalid id | ID > " + std::to_string(ID));
	return -1;
}

int UDP::get_device_no_from_addr(sockaddr_in addr) {
	int device_no = 0;
	for (auto device : device_list) {
		if (device->sock_addr.sin_addr.s_addr == addr.sin_addr.s_addr &&
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
		if (device->sock_addr.sin_addr.s_addr == addr.sin_addr.s_addr &&
			device->sock_addr.sin_port == addr.sin_port) {
			return true;
		}
	}
	return false;
}

void UDP::package_auto_cleanup(ul ID) {
	//wait a few moments
	//Show_log(_DEBU, "clean thread is running | ID > " + std::to_string(ID));
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	mtx.lock();
	//check is device invalid
	long long device_no = get_device_no_from_id(ID);
	if (device_no == -1) {
		this->mtx.unlock();
		return;
	}
	device_struct* device = device_list[device_no];
	mtx.unlock();

	if (!device->status.is_enabled_auto_remove) {
		return;
	}

	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		if (device->is_main_thread_terminated)
			return; 
		if (device->status.last_recv_time < time(nullptr) - this->auto_disconnect_time) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			break;
		}
	}

	package_cleanup(device);
	delete_device(ID);

	//Show_log(_DEBU, "end thread | ID > " + std::to_string(device->ID));
}

void UDP::package_cleanup(device_struct* device) {
	device->data.data_bin.erase(device->data.data_bin.begin(), device->data.data_bin.end());
	device->data.data_CS.erase(device->data.data_CS.begin(), device->data.data_CS.end());
}

void UDP::set_udp_package_recv_callback(package_recv_callback_func callback_func) {
	this->callback_func = callback_func;
}

void UDP::default_recv_callback_func(char* data, long buffer_size, device_struct*, int status, UDP* udp, std::vector<void*> pass_va) {
	printf("[DEBU] [DEFAULT CALLBACK]\n");
	std::cout << data << std::endl;
	return;
}

//UDP& UDP::send_to(char* msg, char* addr, int port) {
//	
//}

void UDP::set_auto_disconnect_time(int seconds) {
	this->auto_disconnect_time = seconds;
}
