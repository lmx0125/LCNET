#include "log.h"

void LOG::Show_log(CString log_type, CString log_msg) {
	mtx.lock();
	std::cout << "[" << log_type.GetString() << "] "
		<< "[" << log_msg.GetString() << "]" << std::endl << std::flush;
	mtx.unlock();
}