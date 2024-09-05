#include "net.h"

NET::~NET() {
	WSACleanup();
}

NET::NET(int port) {
	//Set exit signal
	signal(SIGABRT,&NET::Cleanup);
	signal(SIGFPE, &NET::Cleanup);
	signal(SIGILL, &NET::Cleanup);
	signal(SIGSEGV,&NET::Cleanup);
	signal(SIGINT, &NET::Cleanup);
	signal(SIGTERM,&NET::Cleanup);

#ifdef _WIN32
	//WSA init
	WSAStartup(
		MAKEWORD(2, 0),
		&wsaData
	);
#endif //_WIN32

	//Socket init
	sock = socket(
		AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP
	);

	//TCP init
	srand(static_cast<int>(time(nullptr)));
	if (port == 0)
		PORT = 50000 + rand() % 10000;
	else
		this->PORT = port;

	//setup udp module
	udp.up(PORT + 1);

	std::string msg; //Show the rand Port
	msg = "TCP port > " + std::to_string(PORT);
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

	if (err == SOCKET_ERROR) {
		Show_log(_ERROR, "bind error");
		exit(-1);
	}

	Show_log(_MSG, "Create the socket successfully");

	err = listen(
		sock,
		20
	);

	Show_log(_MSG, "Listening by using backlog 20");

	//init else
	srand((UINT)time(nullptr));
}

void NET::Cleanup(int signum) {
#ifdef _WIN32
	WSACleanup();
#endif // _WIN32
	exit(signum);
}

void NET::service() {
	//set some common val
	std::string str;
	recv_async_struct *new_async;
	device_struct* newDevice;

	new_async = new recv_async_struct;
	newDevice = new device_struct;
	new_async->device = newDevice;

	//Set the size of the sockaddr size
#ifdef _WIN32
	int size_of_struct = sizeof(newDevice->sock_addr);
#else
	socklen_t size_of_struct = sizeof(newDevice->sock_addr);
#endif
	
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

		str = "New device[ ID > " + std::to_string(newDevice->ID) + " ] added";
		Show_log(_MSG, str);

		new_async = new recv_async_struct;
		newDevice = new device_struct;
		new_async->device = newDevice;
	}
}

NET& NET::send(const char* message, unsigned long ID) {
	int err; 
	std::string str;

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
		(int)strlen(message),
		0
	);
	 
	if (err == SOCKET_ERROR) {
#ifdef _WIN32
		str = "send error,WSA error code is " + std::to_string(WSAGetLastError());
		Show_log(_ERROR, str);
#else
		Show_log(_ERROR, "send message error");
#endif
		return *this;
	}

	Show_log(_MSG, "send success");
	return *this;
}

void NET::auto_print_recv(device_struct* this_device) {
	//Set data mem
	char data[4096];
	int data_size = 0;
	std::string str;

	//Get the recv data and print it
	while (data_size = recv(
		this_device->sock,
		data,
		sizeof(data),
		0
	)) {
		if (data_size == SOCKET_ERROR) {
#ifdef _WIN32
			str = "Socket error , error code is > " + std::to_string(WSAGetLastError());
			Show_log(_ERROR, str);
#else
			Show_log(_ERROR, "Socket error");
#endif
			goto dis;
		}
		data[data_size] = '\0';
		//Show_log(_RECV, data);
		this_device->data.data_CS.push_back(data);
	}
dis:
	//shut it down
	disconnect(this_device->ID);
}

unsigned long NET::connect(const char* addr, UINT port) {
	std::string str;
	
	//setup a new connection
	sockaddr_in connect_sockaddr;
	connect_sockaddr.sin_addr.s_addr = inet_addr(addr);
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
#ifdef _WIN32
		str = "socket error when connecting a device , WSA error code is " +
			std::to_string(WSAGetLastError());
		Show_log(_ERROR, str);
#else
		Show_log(_ERROR, "socket error when connecting a device");
#endif
		return 0;
	}
	

	device_list.push_back(device);

	str = "New device connect [ ID > " + std::to_string(device->ID) + " ]";
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
	std::string str;
	int device_no;
	Show_log(_DEBU, "disconnecting a device");
	this->mtx.lock();
	if (( device_no = find_device(ID, device_list) ) != -1) {
		device_struct* device = device_list[device_no];
#ifdef _WIN32
		shutdown(
			device->sock,
			SD_BOTH
		);
#else
		shutdown(
			device->sock,
			SHUT_RDWR
		);
#endif

		str = "a device disconnect [ device ID > " + std::to_string(device->ID) + " ]";
		Show_log(_MSG, str);

		int auto_recv_list_no = find_device(device->ID, auto_receive_list);
		recv_async_struct* del_async = auto_receive_list[auto_recv_list_no];

		device->data.data_bin.erase(device->data.data_bin.begin(),device->data.data_bin.end());
		device->data.data_CS.erase(device->data.data_CS.begin(), device->data.data_CS.end());

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

char* NET::domain_to_IP(char* domain) {
	hostent* host_info;
	host_info = gethostbyname(domain); 
	in_addr* address = (in_addr*)host_info->h_addr_list[0];
	char* ip_address = inet_ntoa(*address);
	return ip_address;
}                                        

char* NET::domain_to_IP(std::string domain) {
	char char_domain[16];
	sprintf(char_domain, "%s", domain.c_str());
	hostent* host_info;
	host_info = gethostbyname(char_domain);
	in_addr* address = (in_addr*)host_info->h_addr_list[0];
	char* ip_address = inet_ntoa(*address);
	return ip_address;
}