#ifndef ILINKER_H_
#define ILINKER_H_

#include <cstdint>

class ILinkable {
public:
	virtual int64_t GetId() = 0;
	virtual void SetId(int64_t id) = 0;
};

#endif // ILINKER_H_