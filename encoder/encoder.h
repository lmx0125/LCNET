#ifndef _ENCODER_H_
#define _ENCODER_H_

#include <string>
#include <iostream>
#include <vector>



class encoder {
public:
	encoder() = default;

	//base encode func
	template<typename T>
	static std::string encode(T encode_val) {
		size_t strsize = std::to_string(encode_val).size();
		return std::to_string(strsize) + ";" + std::to_string(encode_val) + ";";
	};

	static std::string encode(const std::string& encode_val);
	static std::string encode(const char* encode_val) {
		size_t strsize = strlen(encode_val);
		return std::to_string(strsize) + ";" + encode_val + ";";
	};
	static std::string encode(char* encode_val) {
		size_t strsize = strlen(encode_val);
		return std::to_string(strsize) + ";" + encode_val + ";";
	};

	template<typename T>
	static std::string encode(std::vector<T> encode_val) {
		std::string encode_str = "V" + std::to_string(encode_val.size()) + ";";
		for (size_t i = 0; i < encode_val.size(); i++)
			encode_str += encoder::encode(encode_val[i]);
		return encode_str;
	}

	//base decode func
	//maybe it will return string while you need int or etc.
	static std::string decode(std::string& decode_str);
	static std::vector<std::string> decode_vec(std::string& decode_str);

	//template<typename T>
	//static T decode(std::string& decode_str) {
	//	if (is_code_with_vec(decode_str))
	//		return decode_vec(decode_str);
	//	return decode(decode_str);
	//}


	static bool is_code_with_vec(std::string& decode_str);
private:
};

#endif // !_ENCODER_H_
