#ifndef _ChannelHandler_hpp_
#define _ChannelHandler_hpp_

#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include "ChannelHandlerContext.hpp"

using boost::shared_ptr;

namespace comm { 
namespace net {
class  ChannelHandler {
public:
	virtual shared_ptr<ChannelHandler> getSharedPtr() = 0;
	virtual void exceptionCaught(ChannelHandlerContextPtr ctx, const std::exception &ex) = 0;
};

}//namespace net
}//namespace comm
#endif
