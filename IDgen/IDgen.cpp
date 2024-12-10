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

std::string IDgen::getUUID() {
	char uid_c[38];
#ifdef _WIN32
	GUID uid;
	CoCreateGuid(&uid);
	sprintf(uid_c, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
		uid.Data1, uid.Data2, uid.Data3, uid.Data4[0], uid.Data4[1], uid.Data4[2], uid.Data4[3], uid.Data4[4], uid.Data4[5], uid.Data4[6], uid.Data4[7]
	);
#else
	uuid_t uid;
	uuid_generate(uid);
	uuid_unparse(uid, uid_c);
#endif
	return std::string(uid_c);
}