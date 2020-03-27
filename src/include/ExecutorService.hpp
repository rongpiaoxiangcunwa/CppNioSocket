#ifndef _EXECUTOR_SERVICE_HPP
#define _EXECUTOR_SERVICE_HPP

#include <boost/thread.hpp>
#include <boost/functional.hpp>
#include <boost/smart_ptr.hpp>
#include "ObjectQueue.hpp"
#include "AsyncObject.hpp"
#include <list>
#include <stdexcept>

using std::list;
using boost::shared_ptr;
 
class ExecutorService {
public:
	class Callable {
	public:
		virtual ~Callable() {};
		virtual void call() = 0;
	};
	
	template< typename T >
	class FuncCallable: public Callable {
	public:
		FuncCallable( boost::function< T () > func, const boost::shared_ptr<AsyncObject<T> >& result )
		:func_( func ),
		result_( result )
		{
			BOOST_VERIFY( !func_.empty() );
		}
		
		virtual void call() {
			try {
				result_->set_value(func_());
			}catch( ... ) {
				result_->set_exception( "exception occur when execute method");
			}
		}
	private:
		boost::function< T () > func_;
		boost::shared_ptr<AsyncObject<T> > result_;
	};
	
	class VoidFuncCallable: public Callable {
	public:
		VoidFuncCallable( boost::function< void () > func )
		:func_( func )
		{
			BOOST_VERIFY( !func_.empty() );
		}
		
		virtual void call() {
			try {
				func_();
			}catch( ... ) {
			}
		}
	private:
		boost::function< void () > func_;
	};
	
	class SharedCallableWrapper: public Callable {
	public:
		SharedCallableWrapper( const boost::shared_ptr<Callable>& callable )
		:callable_( callable )
		{			
		}
		
		virtual void call() {
			try {
				callable_->call();
			}catch( ... ) {
			}
		}
	private:
		boost::shared_ptr<Callable> callable_;
	};
	
	struct VoidFunctionToCallable {
		Callable* operator()( const  boost::function<void()>& func ) {
			return func.empty()? 0: new VoidFuncCallable( func );
		}
	};
	
	template<typename T>
	struct FunctionToCallable {
		Callable* operator()( const boost::function<T()>& func ) {
			return func.empty() ? 0: new FuncCallable<T>( func );
		}
	};
	
	struct SharedCallableToCallable {
		Callable* operator()( const boost::shared_ptr<Callable>& callable ) {
			return callable ? new SharedCallableWrapper( callable ): 0;
		}
	};
		
public:
	ExecutorService( int threadNums )
	:stop_( false ),
	pause_( false ),
	threadNum_( threadNums )
	{
		for( int i = 0; i < threadNums; i++ ) {
			procThreads_.push_back( shared_ptr< boost::thread >( new boost::thread( boost::bind( &ExecutorService::takeProcTask, this ) ) ) );
		}
	}

	~ExecutorService() {
		shutdown();
	}
	
	template< typename T >
	boost::shared_ptr< AsyncObject<T> > submit( const boost::function< T () >& task ) {
		boost::shared_ptr< AsyncObject<T> > result( new AsyncObject<T>() );
		
		if( task.empty() || pause_ ) {
			result->set_exception( "paused");
		} else {
			Callable* callable = new FuncCallable<T>( task, result );
			tasks_.add( callable );
		}
		return result;
	}
		
	
	bool submit( const boost::function< void () >& task ) {
		if( task.empty() || pause_ ) {
			return false;
		}
		Callable* callable = new VoidFuncCallable( task );
		tasks_.add( callable );
		return true;
	}
	
	bool submit( const std::list< boost::function< void () > >& tasks ) {
		return addTasks< boost::function< void () >, VoidFunctionToCallable >( tasks );					
	}
	
		
	bool submit( const shared_ptr< Callable >& task ) {
		if( pause_ || !task ) {
			return false;
		}

		Callable* callable = new SharedCallableWrapper( task );			
		tasks_.add( callable );
		return true;
	}
	
	bool submit( const std::list<shared_ptr< Callable > >& tasks ) {
		return addTasks<shared_ptr< Callable >, SharedCallableToCallable >( tasks );		
	}
	
	int size() const {
		return tasks_.size();
	}
	
	void shutdown( bool waitAllTaskFinish = true ) {
		if( stop_ ) {
			return;
		}
		
		pause_ = true;
		
		if( waitAllTaskFinish ) {
			while( tasks_.size() > 0 ) {
				::sleep( 1 );
			}
		}
		
		stop_ = true;
		
		for( list< shared_ptr< boost::thread > >::iterator iter = procThreads_.begin(); iter != procThreads_.end(); iter++ ) {
			shared_ptr< boost::thread > p = *iter;
			
			if( p && p->joinable() && p->get_id() != boost::this_thread::get_id() ) {
				p->interrupt();
				p->join();
			}
		}
	}
private:
	template<typename T, typename Converter >
	static void toCallableList( const std::list<T>& t, std::vector<Callable*>& callableList ) {
		Converter converter;
		for( typename std::list<T>::const_iterator iter = t.begin(); iter != t.end(); iter++ ) {
			Callable* callable = converter( *iter );
			if( callable ) {
				callableList.push_back( callable );
			}
		}
		
	}
	
	template< typename T, typename Converter >
	bool addTasks( const std::list<T>& tasks ) {
		if( tasks.empty() || pause_ ) {
			return false;
		}
		
		std::vector<Callable*> callableList;
		toCallableList<T, Converter>( tasks, callableList );
		
		if( callableList.empty() ){
			return false;
		}
		tasks_.add( callableList );
		return true;
	}
		
private:
	void takeProcTask() {
		
		while( !stop_ ) {
			try {
				Callable* task = tasks_.take();
				
				if( task ) {
					task->call();
					delete task;
				}				
			}catch( ... ) {
			}
		}
	}
	
private:
	volatile bool stop_;
	volatile bool pause_;			
	ObjectQueue< Callable* > tasks_;
	int threadNum_;			
	list< shared_ptr< boost::thread > > procThreads_;
};

#endif/*_EXECUTOR_SERVICE_HPP*/
