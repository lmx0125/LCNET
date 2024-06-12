#include "log.h"

void LOG::Show_log(CString log_type, CString log_msg) {
	CString str;
	str.Format("[%s] [%s]\n", log_type, log_msg);
	std::cout << str << std::flush;
}