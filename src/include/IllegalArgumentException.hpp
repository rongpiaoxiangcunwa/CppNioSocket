#ifndef _IllegalArgumentException_hpp_
#define _IllegalArgumentException_hpp_

#include <string>
#include <exception>

using std::exception;
using std::string;

class IllegalArgumentException : public exception
{
public:
	IllegalArgumentException() {}
	explicit IllegalArgumentException(const char* reason) : reason_(reason) {
	}
	
	explicit IllegalArgumentException(const string& reason) :reason_(reason) {
	}

	virtual ~IllegalArgumentException() throw (){}
	
	IllegalArgumentException& operator=(const IllegalArgumentException& rhs) {
		if (this != &rhs) {
			reason_ = rhs.reason_;
		}
		return *this;
	}

	virtual const char* what() const throw() {
		return reason_.c_str();
	}
	
private:
	string reason_;
};

#endif