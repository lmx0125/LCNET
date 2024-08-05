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
	std::string packed_string = P2P::encode_p2p_package(p2p_package);
	char* char_send_message = new char[packed_string.length() + 1];
	sprintf(char_send_message, "%s", packed_string.c_str());
	net->udp.send(char_send_message, verify_server_ID);
	delete[] char_send_message;
}