#pragma once
#ifndef _LOG_H_
#define _LOG_H_

#include <iostream>
#include <fstream>
#include <mutex>
#include <string>
#include <sstream>

#define _DEBU "DEBU"
#define _MSG "MSG"
#define _RECV "RECV"
#define _ERROR "ERROR"

extern std::mutex mtx;

class LOG {
public:

    //deprecated func
	//static void Show_log(std::string log_type,std::string log_msg);

    template<typename... T>
	static void Show_log(T... args) {
        std::lock_guard<std::mutex> guard(mtx);
		std::cout << pack_log(args...) << std::endl;
        std::cout << std::flush;
	}

private:
    static std::string pack_log() {
        return "";
    }

    template<typename T>
    static std::string to_string(const T& val) {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }

    static std::string to_string(const char* val) {
        return std::string(val);
    }

    template<typename T, typename... Args>
    static std::string pack_log(T first, Args... args) {
        std::stringstream ss;

        ss << "[" << to_string(first) << "]";

        std::string rest = pack_log(args...);
        if (!rest.empty()) {
            ss << " " << rest;  
        }

        return ss.str();
    }
};

#endif