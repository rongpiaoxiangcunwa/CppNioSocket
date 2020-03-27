#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boostsctp/asio/detail/sctp_socket_ops.hpp>
#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace detail {
namespace sctp_socket_ops {
	
	using namespace boost::asio::detail::socket_ops;

	inline void clear_last_error()
	{
	#if defined(BOOST_ASIO_WINDOWS) || defined(__CYGWIN__)
		WSASetLastError(0);
	#else
		errno = 0;
	#endif
	}

	template <typename ReturnType>
	ReturnType error_wrapper(ReturnType return_value,
	  boost::system::error_code& ec)
	{
	#if defined(BOOST_ASIO_WINDOWS) || defined(__CYGWIN__)
		ec = boost::system::error_code(WSAGetLastError(), boost::asio::error::get_system_category());
	#else
		ec = boost::system::error_code(errno, boost::asio::error::get_system_category());
	#endif
		return return_value;
	}

	int call_bind(socket_type s, const socket_addr_type* addr, std::size_t addrcnt)
	{
		return ::sctp_bindx(s, const_cast<socket_addr_type*>(addr), addrcnt, SCTP_BINDX_ADD_ADDR);
	}
	
	int bind(socket_type s, const socket_addr_type* addr,
		std::size_t addrcnt, boost::system::error_code& ec)
	{
		if (s == invalid_socket)
		{
			ec = boost::asio::error::bad_descriptor;
			return socket_error_retval;
		}

		clear_last_error();
		int result = error_wrapper(sctp_socket_ops::call_bind(s, addr, addrcnt), ec);
		if (result == 0)
			ec = boost::system::error_code();
		return result;
	}
	
	int call_getlAddr(socket_type s, socket_addr_type* addr, std::size_t* addrlen)
	{
		struct sockaddr *paddrs = 0;
		int n = sctp_getladdrs(s, 0, &paddrs);
		if (n > 0) {
			//std::cout << "call_getlAddr " << n<< std::endl;
			for (int i = 0; i < n; i++) {
				//boost::asio::ip::sctp::endpoint ep;
				*addrlen = std::min( *addrlen, (paddrs[i].sa_family == AF_INET6)?sizeof( struct sockaddr_in6 ): sizeof( struct sockaddr ) );
	            memcpy(addr, &(paddrs[i]), *addrlen );            
				//peerAddresses.insert( ep.address().to_string() );
			}
			sctp_freeladdrs(paddrs);
		}
		return n;
	}
	
	int getlAddr(socket_type s, socket_addr_type* addr,
		std::size_t* addrlen, boost::system::error_code& ec)
	{
		if (s == invalid_socket)
		{
			ec = boost::asio::error::bad_descriptor;
			return socket_error_retval;
		}

		clear_last_error();
		int result = error_wrapper(sctp_socket_ops::call_getlAddr(s, addr, addrlen), ec);
		if (result < 0) ec = boost::system::error_code();
		return result;
	}

	int call_getpAddr(socket_type s, socket_addr_type* addr, std::size_t* addrlen)
	{
		struct sockaddr *paddrs = 0;
		int n = sctp_getpaddrs(s, 0,  &paddrs);
		if (n > 0) {
			//std::cout << "call_getpAddr " << n<< std::endl;
			for (int i = 0; i < n; i++) {
				//boost::asio::ip::sctp::endpoint ep;
				*addrlen = std::min( *addrlen, (paddrs[i].sa_family == AF_INET6)?sizeof( struct sockaddr_in6 ): sizeof( struct sockaddr ) );
	            memcpy(addr, &(paddrs[i]), *addrlen );      
				break;
				//peerAddresses.insert( ep.address().to_string() );
			}
			sctp_freepaddrs(paddrs);
		}
		
		return n;
	}
	
