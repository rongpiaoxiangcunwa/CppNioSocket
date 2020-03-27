#include "DefaultChannelPipeline.hpp"
#include "ChannelInboundEvent.hpp"
#include "ChannelOutboundEvent.hpp"
#include <typeinfo>
#include <boost/format.hpp>

namespace comm { 
namespace net {
	static void cleanupNameCaches(std::map<string, string> *cache) {
		delete cache;
	}	
	
	 boost::thread_specific_ptr< std::map<string, string>  > DefaultChannelPipeline::nameCaches(cleanupNameCaches);

	 void DefaultChannelPipeline::checkDuplicateName(const string &name) {
		if (context0(name)) {
			throw std::runtime_error( (boost::format("Duplicate handler name:%s") % name.c_str()).str().c_str());
		}
	}

	string DefaultChannelPipeline::generateName(const shared_ptr<ChannelHandler> &handler) {
		std::map<string, string> *cache =  nameCaches.get();
		string handlerType = typeid(*handler).name();
		string name;
		
		if (cache == NULL)   {
			cache = new std::map<string, string>();
			nameCaches.reset(cache);
		}

		std::map<string, string>::const_iterator mit= cache->find(handlerType);
		if (mit == cache->end()) {
			name = generateName0(handlerType);
			cache->insert(std::make_pair(handlerType, name));
		}

		if (context0(name) != NULL) {
			string baseName = name.substr(0, name.length()  - 1);
			for (int i = 0; ; ++i) {
				string newName = (boost::format("%s%d")  % baseName.c_str()  % i).str();
				if (context0(newName) == NULL) {
					name = newName;
					break;
				}
			}			
		}
 		return name;
	}

	string DefaultChannelPipeline::generateName0(const string& clzName) {
		return  clzName + "#0";
	}

	void DefaultChannelPipeline::HeadContext::write(ChannelHandlerContextPtr ctx, const shared_ptr<Event>& event, const EventCallBack& callback) {
		if (!event)  {
			if (!callback.empty()) callback(false);
			return;
		}
		
		if (event->eventId() == ChannelOutboundEvent::ID()) {
			shared_ptr<ChannelOutboundEvent> outEvent = boost::dynamic_pointer_cast<ChannelOutboundEvent>(event);
			if (outEvent && outEvent->getMessage()) {
				connection()->sendMessage(*outEvent->getMessage(), callback);
			}
		}
	}

	void DefaultChannelPipeline::TailContext::channelRead(ChannelHandlerContextPtr ctx, const shared_ptr<Event> &event) {
		if (event->eventId() == ChannelInboundEvent::ID()) {
			boost::shared_ptr<ChannelInboundEvent> inEvent = boost::dynamic_pointer_cast<ChannelInboundEvent>(event);
			inEvent->getMessage()->release();
		}
	}
}//namespace net
}//namespace comm