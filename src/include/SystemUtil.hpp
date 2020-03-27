#ifndef _SystemUtil_hpp_
#define _SystemUtil_hpp_

#include <sys/time.h>
#include <boost/format.hpp>
#include "IllegalArgumentException.hpp"

class SystemUtil {
public:
	static uint64_t currentTimeMillis() {
		struct timeval now;
		gettimeofday(&now, NULL);
		return static_cast<uint64_t>(now.tv_sec * 1000 + now.tv_usec / 1000);
	}

	static int checkPositive(int i, const string &name) {
        if (i <= 0) {
            throw  IllegalArgumentException((boost::format("%s:%d (expected: > 0)") % name.c_str() % i).str());
        }
        return i;
    } 

	static int checkPositiveOrZero(int i, const string& name) {
		if (i < 0) {
            throw  IllegalArgumentException((boost::format("%s:%d (expected: >= 0)") % name.c_str() % i).str());
        }
        return i;
	}
};
#endif