	int getpAddr(socket_type s, socket_addr_type* addr,
		std::size_t* addrlen, bool cached, boost::system::error_code& ec)
	{
		if (s == invalid_socket)
		{
			ec = boost::asio::error::bad_descriptor;
			return socket_error_retval;
		}
		
		clear_last_error();
		int result = error_wrapper(sctp_socket_ops::call_getpAddr(s, addr, addrlen), ec);
		if (result  > 0) ec = boost::system::error_code();
		return result;
	}

	
	int call_connect(socket_type s, const socket_addr_type* addr, std::size_t addrcnt)
	{
		sctp_assoc_t assoc_id = 0;
		int ret =  ::sctp_connectx(s, const_cast<socket_addr_type*>(addr), addrcnt, &assoc_id);
		std::cout << s << " call_connect() " << addrcnt << " return :" << ret << std::endl;
		return ret;
	}
	
	int connect(socket_type s, const socket_addr_type* addr,
	  std::size_t addrcnt, boost::system::error_code& ec)
	{
		if (s == invalid_socket)
		{
			ec = boost::asio::error::bad_descriptor;
			return socket_error_retval;
		}

		clear_last_error();
		int result = error_wrapper(sctp_socket_ops::call_connect(s, addr, addrcnt), ec);
		if (result == 0)
			ec = boost::system::error_code();
#if defined(__linux__)
		else if (ec == boost::asio::error::try_again)
			ec = boost::asio::error::no_buffer_space;
#endif // defined(__linux__)
		return result;
	}

	void sync_connect(socket_type s, const socket_addr_type* addr,
		std::size_t addrcnt, boost::system::error_code& ec)
	{
		// Perform the connect operation.
		sctp_socket_ops::connect(s, addr, addrcnt, ec);
		if (ec != boost::asio::error::in_progress && ec != boost::asio::error::would_block)
		{
			// The connect operation finished immediately.
			return;
		}

		// Wait for socket to become ready.
		if (socket_ops::poll_connect(s, -1, ec) < 0)
			return;

		// Get the error code from the connect operation.
		int connect_error = 0;
		size_t connect_error_len = sizeof(connect_error);
		if (socket_ops::getsockopt(s, 0, SOL_SOCKET, SO_ERROR, &connect_error, &connect_error_len, ec) == socket_error_retval)
		return;

		// Return the result of the connect operation.
		ec = boost::system::error_code(connect_error, boost::asio::error::get_system_category());
	}

	bool non_blocking_connect(socket_type s, boost::system::error_code& ec)
	{
		// Check if the connect operation has finished. This is required since we may
		// get spurious readiness notifications from the reactor.
		pollfd fds;
		fds.fd = s;
		fds.events = POLLOUT;
		fds.revents = 0;
		int ready = ::poll(&fds, 1, 0);

		if (ready == 0)
		{
			// The asynchronous connect operation is still in progress.
			return false;
		}

		// Get the error code from the connect operation.
		int connect_error = 0;
		size_t connect_error_len = sizeof(connect_error);
		if (socket_ops::getsockopt(s, 0, SOL_SOCKET, SO_ERROR,&connect_error, &connect_error_len, ec) == 0)
		{
			if (connect_error)
			{
				ec = boost::system::error_code(connect_error, boost::asio::error::get_system_category());
			}
			else
				ec = boost::system::error_code();
		}
		
		return true;
	}


	int call_sctp_sendmsg(boost::asio::detail::socket_type s, int ppid,
	  const void* data, size_t size, int flags)
	{
		struct sctp_sndrcvinfo sinfo;
		memset(&sinfo, 0, sizeof( struct sctp_sndrcvinfo) );
		sinfo.sinfo_ppid = htonl(ppid);
		sinfo.sinfo_flags = SCTP_UNORDERED; 								
		return ::sctp_send(s, data, size, &sinfo, 0);
	}
	
	signed_size_type send(socket_type s, int ppid, 
	  const void* data, size_t size, int flags,  boost::system::error_code& ec)
	{
		clear_last_error();
		int result = error_wrapper(sctp_socket_ops::call_sctp_sendmsg(s, ppid, data, size,flags), ec);
		if (result == 0)
			ec = boost::system::error_code();
		return result;
	}
	
