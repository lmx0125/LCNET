#pragma once
#ifndef _LOG_
#define _LOG_

#include <iostream>
#include <fstream>
#include <mutex>
#include <atlstr.h>

#define _DEBU "DEBU"
#define _MSG "MSG"
#define _RECV "RECV"
#define _ERROR "ERROR"

class LOG {
public:
	void Show_log(CString log_type,CString log_msg);
private:
};

#endif