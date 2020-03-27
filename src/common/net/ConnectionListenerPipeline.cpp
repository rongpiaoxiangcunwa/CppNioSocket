#include <boost/foreach.hpp>
#ifdef USE_BOOST_1_68
#include <boost/thread/lock_guard.hpp>
#endif
#include "ConnectionListenerPipeline.hpp"

namespace comm {
namespace net {
	void ConnectionListenerPipeline::addConnectionListener( ConnectionListener* listener ) {
		if( listener ) {
			boost::lock_guard< boost::mutex > lock( mutex_ );
			listeners_.push_back( listener );
		}
	}
	
	void ConnectionListenerPipeline::removeConnectionListener( ConnectionListener* listener ) {
		if( listener ) {
			boost::lock_guard< boost::mutex > lock( mutex_ );
			listeners_.remove( listener );
		}
	}
	
	void ConnectionListenerPipeline::connectionActive(const shared_ptr< Connection >& conn) {
		boost::lock_guard< boost::mutex > lock( mutex_ );
		BOOST_FOREACH(ConnectionListener* listener, listeners_) {
			listener->connectionActive( conn );
		}
	}
	
	void ConnectionListenerPipeline::connectionInActive(const shared_ptr< Connection >& conn)  {
		boost::lock_guard< boost::mutex > lock( mutex_ );
		BOOST_FOREACH(ConnectionListener* listener, listeners_) {
			if( listener ) listener->connectionInActive( conn );
		}
	}
}//namespace net
}//namespace comm
