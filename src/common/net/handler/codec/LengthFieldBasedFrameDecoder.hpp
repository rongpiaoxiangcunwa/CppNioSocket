#ifndef _LengthFieldBasedFrameDecoder_hpp_
#define _LengthFieldBasedFrameDecoder_hpp_

#include <algorithm>
#include "SystemUtil.hpp"
#include "DecoderException.hpp"
#include "ByteToMessageDecoder.hpp"
#include "ChannelInboundEvent.hpp"

using namespace comm::buffer;

namespace comm { 
namespace net {
	class LengthFieldBasedFrameDecoder : 
		public ByteToMessageDecoder, 
		public boost::enable_shared_from_this<LengthFieldBasedFrameDecoder>
	{
	public:
		LengthFieldBasedFrameDecoder(int maxFrameLength, int lengthFieldOffset, int lengthFieldLength) {
			init(maxFrameLength, lengthFieldOffset, lengthFieldLength, 0, 0, true);
		}

		LengthFieldBasedFrameDecoder(int maxFrameLength, int lengthFieldOffset, int lengthFieldLength, int lengthAdjustment, int initialBytesToStrip) {
			init(maxFrameLength, lengthFieldOffset, lengthFieldLength, lengthAdjustment, initialBytesToStrip, true);
		}

		virtual shared_ptr<ChannelHandler> getSharedPtr() {
			return shared_from_this();
		}

	protected:
		virtual void decode(ChannelHandlerContextPtr ctx, CharBuf &in, list< shared_ptr<Event> > &out) {
			shared_ptr<Event> decoded = decode(ctx, in);
			if (decoded ) {
				out.push_back(decoded);
			}
		}

		shared_ptr<Event> decode(ChannelHandlerContextPtr ctx, CharBuf& in) {
			if (discardingTooLongFrame_) {
				discardingTooLongFrame(in);
			}

			if (in.readableBytes() < lengthFieldEndOffset_) {
				return shared_ptr<Event>();
			}

			int actualLengthFieldOffset = in.readerIndex() + lengthFieldOffset_;
			long frameLength = getUnadjustedFrameLength(in, actualLengthFieldOffset, lengthFieldLength_);

			if (frameLength < 0) {
				failOnNegativeLengthField(in, frameLength, lengthFieldEndOffset_);
			}

			frameLength += lengthAdjustment_+ lengthFieldEndOffset_;
			if (frameLength < lengthFieldEndOffset_) {
				failOnFrameLengthLessThanLengthFieldEndOffset(in, frameLength, lengthFieldEndOffset_);
			}

			if (frameLength > maxFrameLength_) {
				exceededFrameLength(in, frameLength);
				return shared_ptr<Event>();
			}

			// never overflows because it's less than maxFrameLength
			int frameLengthInt = (int) frameLength;
			if (in.readableBytes() < frameLengthInt) {
				return shared_ptr<Event>();
			}

			if (initialBytesToStrip_ > frameLengthInt) {
				failOnFrameLengthLessThanInitialBytesToStrip(in, frameLength, initialBytesToStrip_);
			}
			in.skipBytes(initialBytesToStrip_);

			// extract frame
			int readerIndex = in.readerIndex();
			int actualFrameLength = frameLengthInt - initialBytesToStrip_;
			shared_ptr<CharBuf> frame = extractFrame(in, readerIndex, actualFrameLength);
			in.readerIndex(readerIndex + actualFrameLength);
			//std::cout << "LengthFieldBasedFrameDecoder::extractFrame " << in.readableBytes() << std::endl;
			return shared_ptr<ChannelInboundEvent>(new ChannelInboundEvent(frame));
		}

		shared_ptr<CharBuf> extractFrame(CharBuf& buffer, int index, int length) {
        	return buffer.retainedSlice(index, length);
    	}

		void failOnFrameLengthLessThanInitialBytesToStrip(CharBuf& in, long frameLength, int initialBytesToStrip) {
			in.skipBytes((int) frameLength);
			throw DecoderException((boost::format("Adjusted frame length (%d) is less than initialBytesToStrip: %d") % frameLength %  initialBytesToStrip).str());
		}
		 
		void exceededFrameLength(CharBuf& in, long frameLength) {
			long discard = frameLength - in.readableBytes();
			//std::cout << "LengthFieldBasedFrameDecoder::exceededFrameLength() frameLength " << frameLength << " readerIndex:" << in.readerIndex() << " readableBytes:" <<  in.readableBytes() << " discard:"  << discard << std::endl;
			tooLongFrameLength_ = frameLength;
			if (discard <= 0) {
				//buffer contains more bytes then the frameLength so we can discard all now
				in.skipBytes(frameLength);
			} else {
				//Enter the discard mode and discard everything received so far.
				discardingTooLongFrame_ = true;
				bytesToDiscard_ = discard;
				in.skipBytes(in.readableBytes());
			}
			failIfNecessary(true);
	    }
		
		void failOnFrameLengthLessThanLengthFieldEndOffset(CharBuf& in, long frameLength, int lengthFieldEndOffset) {
			in.skipBytes(lengthFieldEndOffset);
			throw DecoderException((boost::format("Adjusted frame length (%d) is less than lengthFieldEndOffset: %d") % frameLength %  lengthFieldEndOffset).str());
		}

		long getUnadjustedFrameLength(CharBuf &buf, int offset, int length) {
			long frameLength = 0;
			switch (length) {
				case 1:
					frameLength = (long)buf.getChar(offset);
				break;
				case 2:
					frameLength = (long)buf.getUShort(offset);
				break;
				//case 3:
				//	frameLength = (long)buf.getUMedium(offset);
				//break;
				case 4:
					frameLength = (long)buf.getUInt(offset);
				break;
				case 8:
					frameLength = (long)buf.getLong(offset);
				break;
				default:
					throw DecoderException((boost::format("unsupported lengthFieldLength: %d (expected: 1, 2, 4, or 8)") % lengthFieldLength_).str());
			}
			return frameLength;
		}

		void failOnNegativeLengthField(CharBuf& in, long frameLength, int lengthFieldEndOffset) {
			in.skipBytes(lengthFieldEndOffset);
			throw DecoderException((boost::format("negative pre-adjustment length field: %d") % frameLength).str());
		}

		void discardingTooLongFrame(CharBuf &in) {
			long bytesToDiscard = bytesToDiscard_;
			int localBytesToDiscard = (int) std::min((int)bytesToDiscard, in.readableBytes());
			in.skipBytes(localBytesToDiscard);
			bytesToDiscard -= localBytesToDiscard;
			bytesToDiscard_ = bytesToDiscard;
			//std::cout << "LengthFieldBasedFrameDecoder::discardingTooLongFrame() discard " << localBytesToDiscard << " bytesToDiscard_" <<  bytesToDiscard_ << std::endl;
			failIfNecessary(false);
		}

		void failIfNecessary(bool firstDetectionOfTooLongFrame) {
	        if (bytesToDiscard_ == 0) {
	            // Reset to the initial state and tell the handlers that the frame was too large.
	            long tooLongFrameLength = tooLongFrameLength_;
	            tooLongFrameLength_ = 0;
	            discardingTooLongFrame_ = false;
	            if (!failFast_ || firstDetectionOfTooLongFrame) {
	                fail(tooLongFrameLength);
	            }
	        } else {
	            //Keep discarding and notify handlers if necessary.
	            if (failFast_ && firstDetectionOfTooLongFrame) {
	                fail(tooLongFrameLength_);
	            }
	        }
    	}

		void fail(long frameLength) {
			if (frameLength > 0) {
				throw DecoderException((boost::format("Adjusted frame length exceeds %d : %d - discarded") % maxFrameLength_ % frameLength).str());
			} else {
				throw DecoderException((boost::format("Adjusted frame length exceeds %d : - discarding") % maxFrameLength_).str());
			}
		}

		void init(int maxFrameLength, int lengthFieldOffset, int lengthFieldLength, int lengthAdjustment, int initialBytesToStrip, bool failFast) {
			SystemUtil::checkPositive(maxFrameLength, "maxFrameLength");
			SystemUtil::checkPositiveOrZero(lengthFieldOffset, "lengthFieldOffset");
			SystemUtil::checkPositiveOrZero(initialBytesToStrip, "initialBytesToStrip");
			if (lengthFieldOffset > maxFrameLength - lengthFieldLength) {
				throw IllegalArgumentException((boost::format("maxFrameLength(%d) must be equal to or greater than lengthFieldOffset(%d) + lengthFieldLength(%d).") 
					% maxFrameLength % lengthFieldOffset % lengthFieldLength).str());
			}

			maxFrameLength_ = maxFrameLength;
			lengthFieldOffset_ = lengthFieldOffset;
			lengthFieldLength_ = lengthFieldLength;
			lengthFieldEndOffset_ = lengthFieldOffset + lengthFieldLength;
			lengthAdjustment_ = lengthAdjustment;
			initialBytesToStrip_ = initialBytesToStrip;
			failFast_ = failFast;
			discardingTooLongFrame_ = false;
			tooLongFrameLength_ = 0;
			bytesToDiscard_ = 0;
		}
		
	private:
		int maxFrameLength_;
		int lengthFieldOffset_;
		int lengthFieldLength_;
		int lengthFieldEndOffset_;
		int lengthAdjustment_;
		int initialBytesToStrip_;
		volatile bool failFast_;
		volatile bool discardingTooLongFrame_;
		volatile long tooLongFrameLength_;
		volatile long bytesToDiscard_;
	};	
}//namespace net
}//namespace comm
#endif
