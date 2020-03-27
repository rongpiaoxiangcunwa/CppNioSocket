#ifndef BOOST_ASIO_DETAIL_SCTP_SOCKET_OPS_HPP
#define BOOST_ASIO_DETAIL_SCTP_SOCKET_OPS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>
#include <boost/config.hpp>
#include <boost/assert.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <boost/detail/workaround.hpp>
#include <new>
#include <boost/asio/detail/pop_options.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/detail/socket_types.hpp>
#include <boost/asio/detail/socket_ops.hpp>
#include <boostsctp/asio/detail/sctp_socket_types.hpp>

namespace boost {
namespace asio {
namespace detail {
namespace sctp_socket_ops {
	using namespace boost::asio::detail::socket_ops;

	int call_bind(socket_type s, const socket_addr_type* addr, std::size_t addrcnt);
	
	int bind(socket_type s, const socket_addr_type* addr,
		std::size_t addrcnt, boost::system::error_code& ec);
	
	int call_getlAddr(socket_type s, socket_addr_type* addr, std::size_t* addrlen);
	
	int getlAddr(socket_type s, socket_addr_type* addr,
		std::size_t* addrlen, boost::system::error_code& ec);

	int call_getpAddr(socket_type s, socket_addr_type* addr, std::size_t* addrlen);
	
	int getpAddr(socket_type s, socket_addr_type* addr,
		std::size_t* addrlen, bool cached, boost::system::error_code& ec);

	int call_connect(socket_type s, const socket_addr_type* addr, std::size_t addrcnt);
	
	int connect(socket_type s, const socket_addr_type* addr,
	  std::size_t addrcnt, boost::system::error_code& ec);

	void sync_connect(socket_type s, const socket_addr_type* addr,
		std::size_t addrcnt, boost::system::error_code& ec);
	
	bool non_blocking_connect(socket_type s, boost::system::error_code& ec);

	int call_sctp_sendmsg(boost::asio::detail::socket_type s, int ppid,
	  const void* data, size_t size,  int flags);
	
	signed_size_type send(socket_type s, int ppid, 
	  const void* data, size_t size, int flags,  boost::system::error_code& ec);
	
	size_t sync_send(socket_type s, state_type state, int ppid, 
	  const void* data, size_t size,  int flags, bool all_empty, boost::system::error_code& ec);

	bool non_blocking_send(socket_type s, int ppid,
	  const void* data, size_t size, int flags,
	  boost::system::error_code& ec, size_t& bytes_transferred);
	
	int call_sctp_recv(boost::asio::detail::socket_type s,  void* data, size_t size,  uint16_t& stream_no, int &flags);
		
	int revc(boost::asio::detail::socket_type s, void* data, size_t size,
	  uint16_t& stream_no ,int& flags, boost::system::error_code& ec);

	size_t sync_recv(boost::asio::detail::socket_type s, state_type state, void* data, size_t size,
	  uint16_t& stream_no, int& flags, boost::system::error_code& ec);

	bool non_blocking_recv(socket_type s,
		void* data, size_t size, bool is_stream, 
		uint16_t& stream_no, int& flags, boost::system::error_code& ec, size_t& bytes_transferred);
	
} // namespace sctp_socket_ops
} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif
