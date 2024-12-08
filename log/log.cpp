#include "log.h"

//invald
//void LOG::Show_log(std::string log_type, std::string log_msg) {
//	std::string str;
//	str = "[" + log_type + "] [" + log_msg + "]\n";
//	std::cout << str << std::flush;
//}

std::mutex mtx;