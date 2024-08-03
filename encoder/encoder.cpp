#include "encoder.h"

std::string encoder::encode(const std::string& encode_val) {
	size_t strsize = encode_val.size();
	return std::to_string(strsize) + ";" + encode_val + ";";
}

//std::string encode(const char* encode_val) 

std::string encoder::decode(std::string& decode_str) {
	size_t str_start_pos = decode_str.find_first_of(";");
	size_t strsize = std::stoll(decode_str.substr(0, str_start_pos));
	//std::cout << decode_str.substr(str_start_pos);
	decode_str = decode_str.substr(str_start_pos + 1);
	std::string ret_str = decode_str.substr(0, strsize);
	decode_str = decode_str.substr(strsize + 1);
	return ret_str;
}

bool encoder::is_code_with_vec(std::string& decode_str) {
	if (decode_str[0] == 'V')
		return true;
	return false;
}

std::vector<std::string> encoder::decode_vec(std::string& decode_str) {
	size_t str_start_pos = decode_str.find_first_of(";");
	size_t strsize = std::stoll(decode_str.substr(1, str_start_pos));
	decode_str = decode_str.substr(str_start_pos + 1);
	std::vector<std::string> ret_string_vec;
	for (size_t i = 0; i < strsize; i++)
		ret_string_vec.push_back(decode(decode_str));
	return ret_string_vec;
}