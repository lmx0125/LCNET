#pragma once
#ifndef _LOG_H_
#define _LOG_H_

#include <iostream>
#include <fstream>
#include <mutex>
#include <string>

#define _DEBU "DEBU"
#define _MSG "MSG"
#define _RECV "RECV"
#define _ERROR "ERROR"

class LOG {
public:
	static void Show_log(std::string log_type,std::string log_msg);
private:
};

#endif