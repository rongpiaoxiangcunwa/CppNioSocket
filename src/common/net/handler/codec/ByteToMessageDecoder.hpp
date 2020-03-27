#ifndef _ByteToMessageDecoder_hpp_
#define _ByteToMessageDecoder_hpp_

#include <list>
#include "ChannelInboundHandlerAdapter.hpp"
#include "CharBufAllocator.hpp"
using namespace comm::buffer;
using std::list;

namespace comm { 
namespace net {
	class ByteToMessageDecoder : public ChannelInboundHandlerAdapter {
	public:
		class Cumulator {
		public:
			static shared_ptr<CharBuf> expandCumulation(CharBufAllocator* alloc, shared_ptr<CharBuf>& cumulation, int readable);
			
			virtual ~Cumulator() {}
			virtual shared_ptr<CharBuf> cumulate(CharBufAllocator* alloc, shared_ptr<CharBuf>& cumulation, shared_ptr<CharBuf>& in) = 0;
		};

		class MergeCumulator: public Cumulator {
		public:
			virtual shared_ptr<CharBuf> cumulate(CharBufAllocator* alloc, shared_ptr<CharBuf>& cumulation, shared_ptr<CharBuf>& in);
		};
		
	public:
		ByteToMessageDecoder() : cumulator(new MergeCumulator()), singleDecode(false), 
		decodeWasNull(false), first(true), decodeState(STATE_INIT), discardAfterReads(4), numReads(0) {
		}
	
		virtual ~ByteToMessageDecoder() {
			delete cumulator;
		}
	
		virtual void channelInactive(ChannelHandlerContextPtr ctx) {
			if (cumulation_) cumulation_->release();
			cumulation_.reset();
			ctx->fireChannelInactive();
		}
		
		virtual void channelRead(ChannelHandlerContextPtr ctx, const shared_ptr<Event> &event) ;
		void setSingleDecode(bool _singleDecode) { singleDecode = _singleDecode;}
		bool isSingleDecode() const { return singleDecode; }
		
	protected:
		virtual void decode(ChannelHandlerContextPtr ctx, CharBuf &in, list< shared_ptr<Event> > &out)  = 0;
		void callDecode(ChannelHandlerContextPtr ctx, CharBuf& in, list< shared_ptr<Event> > &out) ;
		void decodeRemovalReentryProtection(ChannelHandlerContextPtr ctx, CharBuf& in, list< shared_ptr<Event> > &out);
		void resetCumulation();
		void discardSomeReadBytes();
		
	private:
		static const int STATE_INIT = 0;
		static const int STATE_CALLING_CHILD_DECODE = 1;
		static const int STATE_HANDLER_REMOVED_PENDING = 2;

		shared_ptr<CharBuf> cumulation_;
		Cumulator* cumulator;
		bool singleDecode;
		bool decodeWasNull;
		volatile bool first;
		int decodeState;
		int discardAfterReads;
		int numReads;
	};
}//namespace net
}//namespace comm

#endif