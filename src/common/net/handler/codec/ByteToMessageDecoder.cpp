#include "ByteToMessageDecoder.hpp"
#include "ChannelHandler.hpp"
#include "ChannelInboundEvent.hpp"
#include "DecoderException.hpp"

namespace comm { 
namespace net {
	
	const int ByteToMessageDecoder::STATE_INIT;
	const int ByteToMessageDecoder::STATE_CALLING_CHILD_DECODE;
	const int ByteToMessageDecoder::STATE_HANDLER_REMOVED_PENDING;
		
	shared_ptr<CharBuf> ByteToMessageDecoder::Cumulator::expandCumulation(CharBufAllocator* alloc, shared_ptr<CharBuf>& cumulation, int readable) {
		std::cout << "ByteToMessageDecoder::expandCumulation " << cumulation->readableBytes() << std::endl;
		shared_ptr<CharBuf> oldCumulation = cumulation;
		cumulation = alloc->buffer(oldCumulation->readableBytes() + readable);
		cumulation->writeChars(*oldCumulation);
		oldCumulation->release();
		return cumulation;
	}
	
	shared_ptr<CharBuf> ByteToMessageDecoder::MergeCumulator::cumulate(CharBufAllocator* alloc, 
		shared_ptr<CharBuf>& cumulation, shared_ptr<CharBuf>& in) {
		shared_ptr<CharBuf> buffer;
		if (cumulation->writerIndex() + in->readableBytes() > cumulation->maxCapacity()) {
			buffer = expandCumulation(alloc, cumulation, in->readableBytes());
		} else {
			buffer = cumulation;
		}
		buffer->writeChars(*in);
		return buffer;
	}
	
	static void fireChannelRead(ChannelHandlerContextPtr ctx, const list< shared_ptr<Event> > &msgs, int numElements) {
		list< shared_ptr<Event> >::const_iterator iter;
		for(iter = msgs.begin(); iter != msgs.end(); ++iter) {
			ctx->fireChannelRead(*iter);
		}
	}

	void ByteToMessageDecoder::channelRead(ChannelHandlerContextPtr ctx, const shared_ptr<Event> &event) {
		//std::cout << "ByteToMessageDecoder::channelRead " <<event->eventId() << std::endl;
		if (ChannelInboundEvent::ID() != event->eventId()) {
			ctx->fireChannelRead(event);
			return;
		}
	
		list< shared_ptr<Event> > out;
		try {
			shared_ptr<ChannelInboundEvent> data = boost::dynamic_pointer_cast<ChannelInboundEvent>(event);
			first = !cumulation_;
			if (first) {
				cumulation_ = data->getMessage();
				cumulation_->retain();
			} else {
				cumulation_ = cumulator->cumulate(ctx->alloc(), cumulation_, data->getMessage());
			}
			callDecode(ctx, *cumulation_, out);
			resetCumulation();
			int size = out.size();
			fireChannelRead(ctx, out, size);
		} catch(std::exception &e) {
			std::cerr  << "ByteToMessageDecoder::channelRead " << e.what() << " " << cumulation_->refCnt() << std::endl;
			resetCumulation();
			throw DecoderException(e.what());
		}
	}

	void ByteToMessageDecoder::resetCumulation() {
		//std::cout << "ByteToMessageDecoder::resetCumulation " << cumulation_.get() << " " << cumulation_->readerIndex() << ":" << cumulation_->readableBytes() << std::endl;
		try {
			if (cumulation_ && !cumulation_->isReadable()) {
				numReads = 0;
				cumulation_->release();
				cumulation_.reset();
			} else if (++numReads >= discardAfterReads) {
				numReads = 0;
				discardSomeReadBytes();
			}
		} catch(const std::exception &ex) {
			std::cerr << "ByteToMessageDecoder::resetCumulation " << ex.what() << std::endl;
		}
	}

	void ByteToMessageDecoder::decodeRemovalReentryProtection(ChannelHandlerContextPtr ctx, CharBuf& in, list< shared_ptr<Event> > &out) {
		decodeState = STATE_CALLING_CHILD_DECODE;
		decode(ctx, in, out); 
	}
	
	void ByteToMessageDecoder::callDecode(ChannelHandlerContextPtr ctx, CharBuf& in, list< shared_ptr<Event> > &out) {
		string ex;
		while (in.isReadable()) {
			try {
				int outSize = out.size();
				if (outSize > 0) {
					fireChannelRead(ctx, out, outSize);
					out.clear();
					outSize = 0;
				}

				int oldInputLength = in.readableBytes();
				decodeRemovalReentryProtection(ctx, in, out);
				if (outSize == out.size()) {
					if (oldInputLength == in.readableBytes()) {
						break;
					} else {
						continue;
					}
				}

				if (oldInputLength == in.readableBytes()) {
					throw DecoderException("ByteToMessageDecoder::decode() did not read anything but decoded a message.");
				}

				if (isSingleDecode()) {
					break;
				}
			} catch (std::exception & cause) {
				std::cout <<"[TID#" << boost::this_thread::get_id() << "] ByteToMessageDecoder::callDecode " << cause.what()  << " CharBuf:" << &in << std::endl;
				ex = cause.what();
			}
		}

		if (!ex.empty()) throw DecoderException(ex);
	}

	void ByteToMessageDecoder::discardSomeReadBytes() {
		if (cumulation_ && !first && cumulation_->refCnt() == 1) {
			//discard some bytes if possible to make more room in the buffer
			int before = cumulation_->readerIndex();
			cumulation_->discardSomeReadBytes();
			int after = cumulation_->readerIndex();
			#ifdef DEBUG_LOG
				if (after < before ) std::cout << "ByteToMessageDecoder::discardSomeReadBytes "  << (before - after) << " "  << cumulation_->readableBytes()<< std::endl;
			#endif
		}
	}
	
}//namespace net
}//namespace comm