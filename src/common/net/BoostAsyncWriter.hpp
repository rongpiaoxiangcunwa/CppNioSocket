#ifndef _BoostAsyncWriter_HPP_
#define _BoostAsyncWriter_HPP_

#include <queue>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#ifdef USE_BOOST_1_68
#include <boost/asio/spawn.hpp>
#include <boost/asio/strand.hpp>
#endif
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <utility>

namespace comm { 
namespace net {

template<typename SocketType = boost::asio::ip::tcp::socket>
class BoostAsyncWriter : public boost::enable_shared_from_this <BoostAsyncWriter<SocketType> >
{
public:
	BoostAsyncWriter(const boost::shared_ptr<SocketType>& socket)
		: shutdown_(false), socket_(socket), strand_(socket_->get_io_service() ) {	
	}

	void write( const std::string& message, const boost::function< void( bool ) >& resultCb ){
    #ifdef USE_BOOST_1_68
        boost::asio::spawn(strand_, boost::bind( &BoostAsyncWriter::writeImpl, get_shared_ptr(), message, resultCb ) );
    #else
         strand_.post( boost::bind( &BoostAsyncWriter::writeImpl, get_shared_ptr(), message, resultCb ) );
    #endif
    }
    
    void shutdown( const boost::function<void()>& shutdownCallback ) {
     #ifdef USE_BOOST_1_68
    	boost::asio::spawn(strand_,  boost::bind( &BoostAsyncWriter::handleShutdown, get_shared_ptr(), shutdownCallback ) );
     #else
        strand_.post( boost::bind( &BoostAsyncWriter::handleShutdown, get_shared_ptr(), shutdownCallback ) );
     #endif
	}
	
	void shutdown() {
		shutdown( boost::bind( &BoostAsyncWriter::ignoreShutdownResult ) );
	}

	shared_ptr< BoostAsyncWriter<SocketType> > get_shared_ptr() { 
		return boost::enable_shared_from_this <BoostAsyncWriter<SocketType> >::shared_from_this();
	}
	
private:
	void handleShutdown( boost::function<void()> shutdownCallback ) {
		shutdown_ = true;		
		shutdownCallback();
	}
	
    void writeImpl( std::string message, boost::function< void( bool ) > resultCb ) {
    	//return immediatelly if already shutdown
    	if( shutdown_ ) {
    		resultCb( false );
		} else {		
	        outQueue_.push( std::make_pair(message, resultCb) );
	        if ( outQueue_.size() == 1 ) {
	        	this->write();
	        }
        }
    }

    void write() {
        const std::string& message = outQueue_.front().first;
        boost::asio::async_write( *socket_,
                boost::asio::buffer( message.data(), message.size() ),
                 boost::bind( &BoostAsyncWriter::handleWrite, get_shared_ptr(), _1, _2 )  );
    }

    void handleWrite( const boost::system::error_code& error, const size_t bytesTransferred ) {
    	std::pair<std::string, WriteResultCallback >& firstElem = outQueue_.front();
    	if( error || bytesTransferred >= firstElem.first.size() ) {
    		boost::function< void( bool ) > resultCb = firstElem.second;
    		outQueue_.pop();
    		if( shutdown_ ) {
	        	resultCb( !error );                
	        	clearQueue();
				return ;
			}
	
			if ( !outQueue_.empty() ) {
	            this->write();            
	        }
	
			resultCb( !error );
		} else {
			//write remain
			firstElem.first.erase(0, bytesTransferred );
			this->write();
		} 
    }
    
    void clearQueue() {
    	while( !outQueue_.empty() ) {
			outQueue_.front().second( false );
			outQueue_.pop();
		}
	}
	
	static void ignoreShutdownResult() {
	}

private:
	typedef boost::function< void( bool ) > WriteResultCallback;
	volatile bool shutdown_;
	boost::shared_ptr<SocketType> socket_;
#ifdef USE_BOOST_1_68
	boost::asio::io_context::strand strand_;
#else  
	boost::asio::strand strand_;
#endif
	std::queue< std::pair<std::string, WriteResultCallback > > outQueue_;
};
		
}//namespace net
}//namespace comm
#endif
