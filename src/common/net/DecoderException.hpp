#ifndef _DecoderException_hpp_
#define _DecoderException_hpp_

#include <exception>
#include <string>

using std::exception;
using std::string;

namespace comm { 
namespace net {
	class DecoderException : public exception {
	public:
		DecoderException() {
		}
		
		explicit DecoderException(const char* _reason) : reason(_reason) {
		}

		explicit DecoderException(const string& _reason) : reason(_reason) {
		}
		
		DecoderException(const DecoderException& rhs) : reason(rhs.reason) {
		}

		DecoderException& operator=(const DecoderException& rhs) {
			if ( this != &rhs) {
				reason = rhs.reason;
			}

			return *this;
		}
		
		~DecoderException() throw() {
		}
		
		virtual const char* what() const throw() {
			return reason.c_str();
		}
		
	private:
		string reason;
	};
}//namespace net
}//namespace comm

#endif
