#ifndef _ConnectionListener_HPP_
#define _ConnectionListener_HPP_

#include <boost/shared_ptr.hpp>
using boost::shared_ptr;

namespace comm {
namespace net {

class Connection;
class ConnectionListener {
public:
	virtual ~ConnectionListener() {}
	virtual void connectionActive(const shared_ptr< Connection >& conn) = 0;
	virtual void connectionInActive(const shared_ptr< Connection >& conn) = 0;
};

}//namespace net
}//namespace comm


#endif
