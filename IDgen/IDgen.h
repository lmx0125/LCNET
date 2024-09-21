#ifndef _IDGEN_H_
#define _IDGEN_H_

#include <vector>
#include <time.h>
#include <mutex>
#include "../log/log.h"

class IDgen : LOG {
public:
	IDgen() = default;
	unsigned long generateID();
	void deleteID(unsigned long ID);
private:
	std::vector<unsigned long> ID_list;
	std::mutex mtx;
};

#endif