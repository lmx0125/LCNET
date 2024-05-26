#include "net.h"

NET::NET() {

	CString str;

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

	CString msg; //Show the rand Port
	msg.Format("Open port on %u", PORT);
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
	recv_async_struct new_async;
	CString str;

	while (is_service_on) {
		//add a new device
		connect_devices.push_back(newDevice);

		//set a unique ID
		connect_devices[connect_devices.size() - 1].ID =
			(rand() % 10000) * 1000000 +
			(rand() % 10000) * 1000 +
			(rand() % 10000) * 1;

		//Set the size of the sockaddr size
		size_of_struct = sizeof(connect_devices[connect_devices.size() - 1].sock_addr);

		//accept the request
		connect_devices[connect_devices.size() - 1].sock = accept(
			sock,
			(sockaddr*)&connect_devices[connect_devices.size() - 1].sock_addr,
			&size_of_struct
		);

		str.Format("Accept return value > %d", err);
		Show_log(_DEBU, str);

		//Add the async to the list
		new_async.futu = std::async(
			std::launch::async,
			&::NET::auto_print_recv,
			this,
			connect_devices[connect_devices.size() - 1]
		);
		new_async.ID = connect_devices[connect_devices.size() - 1].ID;

		//Add into the list
		str.Format("New device[ ID > %ul ] added", connect_devices[connect_devices.size() - 1].ID);
		auto_receive_list.push_back(&new_async);

		Show_log(_MSG, str);
	}
}

void NET::send(CString message, unsigned long ID) {
	//Get the device
	device this_device;
	for ( auto temp : connect_devices ) {
		if (temp.ID == ID) {
			this_device = temp;
			break;
		}
	}

	//Send a message to the address
	::send(
		this_device.sock,
		message,
		sizeof(message),
		0
	);
}

void NET::auto_print_recv(device this_device) {
	//Set data mem
	char data[4096 + 1];
	int data_size = 0;
	CString str;

	//Get the recv data and print it
	while (data_size = recv(
		this_device.sock,
		data,
		sizeof(data),
		0
	)) {
		if (data_size == SOCKET_ERROR) {
			str.Format("Socket error , error code is > %d", WSAGetLastError());
			Show_log(_ERROR, str);
			break;
		}
		if (data[data_size - 1] == '\n')
			data[data_size - 1] = '\0';
		else
			data[data_size] = '\0';
		Show_log(_RECV, data);
	}

	//Find the async in the list
	int task_line_num = 0;
	mtx.lock();

	//Del it
	for ( auto dev : auto_receive_list )
		if ( dev->ID != this_device.ID) {
			task_line_num++;
		}

	auto_receive_list.erase(
		auto_receive_list.begin() + task_line_num
	);

	task_line_num = 0;
	for (auto dev : connect_devices)
		if (dev.ID != this_device.ID) {
			task_line_num++;
		}
		else
			break;
	connect_devices.erase(
		connect_devices.begin() + task_line_num
	);
	mtx.unlock();

	str.Format("Device[ ID > %ul ] disconnect" , this_device.ID);
	Show_log(_MSG, str);
}

void connect(char* addr, UINT port) {
	
}