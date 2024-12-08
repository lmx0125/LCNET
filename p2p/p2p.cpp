#include "p2p.h"

P2P::P2P(NET* net) {
	Show_log(_MSG, "p2p service is initializing");
	this->net = net;

	this->core_service = std::thread(&P2P::p2p_core_service,this);
	this->core_service.detach();

	this->net->udp.set_auto_disconnect_time(INT_MAX);

	this->net->udp.set_callback_func_pass_parameter(this);
	this->net->udp.set_udp_package_recv_callback(this->p2p_package_recv_callback);
}

void P2P::p2p_core_service() {
	Show_log(_MSG, "p2p core service is running");
}

void P2P::register_p2p_network(const char* addr, int port) {
	if (addr != "127.0.0.1") {
		if (port != 0) {
			join_p2p_network_by_addr(addr, port);
			Show_log(_MSG, "using other addr");
		}
		return;
	}
	Show_log(_MSG, "using localhost");
	p2p_network_ID = IDgenerator.generateID();
}

void P2P::join_p2p_network_by_addr(const char* addr, int port) {
	Show_log(_MSG, "joining a p2p network");
	ul ID = net->udp.register_new_device(addr, port);
	p2p_data_struct p2p_package;
	p2p_package.package_type = _P2P_NETWORK_CONNECTION_PACKAGE_;
	p2p_package.flags = _P2P_CONNECT_FLAG_;
	this->p2p_send(&p2p_package, ID);
}

void P2P::quit_p2p_network() {
	Show_log(_MSG, "quit a p2p network");
}

void P2P::p2p_package_recv_callback(char* data, long buffer_size, device_struct* device, int status,UDP* udp, std::vector<void*> pass_va) {
	std::string string_data(data);
	p2p_data_struct* p2p_data = decode_p2p_package(string_data);

	if (pass_va.size() < 1) {
		Show_log(_ERROR, "can not get pass_val");
		return;
	}
	P2P* p2p = (P2P*)pass_va[0];

	switch (p2p_data->package_type) {
	//on recv p2p connect request(s)
	case(_P2P_NETWORK_CONNECTION_PACKAGE_):
		Show_log(_DEBU, "recv a p2p network connection package");
		switch (p2p_data->flags) {
		case(_P2P_CONNECT_FLAG_)://connect
			on_recv_connect_request(data, device, status, udp, p2p);
			break;
		case(_P2P_DISCONNECT_FLAG_)://disconnect
			break;
		case(_P2P_HOLE_PUNCHING_FLAG_):
			on_recv_hole_punching_request(data, device, status, udp, p2p);
			break;
		}
		break;

	case(_P2P_NETWORK_DEVICE_LIST_PACKAGE_):
		Show_log(_DEBU, "recv a p2p network device list package");

		break;
	case(_P2P_NETWORK_DATA_PACKAGE_):
		Show_log(_DEBU, "recv a p2p network data package");

		break;
	case(_P2P_HEART_BEAT_PACKAGE_):
		Show_log(_DEBU, "recv a p2p heart beat package");

		break;
	case(_P2P_NETWORK_BOARDCAST_PACKAGE_):
		Show_log(_DEBU, "recv a p2p boardcast package");

		break;
	case(_P2P_NETWORK_STATUS_PACKAGE_):
		Show_log(_DEBU, "recv a p2p network status package");

		break;
	case(_P2P_HOLE_PUNCHING_PACKAGE_):
		Show_log(_DEBU, "recv a hole pubching package [LCNET]");
		switch (p2p_data->flags) {
		case(_P2P_UNKNOWN_DEVICE_FLAG_):
			Show_log(_MSG, "can not find this device in the network");
			break;
		case(_P2P_DEVICE_AVAILABLE_FLAG_):
			if (p2p_data->status == _DROP_) {
				Show_log(_MSG, "recv hole punching package [ valid ]", "my addr", udp->PORT, inet_ntoa(device->sock_addr.sin_addr), htons(device->sock_addr.sin_port));
				break;
			}
			on_recv_hole_punching_request(data, device, status, udp, p2p);
			break;
		case(_P2P_HOLE_PUNCHING_FLAG_):
			on_recv_hole_punching_request(data, device, status, udp, p2p);
			break;
		case(_P2P_CONNECT_FLAG_):
			on_recv_hole_punching_response(data, device, status, udp, p2p);
		}
		break;
	case(_P2P_REGISTRY_PACKAGE_):
		Show_log(_DEBU, "recv a registry package [LCNET]");
		switch (p2p_data->flags) {
		case(_P2P_FROM_SERVER_FLAG_):
			break;
		case(_P2P_FROM_CLIENT_FLAG_):
			on_recv_registry_request(data, device, status, udp, p2p);
			break;
		}
		break;
	default:
		Show_log(_DEBU, "this package is not a p2p package [LCNET]");
		udp->default_recv_callback_func(data, buffer_size, device, status, udp, pass_va);
	}
	Show_log(
		_DEBU,
		"my addr",
		udp->PORT,
		"package addr",
		inet_ntoa(device->sock_addr.sin_addr),
		htons(device->sock_addr.sin_port)
	);
	delete p2p_data;
}

