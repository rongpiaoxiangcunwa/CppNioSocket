#ifndef BOOST_ASIO_SCTP_BASIC_SOCKET_ACCEPTOR_HPP
#define BOOST_ASIO_SCTP_BASIC_SOCKET_ACCEPTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <boost/asio/basic_io_object.hpp>
#include <boost/asio/basic_socket.hpp>
#include <boost/asio/detail/handler_type_requirements.hpp>
#include <boost/asio/detail/throw_error.hpp>
#include <boost/asio/detail/type_traits.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/asio/basic_socket_acceptor.hpp>

#if defined(BOOST_ASIO_HAS_MOVE)
#include <utility>
#endif // defined(BOOST_ASIO_HAS_MOVE)

#include <boostsctp/asio/sctp_socket_acceptor_service.hpp>
#include <boostsctp/asio/detail/sctp_reactive_socket_service.hpp>
#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {

/// Provides the ability to accept new connections.
/**
 * The sctp_basic_socket_acceptor class template is used for accepting new socket
 * connections.
 * @par Example
 * Opening a socket acceptor with the SO_REUSEADDR option enabled:
 * @code
 * boost::asio::ip::sctp::acceptor acceptor(io_context);
 * boost::asio::ip::sctp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
 * acceptor.open(endpoint.protocol());
 * acceptor.set_option(boost::asio::ip::sctp::acceptor::reuse_address(true));
 * acceptor.bind(endpoint);
 * acceptor.listen();
 * @endcode
 */
