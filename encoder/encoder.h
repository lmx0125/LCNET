#ifndef _ENCODER_H_
#define _ENCODER_H_

#include <string>
#include <iostream>

class encoder {
public:
	encoder() = default;

	//base encode func
	template<typename T>
	std::string encode(T encode_val) {
		size_t strsize = std::to_string(encode_val).size();
		return std::to_string(strsize) + ":" + std::to_string(encode_val) + ":";
	};

	std::string encode(std::string encode_val);

	//base decode func
	//maybe it will return string while you need int or etc.
	std::string decode(std::string decode_str);

private:
};

#endif // !_ENCODER_H_
