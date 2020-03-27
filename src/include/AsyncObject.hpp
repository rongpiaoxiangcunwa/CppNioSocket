#ifndef _ASYNC_OBJECT_HPP
#define _ASYNC_OBJECT_HPP

#include <string>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
using std::string;

template<typename T >
class AsyncObject  {
private:
	enum Status { uninitialized, st_value, st_exception, st_timeout };
public:
		
	AsyncObject()
	:status_( uninitialized )
	{
	}
	
	void set_value( const T& value ) {
		{
			boost::unique_lock< boost::mutex > lock( resultMutex_ );
			status_ = st_value;
			value_ = value;
		}
		
		cond_.notify_all();
	}
	
	void set_exception( const string& exceptionDesc = "unknown") {
		{
			boost::unique_lock< boost::mutex > lock( resultMutex_ );
			
			status_ = st_exception;
			exceptionDesc_ = exceptionDesc;
		}
		
		cond_.notify_all();
	}
	
	const T& get() const {
		BOOST_VERIFY( status_ == st_value );
		
		return value_;
	}
	
	const T& get_value() const {
		BOOST_VERIFY( status_ == st_value );
		
		return value_;
	}
	
	const string& getExceptionDesc() const {
		BOOST_VERIFY( status_ == st_exception );
		
		return exceptionDesc_;
	}
	
	void wait() {
		boost::unique_lock< boost::mutex > lock( resultMutex_ );
		
		if( status_ == uninitialized ) {
			cond_.wait( lock );
		}
	}
	
	void wait_for( int millis ) {
		boost::unique_lock< boost::mutex > lock( resultMutex_ );
		
		if( status_ == uninitialized ) {
			cond_.timed_wait( lock, boost::get_system_time() + boost::posix_time::milliseconds( millis ) );
		}
		
		if( status_ == uninitialized ) {
			status_ = st_timeout;
		}
	}
	
	bool has_value() const {
		return status_ == st_value;
	}
	
	bool has_exception() const {
		return status_ == st_exception;
	}
	
	bool is_timeout() const {
		return status_ == st_timeout;
	}		
private:
	volatile Status status_;
	boost::mutex resultMutex_;
	boost::condition_variable cond_;	
	T value_;
	string exceptionDesc_;
};

template<>
class AsyncObject<void> {
private:
	enum Status { uninitialized, st_value, st_exception, st_timeout };
public:
		
	AsyncObject()
	:status_( uninitialized )
	{
	}
	
	void set_value() {
		{
			boost::unique_lock< boost::mutex > lock( resultMutex_ );
			status_ = st_value;
		}
		cond_.notify_all();
	}
	
	void set_exception( const string& exceptionDesc = "unknown") {
		{
			boost::unique_lock< boost::mutex > lock( resultMutex_ );
			
			status_ = st_exception;
			exceptionDesc_ = exceptionDesc;
		}
		
		cond_.notify_all();
	}
	
	const string& getExceptionDesc() const {
		BOOST_VERIFY( status_ == st_exception );
		
		return exceptionDesc_;
	}
	
	void wait() {
		boost::unique_lock< boost::mutex > lock( resultMutex_ );
		
		if( status_ == uninitialized ) {
			cond_.wait( lock );
		}
	}
	
	void wait_for( int millis ) {
		boost::unique_lock< boost::mutex > lock( resultMutex_ );
		
		if( status_ == uninitialized ) {
			cond_.timed_wait( lock, boost::get_system_time() + boost::posix_time::milliseconds( millis ) );
		}
		
		if( status_ == uninitialized ) {
			status_ = st_timeout;
		}
	}
	
	bool has_value() const {
		return status_ == st_value;
	}
	
	bool has_exception() const {
		return status_ == st_exception;
	}
	
	bool is_timeout() const {
		return status_ == st_timeout;
	}		
private:
	volatile Status status_;
	boost::mutex resultMutex_;
	boost::condition_variable cond_;	
	string exceptionDesc_;
};

#endif/*_ASYNC_OBJECT_HPP*/
