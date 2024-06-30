#include "p2p.h"

P2P::P2P(NET* net) {
	Show_log(_MSG, "p2p service is initializing");
	this->net = net;

	this->core_service = std::thread(&::P2P::p2p_core_service,this);
	this->core_service.detach();

	this->net->udp.set_udp_package_recv_callback(this->p2p_package_recv_callback);
}

void P2P::p2p_core_service() {
	Show_log(_MSG, "p2p core service is running");
	
}

void P2P::register_p2p_network(const char* addr, int port) {
	if (addr != "127.0.0.1") {
		if (port != 0) {
			join_p2p_network(addr, port);
			Show_log(_MSG, "using other addr");
		}
		return;
	}
	Show_log(_MSG, "using localhost");
	p2p_network_ID =
		(rand() % 10000) * 100000000 +
		(rand() % 10000) * 10000 +
		(rand() % 10000) * 1;
}

void P2P::join_p2p_network(const char* addr, int port) {
	Show_log(_MSG, "joining a p2p network");
	ul ID = this->net->udp.register_new_device(addr, port);
	p2p_data_struct p2p_package;
	this->net->udp.send(
		p2p_data_2_char(p2p_package).get(),
		ID
	);
}

void P2P::quit_p2p_network(ul p2p_network_ID) {
	Show_log(_MSG, "quit a p2p network");
}

void P2P::p2p_package_recv_callback(char* data, device_struct* device, int status) {
	p2p_data_struct p2p_data = char_2_p2p_data(data);
	printf(p2p_data.data);
	switch (p2p_data.package_type) {
	case(_P2P_NETWORK_DEVICE_LIST_PACKAGE_):
		Show_log(_MSG, "this is a p2p network device list package");

		break;
	case(_P2P_NETWORK_DATA_PACKAGE_):
		Show_log(_MSG, "this is a p2p network data package");

		break;
	case(_P2P_HEART_BEAT_PACKAGE_):
		Show_log(_MSG, "this is a p2p heart beat package");

		break;
	case(_P2P_NETWORK_BOARDCAST_PACKAGE_):
		Show_log(_MSG, "this is a p2p boardcast package");

		break;
	case(_P2P_NETWORK_STATUS_PACKAGE_):
		Show_log(_MSG, "this is a p2p network status package");

		break;
	default:
		Show_log(_ERROR, "this package is not a p2p package [LCNET] ");
	}
}

void P2P::p2p_send(p2p_data_struct p2p_data) {
	std::shared_ptr<char[]> data = p2p_data_2_char(p2p_data);
	this->net->udp.send(data.get(), 12445345);
}


void P2P::add_p2p_device(device_struct device, ul ID) {
	if (ID == 0)
		ID =
		(rand() % 10000) * 100000000 +
		(rand() % 10000) * 10000 +
		(rand() % 10000) * 1;

	device.ID = ID;

	p2p_device_list.push_back(ID);
	this->net->udp.register_new_device(
		inet_ntoa(device.sock_addr.sin_addr),
		ntohs(device.sock_addr.sin_port)
	);
}

device_struct* P2P::get_p2p_device_by_id(ul ID) {
	device_struct* return_device = nullptr;
	this->net->udp.mtx.lock();
	for ( auto device_ID : p2p_device_list )
		if (device_ID == ID)
			return_device = this->net->udp.device_list[
				this->net->udp.get_device_no_from_id(ID)
			];
	this->net->udp.mtx.unlock();
	return return_device;
}

p2p_data_struct P2P::char_2_p2p_data(char* data) {
	p2p_data_struct return_data;
	memcpy(
		&return_data,
		data,
		strlen(data)
	);
	delete[] data;
	return return_data;
}

std::shared_ptr<char[]> P2P::p2p_data_2_char(p2p_data_struct data) {
	std::shared_ptr<char[]> return_data(new char[4200]);
	memcpy(
		&return_data,
		&data,
		4200 + 1
	);
	return return_data;
}