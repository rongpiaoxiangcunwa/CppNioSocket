#ifndef BOOST_ASIO_SCTP_BASIC_STREAM_SOCKET_HPP
#define BOOST_ASIO_SCTP_BASIC_STREAM_SOCKET_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <cstddef>
#include <vector>
#include <boost/asio/async_result.hpp>
#include <boost/asio/basic_socket.hpp>
#include <boost/asio/detail/handler_type_requirements.hpp>
#include <boost/asio/detail/throw_error.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/basic_stream_socket.hpp>
#include <boostsctp/asio/sctp_stream_socket_service.hpp>

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
	
template <typename Protocol 
	BOOST_ASIO_SVC_TPARAM_DEF1(=sctp_stream_socket_service<Protocol>)>
class sctp_basic_stream_socket
  : public boost::asio::basic_stream_socket<Protocol BOOST_ASIO_SVC_TARG>
{
public:
	typedef typename basic_stream_socket<Protocol BOOST_ASIO_SVC_TARG>::native_handle_type native_handle_type;

	/// The protocol type.
	typedef Protocol protocol_type;

	/// The endpoint type.
	typedef typename Protocol::endpoint endpoint_type;

  /// Construct a basic_stream_socket without opening it.
  /**
   * This constructor creates a stream socket without opening it. The socket
   * needs to be opened and then connected or accepted before data can be sent
   * or received on it.
   *
   * @param io_context The io_context object that the stream socket will use to
   * dispatch handlers for any asynchronous operations performed on the socket.
   */
  explicit sctp_basic_stream_socket(boost::asio::io_context& io_context)
    : basic_stream_socket<Protocol BOOST_ASIO_SVC_TARG>(io_context)
  {
  }

  /// Construct and open a basic_stream_socket.
  /**
   * This constructor creates and opens a stream socket. The socket needs to be
   * connected or accepted before data can be sent or received on it.
   *
   * @param io_context The io_context object that the stream socket will use to
   * dispatch handlers for any asynchronous operations performed on the socket.
   *
   * @param protocol An object specifying protocol parameters to be used.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  sctp_basic_stream_socket(boost::asio::io_context& io_context, const protocol_type& protocol)
    : basic_stream_socket<Protocol BOOST_ASIO_SVC_TARG>(io_context, protocol)
  {
  }

  /// Construct a basic_stream_socket, opening it and binding it to the given
  /// local endpoint.
  /**
   * This constructor creates a stream socket and automatically opens it bound
   * to the specified endpoint on the local machine. The protocol used is the
   * protocol associated with the given endpoint.
   *
   * @param io_context The io_context object that the stream socket will use to
   * dispatch handlers for any asynchronous operations performed on the socket.
   *
   * @param endpoint An endpoint on the local machine to which the stream
   * socket will be bound.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  sctp_basic_stream_socket(boost::asio::io_context& io_context, const endpoint_type& endpoint)
    : basic_stream_socket<Protocol BOOST_ASIO_SVC_TARG>(io_context, endpoint)
  {
  }

  /// Construct a basic_stream_socket on an existing native socket.
  /**
   * This constructor creates a stream socket object to hold an existing native
   * socket.
   *
   * @param io_context The io_context object that the stream socket will use to
   * dispatch handlers for any asynchronous operations performed on the socket.
   *
   * @param protocol An object specifying protocol parameters to be used.
   *
   * @param native_socket The new underlying socket implementation.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  sctp_basic_stream_socket(boost::asio::io_context& io_context,
      const protocol_type& protocol, const native_handle_type& native_socket)
    : basic_stream_socket<Protocol BOOST_ASIO_SVC_TARG>(io_context, protocol, native_socket)
  {
  }

#if defined(BOOST_ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  /// Move-construct a basic_stream_socket from another.
  /**
   * This constructor moves a stream socket from one object to another.
   *
   * @param other The other basic_stream_socket object from which the move
   * will occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c basic_stream_socket(io_context&) constructor.
   */
  sctp_basic_stream_socket(sctp_basic_stream_socket&& other)
    : basic_stream_socket<Protocol BOOST_ASIO_SVC_TARG>(std::move(other))
  {
  }

  /// Move-assign a basic_stream_socket from another.
  /**
   * This assignment operator moves a stream socket from one object to another.
   *
   * @param other The other basic_stream_socket object from which the move
   * will occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c basic_stream_socket(io_context&) constructor.
   */
  sctp_basic_stream_socket& operator=(sctp_basic_stream_socket&& other)
  {
	basic_stream_socket<Protocol BOOST_ASIO_SVC_TARG>::operator=(std::move(other));
	return *this;
  }

  /// Move-construct a basic_stream_socket from a socket of another protocol
  /// type.
  /**
   * This constructor moves a stream socket from one object to another.
   *
   * @param other The other basic_stream_socket object from which the move
   * will occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c basic_stream_socket(io_context&) constructor.
   */
  template <typename Protocol1 BOOST_ASIO_SVC_TPARAM1>
  sctp_basic_stream_socket(sctp_basic_stream_socket<Protocol1 BOOST_ASIO_SVC_TARG1>&& other,
    typename enable_if<is_convertible<Protocol1, Protocol>::value>::type* = 0)
    : basic_stream_socket<Protocol BOOST_ASIO_SVC_TARG>(std::move(other))
  {
  }

  /// Move-assign a basic_stream_socket from a socket of another protocol type.
  /**
   * This assignment operator moves a stream socket from one object to another.
   *
   * @param other The other basic_stream_socket object from which the move
   * will occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c basic_stream_socket(io_context&) constructor.
   */
  template <typename Protocol1 BOOST_ASIO_SVC_TPARAM1>
  typename enable_if<is_convertible<Protocol1, Protocol>::value, sctp_basic_stream_socket>::type& 
    operator=(sctp_basic_stream_socket<Protocol1 BOOST_ASIO_SVC_TARG1>&& other)
  {
		basic_stream_socket<Protocol BOOST_ASIO_SVC_TARG>::operator=(std::move(other));
		return *this;
  }
#endif // defined(BOOST_ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

  /// Destroys the socket.
  /**
   * This function destroys the socket, cancelling any outstanding asynchronous
   * operations associated with the socket as if by calling @c cancel.
   */
  ~sctp_basic_stream_socket()
  {
  }

	void connect(const std::vector<endpoint_type>& peer_endpoints)
	{
		boost::system::error_code ec;
		if (!this->is_open())
		{
			this->get_service().open(this->get_implementation(), peer_endpoints.front().protocol(), ec);
			boost::asio::detail::throw_error(ec, "connect");
		}
		this->get_service().connect(this->get_implementation(), peer_endpoints, ec);
		boost::asio::detail::throw_error(ec, "connect");
	}

	BOOST_ASIO_SYNC_OP_VOID connect(const std::vector<endpoint_type>& peer_endpoints,
		boost::system::error_code& ec)
	{
		if (!this->is_open())
		{
			this->get_service().open(this->get_implementation(),
			  peer_endpoints.front().protocol(), ec);
			if (ec)
			{
				BOOST_ASIO_SYNC_OP_VOID_RETURN(ec);
			}
		}

		this->get_service().connect(this->get_implementation(), peer_endpoints, ec);
		BOOST_ASIO_SYNC_OP_VOID_RETURN(ec);
	}

	
	BOOST_ASIO_SYNC_OP_VOID connect(const endpoint_type& peer_endpoints,
		boost::system::error_code& ec)
	{
		if (!this->is_open())
		{
			this->get_service().open(this->get_implementation(),
			  peer_endpoints.protocol(), ec);
			if (ec)
			{
				BOOST_ASIO_SYNC_OP_VOID_RETURN(ec);
			}
		}

		this->get_service().connect(this->get_implementation(), peer_endpoints, ec);
		BOOST_ASIO_SYNC_OP_VOID_RETURN(ec);
	}
	

	template <typename ConnectHandler>
	BOOST_ASIO_INITFN_RESULT_TYPE(ConnectHandler, void (boost::system::error_code))
	async_connect(const std::vector<endpoint_type>& peer_endpoints,
	  BOOST_ASIO_MOVE_ARG(ConnectHandler) handler)
	{
		// If you get an error on the following line it means that your handler does
		// not meet the documented type requirements for a ConnectHandler.
		BOOST_ASIO_CONNECT_HANDLER_CHECK(ConnectHandler, handler) type_check;

		if (!this->is_open())
		{
			boost::system::error_code ec;
			const protocol_type protocol = peer_endpoints.front().protocol();
			  this->get_service().open(this->get_implementation(), protocol, ec);
			if (ec)
			{
				async_completion<ConnectHandler,void (boost::system::error_code)> init(handler);

				 boost::asio::post(this->get_executor(),
            		boost::asio::detail::bind_handler(
              			BOOST_ASIO_MOVE_CAST(BOOST_ASIO_HANDLER_TYPE(
                			ConnectHandler, void (boost::system::error_code)))(
                  				init.completion_handler), ec));

				return init.result.get();
			}
		}

#if defined(BOOST_ASIO_ENABLE_OLD_SERVICES)
	return this->get_service().async_connect(this->get_implementation(),
		peer_endpoints, BOOST_ASIO_MOVE_CAST(ConnectHandler)(handler));
#else // defined(BOOST_ASIO_ENABLE_OLD_SERVICES)
	async_completion<ConnectHandler,void(boost::system::error_code)> init(handler);

	this->get_service().async_connect(this->get_implementation(), peer_endpoints, init.completion_handler);

	return init.result.get();
#endif // defined(BOOST_ASIO_ENABLE_OLD_SERVICES)
	}
	
};

} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif //BOOST_ASIO_SCTP_BASIC_STREAM_SOCKET_HPP
