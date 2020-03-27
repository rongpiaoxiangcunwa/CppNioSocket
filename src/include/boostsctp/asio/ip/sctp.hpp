#ifndef BOOST_ASIO_IP_SCTP_HPP
#define BOOST_ASIO_IP_SCTP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif 

#include <boost/asio/basic_socket_acceptor.hpp>
#include <boost/asio/basic_socket_iostream.hpp>
#include <boost/asio/basic_stream_socket.hpp>
#include <boost/asio/ip/basic_endpoint.hpp>
#include <boost/asio/ip/basic_resolver.hpp>
#include <boost/asio/ip/basic_resolver_iterator.hpp>
#include <boost/asio/ip/basic_resolver_query.hpp>
#include <boost/asio/detail/socket_option.hpp>
#include <boost/asio/detail/socket_types.hpp>

#include <boostsctp/asio/sctp_basic_stream_socket.hpp>
#include <boostsctp/asio/sctp_basic_socket_acceptor.hpp>


namespace boost {
namespace asio {
namespace ip {

class sctp
{
public:
	/// The type of a SCTP endpoint.
	typedef basic_endpoint<sctp> endpoint;

	/// The type of a resolver query.
	typedef basic_resolver_query<sctp> resolver_query;

	/// The type of a resolver iterator.
	typedef basic_resolver_iterator<sctp> resolver_iterator;

	/// The SCTP resolver type.
	typedef basic_resolver<sctp> resolver;

	/// Construct to represent the IPv4 TCP protocol.
	static sctp v4()
	{
		return sctp(PF_INET);
	}

	/// Construct to represent the IPv6 TCP protocol.
	static sctp v6()
	{
		return sctp(PF_INET6);
	}

	/// Obtain an identifier for the type of the protocol.
	int type() const
	{
		return SOCK_STREAM;
	}

	/// Obtain an identifier for the protocol.
	int protocol() const
	{
		return IPPROTO_SCTP;
	}

	/// Obtain an identifier for the protocol family.
	int family() const
	{
		return family_;
	}

	/// The SCTP socket type.
	typedef sctp_basic_stream_socket<sctp> socket;

	/// The SCTP acceptor type.
	typedef sctp_basic_socket_acceptor<sctp> acceptor;

#if !defined(BOOST_NO_IOSTREAM)
	/// The SCTP iostream type.
	typedef basic_socket_iostream<sctp> iostream;
#endif // !defined(BOOST_NO_IOSTREAM)

	/// Socket option for disabling the Nagle algorithm.
	/**
	* Implements the IPPROTO_TCP/TCP_NODELAY socket option.
	*
	* @par Examples
	* Setting the option:
	* @code
	* boost::asio::ip::sctp::socket socket(io_service); 
	* ...
	* boost::asio::ip::sctp::no_delay option(true);
	* socket.set_option(option);
	* @endcode
	*
	* @par
	* Getting the current option value:
	* @code
	* boost::asio::ip::sctp::socket socket(io_service); 
	* ...
	* boost::asio::ip::sctp::no_delay option;
	* socket.get_option(option);
	* bool is_set = option.value();
	* @endcode
	*
	* @par Concepts:
	* Socket_Option, Boolean_Socket_Option.
	*/
	typedef boost::asio::detail::socket_option::boolean<IPPROTO_SCTP,  SCTP_NODELAY> no_delay;
	typedef boost::asio::detail::socket_option::integer<IPPROTO_SCTP,  SCTP_DEFAULT_SEND_PARAM> sctp_ppid;

	/// Compare two protocols for equality.
	friend bool operator==(const sctp& p1, const sctp& p2)
	{
		return p1.family_ == p2.family_;
	}

	/// Compare two protocols for inequality.
	friend bool operator!=(const sctp& p1, const sctp& p2)
	{
		return p1.family_ != p2.family_;
	}

private:
	// Construct with a specific family.
	explicit sctp(int family)
	: family_(family)
	{
	}

	int family_;
};
} // namespace ip
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif
