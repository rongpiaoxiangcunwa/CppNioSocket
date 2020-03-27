#ifndef _OBJECT_QUEUE_H
#define _OBJECT_QUEUE_H

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


template< typename T>
class ObjectQueue {
private:
    struct Item {
        Item( const T& _data )
        :data( _data ),
        next( 0 )
        {
        }
        T data;
        Item* next;
    };
    
public:
    ObjectQueue()
    :head_( 0 ),
    tail_( 0 ),
    size_( 0 )
    {
    }
    
    ~ObjectQueue() {
    	clear();
	}
		
	/**
	 * timeout in milliseconds:
	 * 	  <0 wait for until data is available
	 * 	  0 return immediatelly
	 * 	  >	wait for timeout milliseconds 	 
	 */	 	
	T take( int timeout = -1 ) {
		boost::lock_guard<boost::mutex> lock(mutex_);
	    
		try {
			if( size_ <= 0 ) {
				if( timeout < 0 ) {
					cond_.wait( mutex_ );
				} else if( timeout > 0 ) {
					cond_.timed_wait( mutex_, boost::get_system_time() + boost::posix_time::milliseconds( timeout ) );
				}
			}
		}catch( ... ) {
		}
		
		//maybe interrupted
		if( size_ <= 0 ) {
			return T();
		}
		
        
		T data = head_->data;
        Item* p = head_->next;
        delete head_; head_ = p;
        size_ --;
        
		return data;
 	}
 	
 	template< typename List >
 	void take( List& l, int drain_num = 0, int timeout = -1 ) {
		boost::lock_guard<boost::mutex> lock(mutex_);
	
		try {
			if( size_ <= 0 ) {
				if( timeout < 0 ) {
					cond_.wait( mutex_ );
				} else if( timeout > 0 ) {
					cond_.timed_wait( mutex_, boost::get_system_time() + boost::posix_time::milliseconds( timeout ) );

				}
			}
		}catch( ... ) {
		}
		
		//maybe interrupted
		if( size_ <= 0 ) {
			return;
		}
		
        drain_num = (drain_num <= 0)?size_:( ( drain_num < size_ ) ? drain_num:size_ );
        
        for( ;drain_num > 0; drain_num-- ) {
            BOOST_VERIFY( head_ != 0 && size_ > 0 );            
            l.push_back( head_->data );
            Item* p = head_->next;
            delete head_; head_ = p;
            size_ --;
        }
 	}
 	
 	T peek() {
 		boost::lock_guard<boost::mutex> lock(mutex_);
 		
 		if( size_ <= 0 ) {
 			return T();
		}
		
        BOOST_VERIFY( head_ != 0 );
        
		return head_->data;
	}
	int add( const T& data ) {
		int n = 0;
		
		{
			boost::lock_guard<boost::mutex> lock(mutex_);
	
			addItem( new Item( data ) );
            n = size_;
		}
 		cond_.notify_one();
 		
 		return n;
	}
	
	template< typename List >
	int add( const List& datas ) {
		int n = 0;
		
		{
			boost::lock_guard<boost::mutex> lock(mutex_);
			
			typename List::const_iterator iter;
			
			for( iter = datas.begin(); iter != datas.end(); iter++ ) {
				addItem( new Item( *iter ) );
			}
			
			n = size_;	
		}
 		cond_.notify_one();
 		
 		return n;
	}
	
	int size() const {
		return size_;
	}

	void clear() {
		boost::lock_guard<boost::mutex> lock(mutex_);

		while( head_ ) {
            Item* p = head_->next;
            delete head_; head_ = p;
            size_ --;
        }
        
        BOOST_VERIFY( size_ == 0 );
	}
private:
	void addItem( Item* p ) {
		if( head_ == 0 ) {
            BOOST_VERIFY( size_ == 0 );
            head_ = tail_ = p;
        } else {
            BOOST_VERIFY( size_ > 0 && tail_ != 0 );                
            tail_->next = p;
            tail_ = p;
        }
        size_ ++;
	}
	
private:
	boost::condition_variable_any cond_;
	boost::mutex mutex_;
	Item* head_;
    Item* tail_;
    volatile int size_;	
};

#endif /*_OBJECT_QUEUE_H*/

