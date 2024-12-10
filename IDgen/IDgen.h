#ifndef _IDGEN_H_
#define _IDGEN_H_

#ifdef _WIN32
#include <combaseapi.h>
#else
#include <uuid/uuid.h>
#endif

#include <vector>
#include <time.h>
#include <mutex>
#include <algorithm>
#include "../log/log.h"

class IDgen : LOG {
public:
	IDgen() = default;
	unsigned long generateID();
	void deleteID(unsigned long ID);

	static std::string getUUID();

private:
	std::vector<unsigned long> ID_list;
	std::mutex mtx;
};

#endif