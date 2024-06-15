#include "net.h"

NET::NET() {
	//Set exit signal
	signal(SIGABRT,&NET::Cleanup);
	signal(SIGFPE, &NET::Cleanup);
	signal(SIGILL, &NET::Cleanup);
	signal(SIGSEGV,&NET::Cleanup);
	signal(SIGINT, &NET::Cleanup);
	signal(SIGTERM,&NET::Cleanup);

	//WSA init
	WSAStartup(
		MAKEWORD(2, 0),
		&wsaData
	);

	//Socket init
	sock = socket(
		AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP
	);

	//TCP init
	srand(static_cast<int>(time(nullptr)));
	PORT = 50000 + rand() % 10000;

	//setup udp module
	udp.up(PORT + 1);

	CString msg; //Show the rand Port
	msg.Format("TCP port > %d", PORT);
	Show_log(_MSG, msg);

	sock_addr.sin_port = htons(PORT);
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	//Bind
	err = bind(
		sock,
		(sockaddr*)&sock_addr,
		sizeof(sock_addr)
	);

	err = WSAGetLastError();

	Show_log(_MSG, "Create the socket successfully");

	err = listen(
		sock,
		20
	);

	Show_log(_MSG, "Listening by using backlog 20");

	//init else
	srand(time(nullptr));
}

void NET::Cleanup(int signum) {
	WSACleanup();
	exit(signum);
}

void NET::service() {
	//set some common val
	int size_of_struct;
	CString str;
	recv_async_struct *new_async;
	device_struct* newDevice;

	new_async = new recv_async_struct;
	newDevice = new device_struct;
	new_async->device = newDevice;

	//Set the size of the sockaddr size
	size_of_struct = sizeof(newDevice->sock_addr);
	
	while (is_service_on) {
		//add a new device
		device_list.push_back(newDevice);

		//set a unique ID
		newDevice->ID =
			(rand() % 10000) * 100000000 +
			(rand() % 10000) * 10000 +
			(rand() % 10000) * 1;

		//accept the request
		newDevice->sock = accept(
			sock,
			(sockaddr*)&newDevice->sock_addr,
			&size_of_struct
		);

		/*str.Format("Accept return value > %d", err);
		Show_log(_DEBU, str);*/

		//Add the async to the list
		new_async->futu = std::async(
			std::launch::async,
			&::NET::auto_print_recv,
			this,
			newDevice
		);

		//Add into the list
		auto_receive_list.push_back(new_async);

		str.Format("New device[ ID > %ul ] added", newDevice->ID);
		Show_log(_MSG, str);

		new_async = new recv_async_struct;
		newDevice = new device_struct;
		new_async->device = newDevice;
	}
}

NET& NET::send(const char* message, unsigned long ID) {
	int err; 
	CString str;

	//Get the device
	device_struct *this_device = nullptr;
	for ( auto temp : device_list) {
		if (temp->ID == ID) {
			this_device = temp;
			break;
		}
	}

	if (this_device == nullptr)
		return *this;

	//Send a message to the address
	err = ::send(
		this_device->sock,
		message,
		strlen(message),
		0
	);
	 
	if (err == SOCKET_ERROR) {
		str.Format("send error,WSA error code is %d", WSAGetLastError());
		Show_log(_ERROR, str);
		return *this;
	}

	Show_log(_MSG, "send success");
	return *this;
}

void NET::auto_print_recv(device_struct* this_device) {
	//Set data mem
	char data[4096];
	int data_size = 0;
	CString str;

	//Get the recv data and print it
	while (data_size = recv(
		this_device->sock,
		data,
		sizeof(data),
		0
	)) {
		if (data_size == SOCKET_ERROR) {
			str.Format("Socket error , error code is > %d", WSAGetLastError());
			Show_log(_ERROR, str);
			goto dis;
		}
		data[data_size] = '\0';
		Show_log(_RECV, data);
		this_device->data.data_CS.push_back(data);
		str.Format("data_CS size > %d", this_device->data.data_CS.size());
		Show_log(_DEBU, str);
	}
dis:
	//shut it down
	disconnect(this_device->ID);
}

unsigned long NET::connect(const char* addr, UINT port) {
	CString str;
	
	//setup a new connection
	sockaddr_in connect_sockaddr;
	connect_sockaddr.sin_addr.S_un.S_addr = inet_addr(addr);
	connect_sockaddr.sin_port = htons(port);
	connect_sockaddr.sin_family = AF_INET;

	//add to the devices' list
	connect_device* device = new connect_device;

	device->sock = socket(
		AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP
	);

	device->sock_addr = connect_sockaddr;

	device->ID =
		(rand() % 10000) * 100000000 +
		(rand() % 10000) * 10000 +
		(rand() % 10000) * 1;

	//connect the target device
	int err = ::connect(
		device->sock,
		(sockaddr*)&device->sock_addr,
		sizeof(device->sock_addr)
	);

	if (err == SOCKET_ERROR) {
		str.Format("socket error when connecting a device , WSA error code is %d", WSAGetLastError());
		Show_log(_ERROR, str);
		return 0;
	}
	
	str.Format("New device connect [ ID > %ul ] ", device->ID);

	device_list.push_back(device);

	Show_log(_MSG, str);

	//add it in to auto recv list
	recv_async_struct* new_async = new recv_async_struct;
	new_async->device = device;
	new_async->futu = std::async(
		std::launch::async,
		&::NET::auto_print_recv,
		this,
		device_list[find_device(device->ID, device_list)]
	);
	auto_receive_list.push_back(new_async);

	return device->ID;
}

void NET::disconnect(unsigned long ID) {
	CString str;
	int device_no;
	Show_log(_DEBU, "disconnecting a device");
	this->mtx.lock();
	if (( device_no = find_device(ID, device_list) ) != -1) {
		device_struct* device = device_list[device_no];
		shutdown(
			device->sock,
			SD_BOTH
		);

		str.Format("a device disconnect [ device ID > %ul ]", device->ID);
		Show_log(_MSG, str);

		int auto_recv_list_no = find_device(device->ID, auto_receive_list);
		recv_async_struct* del_async = auto_receive_list[auto_recv_list_no];

		device->data.data_bin.clear();
		device->data.data_CS.clear();

		delete del_async->device;
		del_async->device = nullptr;
		//delete auto_receive_list[auto_recv_list_no];
		//auto_receive_list[auto_recv_list_no] = nullptr;
		auto_receive_list.erase(
			auto_receive_list.begin() + auto_recv_list_no
		);

		device_list.erase(device_list.begin() + device_no);
		
		this->mtx.unlock();
		device = nullptr;
		delete device;
		delete del_async;
		return;
	}
	this->mtx.unlock();
}

int NET::find_device(unsigned long ID, std::vector<device_struct*> list) {
	int device_num = 0;
	for (auto dev : list) {
		if (dev->ID == ID) {
			return device_num;
		}
		device_num++;
	}
	return -1;
}

int NET::find_device(unsigned long ID, std::vector<recv_async_struct*> list) {
	int device_num = 0;
	for (auto dev : list) {
		if (dev->device->ID == ID) {
			return device_num;
		}
		device_num++;
	}
	return -1;
}