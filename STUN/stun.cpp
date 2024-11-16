#include "stun.h"

STUN::STUN(NET* net) {
	this->net = net;
}

STUN::~STUN() {
	delete[] recv_data;
}

void STUN::build_stun_package_header(uint16_t msg_type, uint16_t att_type) {
	memset(header, 0, sizeof(header));
	header[0] = (msg_type >> 8) & 0xFF;
	header[1] = msg_type & 0xFF;
	header[2] = 0; // Length (to be updated later) 
	header[3] = 0;
	memcpy(header + 4, &MAGIC_COOKIE, sizeof(MAGIC_COOKIE));
	gen_transaction_id();
	for (size_t i = 0; i < 12; i++)
		header[i + 8] = transaction_id[i];
}

void STUN::gen_transaction_id() {
	srand((unsigned int)time(nullptr));
	for (size_t i = 0; i < 12; i++)
		transaction_id[i] = rand() % 256;
}

void STUN::send_stun_bind_request(const char* addr, UINT port) {
	// get old callback details
	old_parameter = net->udp.pass_parameter;
	old_callback_func = net->udp.callback_func;
	net->udp.pass_parameter.clear(); 
	
	if (recv_data != nullptr)
		delete[] recv_data;
	recv_data = new uint8_t[256];
	net->udp.set_callback_func_pass_parameter(recv_data);
	net->udp.set_callback_func_pass_parameter(this);
	// set callback
	this->net->udp.set_udp_package_recv_callback(stun_recv_callback);
	STUN_SERVER_ID = this->net->udp.register_new_device(addr, port);
	char msg[ARRAYSIZE(header)];
	for (size_t i = 0; i < ARRAYSIZE(header); i++)
		msg[i] = (char)header[i];
	this->net->udp.send(msg, STUN_SERVER_ID,ARRAYSIZE(header));
}

void STUN::stun_recv_callback(char* data, long buffer_size, device_struct*, int status, UDP* udp, std::vector<void*> else_parameter) {
	STUN* stun = (STUN*)(else_parameter[1]);
	memcpy(else_parameter[0], data, buffer_size);
	uint8_t* test_array = (uint8_t*)data;
	stun->stun_callback_clear();
}

void STUN::stun_callback_clear() {
	this->net->udp.callback_func = old_callback_func;
	net->udp.pass_parameter.clear();
	this->net->udp.pass_parameter = old_parameter;
}

uint16_t STUN::get_end_port() {
	return recv_data[26] * 256 + recv_data[27];
}

std::string STUN::get_end_IP() {
	std::string ret_str = std::to_string(recv_data[28]) + "."
		+ std::to_string(recv_data[29]) + "."
		+ std::to_string(recv_data[30]) + "."
		+ std::to_string(recv_data[31]);
	return ret_str;
}