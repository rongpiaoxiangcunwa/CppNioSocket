#ifndef COMM_BUFFER_ABSTRACT_POOLED_DERIVED_CHARBUF_HPP
#define COMM_BUFFER_ABSTRACT_POOLED_DERIVED_CHARBUF_HPP

#include "AbstractReferenceCountedCharBuf.hpp"

namespace comm{ namespace buffer{

	class AbstractPooledDerivedCharBuf : public AbstractReferenceCountedCharBuf {
	public:	
		AbstractPooledDerivedCharBuf() : AbstractReferenceCountedCharBuf(0) {
		}
		
		void init(const shared_ptr<AbstractCharBuf>& unwrapped, const shared_ptr<CharBuf> &wrapped, int _readerIndex, int _writerIndex, int _maxCapacity) {
			wrapped->retain(); // Retain up front to ensure the parent is accessible before doing more work.
	        parent_ = wrapped;
	        rootParent_ = unwrapped;
            maxCapacity(_maxCapacity);
            setIndex0(_readerIndex, _writerIndex); // It is assumed the bounds checking is done by the caller.
            resetRefCnt();
		}

		virtual shared_ptr<CharBuf> slice(int index, int length) {
			//todo
			throw std::runtime_error("Unsupported slice(int newCapacity, int length) in PooledSlicedCharBuf");
		}
		
		virtual shared_ptr<CharBuf> retainedSlice(int index, int length) {
			//todo
			throw std::runtime_error("Unsupported retainedSlice(int index, int length) in PooledSlicedCharBuf");
		}

		virtual AbstractCharBuf* unwrap() const { return rootParent_.get(); }

		virtual char* array() { return unwrap()->array(); }

	protected:
		virtual void deallocate() { 
			 parent_->release();
		}
		
	private:
		shared_ptr<AbstractCharBuf> rootParent_;
		shared_ptr<CharBuf> parent_;
	};

}}//namespace

#endif
