#ifndef _AtomicInteger_HPP_
#define _AtomicInteger_HPP_

#include<boost/thread/mutex.hpp>
#ifdef USE_BOOST_1_68
#include<boost/thread/lock_guard.hpp>
#endif
template<typename IntType>
class AtomicInteger
{
public:
	typedef boost::mutex MutexType;
	AtomicInteger( IntType i ) {
		val_ = i;
	}
	
	AtomicInteger() :val_() {}
	
	AtomicInteger( const AtomicInteger<IntType>& right )
	:val_( right.get() ) {
	}

	AtomicInteger& operator=(const AtomicInteger& right) {
		if(this == &right) return *this;
		set( right.get() );
		return *this;
	}

	AtomicInteger<IntType>& operator+=( IntType i ) {
		incrementAndGet( i );		
		return *this;
	}

	AtomicInteger<IntType>& operator-=( IntType i ) {
		descrementAndGet( i );		
		return *this;
	}

	AtomicInteger& operator++() {
		incrementAndGet(1);
		return *this;
	}

	AtomicInteger operator++(int) {
		AtomicInteger t = *this;
		incrementAndGet(1);
		return t;
	}

	AtomicInteger& operator-() {
		descrementAndGet(1);
		return *this;
	}

	AtomicInteger operator--(int) {
		AtomicInteger t = *this;
		descrementAndGet(1);
		return t;
	}

	bool operator!() const {
		return get() != 0;
	}

	bool operator==( const AtomicInteger<IntType>& right ) const {
		return get() == right.get();
	}

	bool operator!=( const AtomicInteger<IntType>& right ) const {
		return get() != right.get();
	}
	
	bool operator>( const AtomicInteger<IntType>& right ) const {
		return get() > right.get();
	}

	bool operator>=( const AtomicInteger<IntType>& right ) const {
		return get() >= right.get();
	}
	
	bool operator<( const AtomicInteger<IntType>& right ) const {
		return get() < right.get();
	}
	
	bool operator<=( const AtomicInteger<IntType>& right ) const {
		return get() <= right.get();
	}
	
	operator IntType() const {
		boost::lock_guard<MutexType> lock(mutex_);
		return val_;
	}
	
	void set( IntType i ) {
		boost::lock_guard<MutexType> lock(mutex_);
		val_ = i;
	}
	
	IntType get() const {
		boost::lock_guard<MutexType> lock(mutex_);
		return val_;
	}

	IntType getAndSet( IntType i ) {
		boost::lock_guard<MutexType> lock(mutex_);		
		IntType t = val_;
		val_ = i;
		return t;
	}

	bool compareAndSet( IntType expect, IntType update ) {
		boost::lock_guard<MutexType> lock(mutex_);	
		if( val_ == expect ) {
			val_ = update;
			return true;
		}
		return false;
	}
	
	template< typename UnaryPredicate >
	bool compareAndSet( UnaryPredicate pred, IntType update ) {		
		boost::lock_guard<MutexType> lock(mutex_);
		if( pred( val_ ) ) {
			val_ = update;
			return true;
		}
		return false;
	}
	
	IntType getAndIncrement(IntType i = 1) {
		boost::lock_guard<MutexType> lock(mutex_);
		IntType t = val_;
		val_ += i;
		return t;
	}

	IntType incrementAndGet( IntType i = 1 ) {
		boost::lock_guard<MutexType> lock(mutex_);		
		val_ += i;
		return val_;
	}

	IntType getAndDescrement( IntType i = 1) {
		boost::lock_guard<MutexType> lock(mutex_);
		IntType t = val_;
		val_ -= i;
		return t;	
	}
	
	IntType descrementAndGet(IntType i = 1) {
		boost::lock_guard<MutexType> lock(mutex_);		
		val_ -= i;
		return val_;
	}
	
private:
	mutable MutexType mutex_;
	IntType val_;
};

#endif