template <typename Protocol BOOST_ASIO_SVC_TPARAM_DEF1(=sctp_socket_acceptor_service<Protocol>)>
class sctp_basic_socket_acceptor
  : public basic_socket_acceptor<Protocol BOOST_ASIO_SVC_TARG>
{
public:
	/// The type of the executor associated with the object.
	typedef io_context::executor_type executor_type;

	/// The native representation of an acceptor.
	typedef typename  basic_socket_acceptor<Protocol BOOST_ASIO_SVC_TARG>::native_handle_type native_handle_type;

	/// The protocol type.
	typedef Protocol protocol_type;

	/// The endpoint type.
	typedef typename Protocol::endpoint endpoint_type;

	/// Construct an acceptor without opening it.
	/**
	* This constructor creates an acceptor without opening it to listen for new
	* connections. The open() function must be called before the acceptor can
	* accept new socket connections.
	*
	* @param io_context The io_context object that the acceptor will use to
	* dispatch handlers for any asynchronous operations performed on the
	* acceptor.
	*/
	explicit sctp_basic_socket_acceptor(boost::asio::io_context& io_context)
	: basic_socket_acceptor<Protocol BOOST_ASIO_SVC_TARG>(io_context)
	{
	}

	/// Construct an open acceptor.
	/**
	* This constructor creates an acceptor and automatically opens it.
	*
	* @param io_context The io_context object that the acceptor will use to
	* dispatch handlers for any asynchronous operations performed on the
	* acceptor.
	*
	* @param protocol An object specifying protocol parameters to be used.
	*
	* @throws boost::system::system_error Thrown on failure.
	*/
	sctp_basic_socket_acceptor(boost::asio::io_context& io_context,
	  const protocol_type& protocol)
	: basic_socket_acceptor<Protocol BOOST_ASIO_SVC_TARG>(io_context, protocol)
	{
	}

	/// Construct an acceptor opened on the given endpoint.
	/**
	* This constructor creates an acceptor and automatically opens it to listen
	* for new connections on the specified endpoint.
	*
	* @param io_context The io_context object that the acceptor will use to
	* dispatch handlers for any asynchronous operations performed on the
	* acceptor.
	*
	* @param endpoint An endpoint on the local machine on which the acceptor
	* will listen for new connections.
	*
	* @param reuse_addr Whether the constructor should set the socket option
	* socket_base::reuse_address.
	*
	* @throws boost::system::system_error Thrown on failure.
	*
	* @note This constructor is equivalent to the following code:
	* @code
	* basic_socket_acceptor<Protocol> acceptor(io_context);
	* acceptor.open(endpoint.protocol());
	* if (reuse_addr)
	*   acceptor.set_option(socket_base::reuse_address(true));
	* acceptor.bind(endpoint);
	* acceptor.listen(listen_backlog);
	* @endcode
	*/
	sctp_basic_socket_acceptor(boost::asio::io_context& io_context,
	  const endpoint_type& endpoint, bool reuse_addr = true)
	  : basic_socket_acceptor<Protocol BOOST_ASIO_SVC_TARG>(io_context, endpoint, reuse_addr)
	{
	}

	sctp_basic_socket_acceptor(boost::asio::io_context& io_context,
	  const std::vector<endpoint_type>& endpoints, bool reuse_addr = true)
	  : basic_socket_acceptor<Protocol BOOST_ASIO_SVC_TARG>(io_context)
	{
		boost::system::error_code ec;
		const protocol_type protocol = endpoints.front().protocol();
		this->get_service().open(this->get_implementation(), protocol, ec);
		boost::asio::detail::throw_error(ec, "open");
		if (reuse_addr)
		{
			this->get_service().set_option(this->get_implementation(), socket_base::reuse_address(true), ec);
			boost::asio::detail::throw_error(ec, "set_option");
		}
		this->get_service().bind(this->get_implementation(), endpoints, ec);
		boost::asio::detail::throw_error(ec, "bind");
		this->get_service().listen(this->get_implementation(), 10000, ec);
		boost::asio::detail::throw_error(ec, "listen");
	}

	/// Construct a basic_socket_acceptor on an existing native acceptor.
	/**
	* This constructor creates an acceptor object to hold an existing native
	* acceptor.
	*
	* @param io_context The io_context object that the acceptor will use to
	* dispatch handlers for any asynchronous operations performed on the
	* acceptor.
	*
	* @param protocol An object specifying protocol parameters to be used.
	*
	* @param native_acceptor A native acceptor.
	*
	* @throws boost::system::system_error Thrown on failure.
	*/
	sctp_basic_socket_acceptor(boost::asio::io_context& io_context,
	  const protocol_type& protocol, const native_handle_type& native_acceptor)
	: basic_socket_acceptor<Protocol BOOST_ASIO_SVC_TARG>(io_context, protocol, native_acceptor)
	{
	}

#if defined(BOOST_ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  /// Move-construct a basic_socket_acceptor from another.
  /**
   * This constructor moves an acceptor from one object to another.
   *
   * @param other The other basic_socket_acceptor object from which the move
   * will occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c basic_socket_acceptor(io_context&) constructor.
   */
	sctp_basic_socket_acceptor(sctp_basic_socket_acceptor&& other)
	: basic_socket_acceptor<Protocol BOOST_ASIO_SVC_TARG>(std::move(other))
	{
	}

  /// Move-assign a basic_socket_acceptor from another.
  /**
   * This assignment operator moves an acceptor from one object to another.
   *
   * @param other The other basic_socket_acceptor object from which the move
   * will occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c basic_socket_acceptor(io_context&) constructor.
   */
	sctp_basic_socket_acceptor& operator=(sctp_basic_socket_acceptor&& other)
	{
		basic_socket_acceptor<Protocol BOOST_ASIO_SVC_TARG>::operator=(std::move(other));
		return *this;
	}

	// All socket acceptors have access to each other's implementations.
	template <typename Protocol1 BOOST_ASIO_SVC_TPARAM1>
	friend class sctp_basic_socket_acceptor;

  /// Move-construct a basic_socket_acceptor from an acceptor of another
  /// protocol type.
  /**
   * This constructor moves an acceptor from one object to another.
   *
   * @param other The other basic_socket_acceptor object from which the move
   * will occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c basic_socket(io_context&) constructor.
   */
  template <typename Protocol1 BOOST_ASIO_SVC_TPARAM1>
	sctp_basic_socket_acceptor(
	  sctp_basic_socket_acceptor<Protocol1 BOOST_ASIO_SVC_TARG1>&& other,
	  typename enable_if<is_convertible<Protocol1, Protocol>::value>::type* = 0)
	: basic_socket_acceptor<Protocol BOOST_ASIO_SVC_TARG>(std::move(other))
	{
	}

  /// Move-assign a basic_socket_acceptor from an acceptor of another protocol
  /// type.
  /**
   * This assignment operator moves an acceptor from one object to another.
   *
   * @param other The other basic_socket_acceptor object from which the move
   * will occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c basic_socket(io_context&) constructor.
   */
	template <typename Protocol1 BOOST_ASIO_SVC_TPARAM1>
	typename enable_if<is_convertible<Protocol1, Protocol>::value,sctp_basic_socket_acceptor>::type& 
	  operator=(sctp_basic_socket_acceptor<Protocol1 BOOST_ASIO_SVC_TARG1>&& other)
	{
		sctp_basic_socket_acceptor tmp(std::move(other));
		basic_socket_acceptor<Protocol BOOST_ASIO_SVC_TARG>::operator=(std::move(tmp));
		return *this;
	}
#endif // defined(BOOST_ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

	/// Destroys the acceptor.
	/**
	* This function destroys the acceptor, cancelling any outstanding
	* asynchronous operations associated with the acceptor as if by calling
	* @c cancel.
	*/
	~sctp_basic_socket_acceptor()
	{
	}

	void bind(std::vector<endpoint_type>& endpoints)
	{
		boost::system::error_code ec;
		this->get_service().bind(this->get_implementation(), endpoints, ec);
		boost::asio::detail::throw_error(ec, "bind");
	}
	

	BOOST_ASIO_SYNC_OP_VOID bind(const std::vector<endpoint_type>& endpoints,
	  boost::system::error_code& ec)
	{
		this->get_service().bind(this->get_implementation(), endpoints, ec);
		BOOST_ASIO_SYNC_OP_VOID_RETURN(ec);
	}
};

} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#if !defined(BOOST_ASIO_ENABLE_OLD_SERVICES)
#undef BOOST_ASIO_SVC_T
#endif // !defined(BOOST_ASIO_ENABLE_OLD_SERVICES)

#endif // BOOST_ASIO_BASIC_SOCKET_ACCEPTOR_HPP
