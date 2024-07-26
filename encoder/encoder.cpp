#include "encoder.h"

std::string encoder::encode(std::string encode_val) {
	size_t strsize = encode_val.size();
	return std::to_string(strsize) + ":" + encode_val + ":";
}

std::string encoder::decode(std::string decode_str) {
	size_t str_start_pos = decode_str.find_first_of(":");
	std::cout << decode_str.substr(str_start_pos);
	return decode_str;
}