void P2P::p2p_send(p2p_data_struct* p2p_data, ul ID) {
	std::string packed_data = encode_p2p_package(p2p_data);
	this->net->udp.send(packed_data, ID);
}

void P2P::p2p_send(p2p_data_struct* p2p_data, const char* addr, int port) {
	p2p_send(
		p2p_data,
		this->net->udp.register_new_device(addr, port)
	);
}


void P2P::send_add_p2p_device_request(device_struct device, ul ID) {
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

//p2p_data_struct P2P::char_2_p2p_data(char* data) {
//	p2p_data_struct return_data;
//	memcpy(
//		&return_data,
//		data,
//		strlen(data)
//	);
//	delete[] data;
//	return return_data;
//}

//std::shared_ptr<char[]> P2P::p2p_data_2_char(p2p_data_struct data) {
//	std::shared_ptr<char[]> return_data(new char[4200]);
//	memcpy(
//		&return_data,
//		&data,
//		4200 + 1
//	);
//	return return_data;
//}

template<typename T, typename T2>
bool P2P::is_id_in_list(T ID, std::vector<T2> list) {
	for (auto list_id : list)
		if (ID == list_id)
			return true;
	return false;
}

std::string P2P::encode_p2p_package(p2p_data_struct* pack) {
	/*
	*	encode member
	*	std::vector<device_struct> p2p_network_device_list
	*	char* data;
	*	UINT package_type;
	*	UINT status;
	*	unsigned short flags;
	*/
	std::string packed_str;
	std::vector<std::string> packed_device_list;
	for (auto dev : pack->p2p_network_device_list)
		packed_device_list.push_back(P2P::encode_device_struct(&dev));
	packed_str += encoder::encode(packed_device_list);
	packed_str += encoder::encode(pack->data);
	packed_str += encoder::encode((long)pack->package_type);
	packed_str += encoder::encode((long)pack->status);
	packed_str += encoder::encode((int)pack->flags);
	return packed_str;
}

p2p_data_struct* P2P::decode_p2p_package(std::string& pack_str) {
	/*
	*	decode member
	*	std::vector<device_struct> p2p_network_device_list
	*	char* data;
	*	UINT package_type;
	*	UINT status;
	*	unsigned short flags;
	*/
	p2p_data_struct* decoded_pack = new p2p_data_struct;
	std::vector<std::string> decode_device_list = encoder::decode_vec(pack_str);
	for (auto dev : decode_device_list)
		decoded_pack->p2p_network_device_list.push_back(*P2P::decode_device_struct(dev));
	std::string decode_string = encoder::decode(pack_str);
	decoded_pack->set_data_size(decode_string.size() + 1);
	sprintf(decoded_pack->data, decode_string.c_str());
	decoded_pack->package_type = (UINT)std::stoul(encoder::decode(pack_str));
	decoded_pack->status = (UINT)std::stoul(encoder::decode(pack_str));
	decoded_pack->flags = (unsigned short)std::stoul(encoder::decode(pack_str));
	return decoded_pack;
}

std::string P2P::encode_device_struct(device_struct* device) {
	/*
	*	encode member
	*	sockaddr_in addr
	*	ul ID
	*/
	std::string packed_str;
	packed_str += P2P::encode_sockaddr_in(device->sock_addr);
	packed_str += encoder::encode(device->ID);
	return packed_str;
}

device_struct* P2P::decode_device_struct(std::string& device_str) {
	/*
	*	decode member
	*	sockaddr_in addr
	*	ul ID
	*/
	device_struct* device = new device_struct;
	device->sock_addr = P2P::decode_sockaddr_in(device_str);
	device->ID = std::stoul(encoder::decode(device_str));
	return device;
}

std::string P2P::encode_sockaddr_in(sockaddr_in addr) {
	/*
	*	encode menber
	*	USHORT sin_port;
	*	IN_ADDR sin_addr;
	*/
	return encoder::encode(addr.sin_port) 
		 + encoder::encode(addr.sin_addr.s_addr);
}

sockaddr_in P2P::decode_sockaddr_in(std::string& addr) {
	/*
	*	decode menber
	*	USHORT sin_port;
	*	IN_ADDR sin_addr;
	*/
	sockaddr_in ret_addr;
	ret_addr.sin_port = (unsigned short)std::stoul(encoder::decode(addr));
	ret_addr.sin_addr.s_addr = std::stoul(encoder::decode(addr));
	return ret_addr;
}