	size_t sync_send(socket_type s, state_type state, int ppid, 
	  const void* data, size_t size,  int flags, bool all_empty, boost::system::error_code& ec)
	{
		if (s == invalid_socket)
		{
			ec = boost::asio::error::bad_descriptor;
			return 0;
		}

		// A request to write 0 bytes to a stream is a no-op.
		if (all_empty && (state & stream_oriented))
		{
			ec = boost::system::error_code();
			return 0;
		}

		return sctp_socket_ops::send(s, ppid, data, size, flags, ec);
	}

	bool non_blocking_send(socket_type s, int ppid,
	  const void* data, size_t size, int flags,
	  boost::system::error_code& ec, size_t& bytes_transferred)
	{
		bytes_transferred = sctp_socket_ops::send(s, ppid, data, size, flags, ec);
		return bytes_transferred == size;
	}
	
	int call_sctp_recv(boost::asio::detail::socket_type s,  void* data, size_t size,  uint16_t& stream_no, int &flags)
	{
		struct sockaddr_storage client_addr;
		socklen_t addr_len = sizeof(struct sockaddr_storage);
		struct sctp_sndrcvinfo sinfo;
		flags = 0;
		memset(&sinfo, 0, sizeof(struct sctp_sndrcvinfo));
		memset(&client_addr, 0, sizeof(struct sockaddr_storage));
		int n = sctp_recvmsg(s, data, size, (struct sockaddr*) &client_addr, &addr_len, &sinfo, &flags);
		stream_no = sinfo.sinfo_stream;
		return n;
	}
		
	int revc(boost::asio::detail::socket_type s, void* data, size_t size,
	  uint16_t& stream_no ,int& flags, boost::system::error_code& ec)
	{
		clear_last_error();
		int result = error_wrapper(sctp_socket_ops::call_sctp_recv(s, data, size, stream_no, flags), ec);
		if (result == 0)
			ec = boost::system::error_code();
		return result;
	}

	size_t sync_recv(boost::asio::detail::socket_type s, state_type state, void* data, size_t size,
	  uint16_t& stream_no, int& flags, boost::system::error_code& ec)
	{
		if (s == invalid_socket)
		{
			ec = boost::asio::error::bad_descriptor;
			return 0;
		}

		// A request to read 0 bytes on a stream is a no-op.
		if (size <= 0 && (state & stream_oriented))
		{
			ec = boost::system::error_code();
			return 0;
		}

		
		// Try to complete the operation without blocking.
		signed_size_type bytes = sctp_socket_ops::revc(s, data, size, stream_no, flags, ec);

		// Check if operation succeeded.
		if (bytes > 0) return bytes;

		// Check for EOF.
		if ((state & stream_oriented) && bytes == 0)
		{
			ec = boost::asio::error::eof;
			return 0;
		}

		return 0;	
	}

	bool non_blocking_recv(socket_type s,
		void* data, size_t size, bool is_stream, 
		uint16_t& stream_no, int& flags, boost::system::error_code& ec, size_t& bytes_transferred)
	{
		for (;;)
		{
			// Read some data.
			signed_size_type bytes = sctp_socket_ops::revc(s, data, size, stream_no, flags, ec);

			// Check for end of stream.
			if (is_stream && bytes == 0)
			{
				ec = boost::asio::error::eof;
				return true;
			}

			// Retry operation if interrupted by signal.
			if (ec == boost::asio::error::interrupted)
				continue;

			// Check if we need to run the operation again.
			if (ec == boost::asio::error::would_block || ec == boost::asio::error::try_again)
				return false;

			// Operation is complete.
			if (bytes >= 0)
			{
				ec = boost::system::error_code();
				bytes_transferred = bytes;
			}
			else
				bytes_transferred = 0;

			return true;
		}
	}
	
} // namespace sctp_socket_ops
} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>
