#ifndef _ConnectionListenerPipeline_HPP_
#define _ConnectionListenerPipeline_HPP_

#include <list>
#include <boost/thread/mutex.hpp>
#include "ConnectionListener.hpp"

namespace comm {
namespace net {
	
class ConnectionListenerPipeline : public ConnectionListener  {
public:
	void addConnectionListener( ConnectionListener* listener );
	void removeConnectionListener( ConnectionListener* listener ) ;
	virtual void connectionActive(const shared_ptr< Connection >& conn) ;
	virtual void connectionInActive(const shared_ptr< Connection >& conn) ;
	
private:
	boost::mutex mutex_;
	std::list<ConnectionListener*> listeners_;
};

}//namespace net
}//namespace comm


#endif
