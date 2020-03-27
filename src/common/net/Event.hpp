#ifndef _Event_hpp_
#define _Event_hpp_

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
using std::string;
using boost::shared_ptr;

namespace comm { 
namespace net {
	typedef boost::function<void(bool)> EventCallBack;
	class Event {
	public:
		//current max event id 3
		virtual ~Event() {}
		virtual int eventId() = 0;
		virtual const string& name() = 0;
	};
}//namespace net
}//namespace comm
#endif


/*
#ifndef 
#define 


namespace comm { 
namespace net {
	class {
	};
}//namespace net
}//namespace comm

#endif

*/
