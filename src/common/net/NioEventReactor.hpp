#ifndef _NioEventReactor_HPP_
#define _NioEventReactor_HPP_

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

using boost::shared_ptr;

namespace comm { 
namespace net {
	
class NioEventReactor
{
public:
	NioEventReactor();
	~NioEventReactor();	
	boost::asio::io_service& get_io_service() { return io_service_; }
	void start();
	bool isInEventLoop();
	void shutdownGracefully();
	
private:
	void run();
	
private:
	volatile bool stop_;
	boost::mutex mutex_;
	boost::asio::io_service io_service_;
	boost::asio::io_service::work io_service_work_;
	shared_ptr<boost::thread> serviceRunThread_;
};
}//namespace net
}//namespace comm
#endif
