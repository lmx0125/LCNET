#include "p2p.h"

void P2P::add_verify_server(const char* addr, int port) {
	verify_server = const_cast<char*>(addr);
	verify_server_port = port;
	verify_server_ID = net->udp.register_new_device(addr, port);
}

void P2P::p2p_send_join_package() {
	p2p_data_struct p2p_package;
	p2p_package.package_type = _P2P_NETWORK_CONNECTION_PACKAGE_;
	p2p_package.flags = _P2P_CONNECT_FLAG_;
	std::string packed_string = P2P::encode_p2p_package(&p2p_package);
	char* char_send_message = new char[packed_string.length() + 1];
	sprintf(char_send_message, "%s", packed_string.c_str());
	net->udp.send(char_send_message, verify_server_ID);
	delete[] char_send_message;
}

void P2P::on_recv_connect_request(char* data, device_struct* device, int status, UDP* udp, P2P* p2p) {
	p2p_data_struct p2p_package;
	p2p_package.package_type = _P2P_HOLE_PUNCHING_PACKAGE_;
	p2p_package.flags = _P2P_CONNECT_FLAG_;
	sprintf(p2p_package.data, "_P2P_HOLE_PUNCHING_PACKAGE_");
	udp->send(encode_p2p_package(&p2p_package), device->ID);
}

void P2P::send_hole_punching_request(const char* addr, int port) {
	if (addr == nullptr || port == 0) {
		Show_log(_MSG, "this addr or port is invalid");
		return;
	}

	p2p_data_struct p2p_package(128);
	p2p_package.package_type = _P2P_HOLE_PUNCHING_PACKAGE_;
	p2p_package.flags = _P2P_HOLE_PUNCHING_FLAG_;
	sockaddr_in sock_addr;
	sock_addr.sin_addr.s_addr = inet_addr(addr);
	sock_addr.sin_port = htons(port);
	sprintf(p2p_package.data, P2P::encode_sockaddr_in(sock_addr).c_str());
	p2p_send(&p2p_package, verify_server, verify_server_port);
}

void P2P::on_recv_hole_punching_request(char* data, device_struct* device, int status, UDP* udp, P2P* p2p) {
	std::string data_str = data;
	p2p_data_struct* recv_package = decode_p2p_package(data_str);
	p2p_data_struct send_package(64);
	send_package.package_type = _P2P_HOLE_PUNCHING_PACKAGE_;
	send_package.flags = _P2P_UNKNOWN_DEVICE_FLAG_;
	std::string str_data = recv_package->data;
	sockaddr_in target_addr = P2P::decode_sockaddr_in(str_data);
	std::string source_addr = P2P::encode_sockaddr_in(device->sock_addr);

	if (recv_package->flags == _P2P_DEVICE_AVAILABLE_FLAG_) {
		send_package.flags = _P2P_DEVICE_AVAILABLE_FLAG_;
		send_package.status = _DROP_;
		p2p->p2p_send(&send_package, device->ID);
		goto clean_up;
	}

	if (udp->get_device_no_from_addr(target_addr) == -1) {
		p2p->p2p_send(&send_package, device->ID);
		Show_log(_MSG, "no excepted device found", inet_ntoa(device->sock_addr.sin_addr), htons(device->sock_addr.sin_port));
		goto clean_up;
	}

	send_package.flags = _P2P_DEVICE_AVAILABLE_FLAG_;

	sprintf(send_package.data, source_addr.c_str());
	p2p->p2p_send(&send_package, device->ID);
	sprintf(send_package.data, P2P::encode_sockaddr_in(target_addr).c_str());
	p2p->p2p_send(
		&send_package,
		inet_ntoa(device->sock_addr.sin_addr),
		ntohs(device->sock_addr.sin_port)
	);
	
clean_up:
	delete recv_package;
}

void P2P::on_recv_hole_punching_response(char* data, device_struct* device, int status, UDP* udp, P2P* p2p) {
	p2p_data_struct p2p_package(8);
	p2p_package.package_type = _P2P_HOLE_PUNCHING_PACKAGE_;
	p2p_package.flags = _P2P_DEVICE_AVAILABLE_FLAG_;
	p2p_package.status = _DROP_;
	p2p->p2p_send(&p2p_package);
}

void P2P::on_recv_registry_request(char* data, device_struct* device, int status, UDP* udp, P2P* p2p) {
	ul ID = udp->register_new_device(
		inet_ntoa(device->sock_addr.sin_addr),
		ntohs(device->sock_addr.sin_port)
	);
	p2p_data_struct p2p_package(32);
	p2p_package.package_type = _P2P_REGISTRY_PACKAGE_;
	p2p_package.flags = _P2P_FROM_SERVER_FLAG_;
	sprintf(p2p_package.data, "%l", ID);
	p2p->p2p_send(
		&p2p_package,
		ID
	);
}

void P2P::send_registry_request() {
	p2p_data_struct p2p_package(32);
	p2p_package.package_type = _P2P_REGISTRY_PACKAGE_;
	p2p_package.flags = _P2P_FROM_CLIENT_FLAG_;
	sprintf(p2p_package.data, "_P2P_FROM_CLIENT_FLAG_");
	p2p_send(&p2p_package, verify_server_ID);
}

void P2P::on_recv_registry_response(char* data, device_struct* device, int status, UDP* udp, P2P* p2p) {
	
}