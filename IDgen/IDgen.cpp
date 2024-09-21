#include "IDgen.h"

unsigned long IDgen::generateID() {
	unsigned long ID;
	if (ID_list.size() == 0)
		ID = 1;
	else
		ID = ID_list[ID_list.size() - 1] + 1;
	ID_list.push_back(ID);
	return ID;
}

void IDgen::deleteID(unsigned long ID) {
	std::lock_guard<std::mutex> lock(mtx);
	auto val = std::remove(ID_list.begin(), ID_list.end(), ID);
	ID_list.erase(val, ID_list.end());
}