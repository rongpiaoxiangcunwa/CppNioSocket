#ifndef BOOST_ASIO_DETAIL_SCTP_REACTIVE_SOCKET_SERVICE_BASE_HPP
#define BOOST_ASIO_DETAIL_SCTP_REACTIVE_SOCKET_SERVICE_BASE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if !defined(BOOST_ASIO_HAS_IOCP)  && !defined(BOOST_ASIO_WINDOWS_RUNTIME)

#include <boost/asio/buffer.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/asio/detail/buffer_sequence_adapter.hpp>
#include <boost/asio/detail/memory.hpp>
#include <boost/asio/detail/reactive_null_buffers_op.hpp>
#include <boost/asio/detail/reactive_socket_recv_op.hpp>
#include <boost/asio/detail/reactive_socket_recvmsg_op.hpp>
#include <boost/asio/detail/reactive_socket_send_op.hpp>
#include <boost/asio/detail/reactive_wait_op.hpp>
#include <boost/asio/detail/reactor.hpp>
#include <boost/asio/detail/reactor_op.hpp>
#include <boost/asio/detail/socket_holder.hpp>
#include <boost/asio/detail/socket_ops.hpp>
#include <boost/asio/detail/socket_types.hpp>

#include <string>
#include <map>
#include <boostsctp/asio/detail/sctp_reactive_socket_send_op.hpp>
#include <boostsctp/asio/detail/sctp_reactive_socket_recv_op.hpp>
#include <boostsctp/asio/detail/sctp_reactive_socket_recv_op.hpp>
#include <boostsctp/asio/detail/sctp_reactive_socket_connect_op.hpp>
#include <boostsctp/asio/detail/sctp_socket_ops.hpp>
#include <boostsctp/asio/detail/sctp_socket_types.hpp>

#include <boost/thread/mutex.hpp>
#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace detail {

class sctp_reactive_socket_service_base
{
public:
	// The native type of a socket.
	typedef socket_type native_handle_type;

	// The implementation type of the socket.
	struct base_implementation_type
	{
		// The native socket representation.
		socket_type socket_;

		// The current state of the socket.
		socket_ops::state_type state_;

		// Per-descriptor data used by the reactor.
		reactor::per_descriptor_data reactor_data_;
		int ppid_;
	};

	// Constructor.
	sctp_reactive_socket_service_base(boost::asio::io_context& io_context) :  io_context_(io_context),
    	reactor_(use_service<reactor>(io_context))
	{
		reactor_.init_task();
	}

	// Destroy all user-defined handler objects owned by the service.
	void base_shutdown() 
	{
	}

	// Construct a new socket implementation.
	void construct(base_implementation_type& impl)
	{
		impl.socket_ = invalid_socket;
		impl.state_ = 0;
		impl.ppid_ = 0;
	}

	// Move-construct a new socket implementation.
	void base_move_construct(base_implementation_type& impl,
		base_implementation_type& other_impl)
	{
		impl.socket_ = other_impl.socket_;
		other_impl.socket_ = invalid_socket;

		impl.state_ = other_impl.state_;
		other_impl.state_ = 0;

		impl.ppid_ = other_impl.ppid_ ;
		other_impl.ppid_  = 0;

		reactor_.move_descriptor(impl.socket_, impl.reactor_data_, other_impl.reactor_data_);
	}

	// Move-assign from another socket implementation.
	void base_move_assign(base_implementation_type& impl,
		sctp_reactive_socket_service_base& other_service,
		base_implementation_type& other_impl)
	{
		destroy(impl);

		impl.socket_ = other_impl.socket_;
		other_impl.socket_ = invalid_socket;

		impl.state_ = other_impl.state_;
		other_impl.state_ = 0;

		impl.ppid_ = other_impl.ppid_;
		other_impl.ppid_ = 0;
		
		other_service.reactor_.move_descriptor(impl.socket_,
			impl.reactor_data_, other_impl.reactor_data_);
	}

	// Destroy a socket implementation.
	void destroy(base_implementation_type& impl)
	{
		if (impl.socket_ != invalid_socket)
		{
			BOOST_ASIO_HANDLER_OPERATION((reactor_.context(), "socket", &impl, impl.socket_, "close"));

			reactor_.deregister_descriptor(impl.socket_, impl.reactor_data_,
			(impl.state_ & socket_ops::possible_dup) == 0);

			boost::system::error_code ignored_ec;
			socket_ops::close(impl.socket_, impl.state_, true, ignored_ec);
			reactor_.cleanup_descriptor_data(impl.reactor_data_);
		}
	}
	

	// Determine whether the socket is open.
	bool is_open(const base_implementation_type& impl) const
	{
		return impl.socket_ != invalid_socket;
	}

	// Destroy a socket implementation.
	boost::system::error_code close(base_implementation_type& impl, boost::system::error_code& ec)
	{
		if (is_open(impl)) {
			BOOST_ASIO_HANDLER_OPERATION((reactor_.context(), "socket", &impl, impl.socket_, "close"));

			reactor_.deregister_descriptor(impl.socket_, impl.reactor_data_,
			(impl.state_ & socket_ops::possible_dup) == 0);

			socket_ops::close(impl.socket_, impl.state_, false, ec);

			reactor_.cleanup_descriptor_data(impl.reactor_data_);
		} else {
			ec = boost::system::error_code();
		}
		construct(impl);
		return ec;
	}
	
	// Release ownership of the socket.
	socket_type release(base_implementation_type& impl, boost::system::error_code& ec)
	{
		if (!is_open(impl))
		{
			ec = boost::asio::error::bad_descriptor;
			return invalid_socket;
		}

		BOOST_ASIO_HANDLER_OPERATION((reactor_.context(), "socket", &impl, impl.socket_, "release"));

		reactor_.deregister_descriptor(impl.socket_, impl.reactor_data_, false);
		reactor_.cleanup_descriptor_data(impl.reactor_data_);
		socket_type sock = impl.socket_;
		construct(impl);
		ec = boost::system::error_code();
		return sock;
	}

	// Get the native socket representation.
	native_handle_type native_handle(base_implementation_type& impl)
	{
		return impl.socket_;
	}

 	 // Cancel all operations associated with the socket.
  	boost::system::error_code cancel(base_implementation_type& impl, boost::system::error_code& ec)
  	{
		if (!is_open(impl))
		{
			ec = boost::asio::error::bad_descriptor;
			return ec;
		}

		BOOST_ASIO_HANDLER_OPERATION((reactor_.context(), "socket", &impl, impl.socket_, "cancel"));
		reactor_.cancel_ops(impl.socket_, impl.reactor_data_);
		ec = boost::system::error_code();
		return ec;
  	}

	// Determine whether the socket is at the out-of-band data mark.
	bool at_mark(const base_implementation_type& impl,
		boost::system::error_code& ec) const
	{
		return socket_ops::sockatmark(impl.socket_, ec);
	}

	// Determine the number of bytes available for reading.
	std::size_t available(const base_implementation_type& impl,
		boost::system::error_code& ec) const
	{
		return socket_ops::available(impl.socket_, ec);
	}

	// Place the socket into the state where it will listen for new connections.
	boost::system::error_code listen(base_implementation_type& impl,
	int backlog, boost::system::error_code& ec)
	{
		socket_ops::listen(impl.socket_, backlog, ec);
		return ec;
	}

	// Perform an IO control command on the socket.
	template <typename IO_Control_Command>
	boost::system::error_code io_control(base_implementation_type& impl,
		IO_Control_Command& command, boost::system::error_code& ec)
	{
		socket_ops::ioctl(impl.socket_, impl.state_, command.name(),
			static_cast<ioctl_arg_type*>(command.data()), ec);
		return ec;
	}

	// Gets the non-blocking mode of the socket.
	bool non_blocking(const base_implementation_type& impl) const
	{
		return (impl.state_ & socket_ops::user_set_non_blocking) != 0;
	}

	// Sets the non-blocking mode of the socket.
	boost::system::error_code non_blocking(base_implementation_type& impl,
		bool mode, boost::system::error_code& ec)
	{
		socket_ops::set_user_non_blocking(impl.socket_, impl.state_, mode, ec);
		return ec;
	}

	// Gets the non-blocking mode of the native socket implementation.
	bool native_non_blocking(const base_implementation_type& impl) const
	{
		return (impl.state_ & socket_ops::internal_non_blocking) != 0;
	}

	// Sets the non-blocking mode of the native socket implementation.
	boost::system::error_code native_non_blocking(base_implementation_type& impl,
	bool mode, boost::system::error_code& ec)
	{
		socket_ops::set_internal_non_blocking(impl.socket_, impl.state_, mode, ec);
		return ec;
	}

	// Wait for the socket to become ready to read, ready to write, or to have
	// pending error conditions.
	boost::system::error_code wait(base_implementation_type& impl,
		socket_base::wait_type w, boost::system::error_code& ec)
	{
		switch (w)
		{
			case socket_base::wait_read:
				socket_ops::poll_read(impl.socket_, impl.state_, -1, ec);
			break;
			case socket_base::wait_write:
				socket_ops::poll_write(impl.socket_, impl.state_, -1, ec);
			break;
			case socket_base::wait_error:
				socket_ops::poll_error(impl.socket_, impl.state_, -1, ec);
			break;
			default:
				ec = boost::asio::error::invalid_argument;
			break;
		}

		return ec;
	}

	// Asynchronously wait for the socket to become ready to read, ready to
	// write, or to have pending error conditions.
	template <typename Handler>
	void async_wait(base_implementation_type& impl,
		socket_base::wait_type w, Handler& handler)
	{
		bool is_continuation =boost_asio_handler_cont_helpers::is_continuation(handler);

		// Allocate and construct an operation to wrap the handler.
		typedef reactive_wait_op<Handler> op;
		typename op::ptr p = { boost::asio::detail::addressof(handler), op::ptr::allocate(handler), 0 };
		p.p = new (p.v) op(handler);

		BOOST_ASIO_HANDLER_CREATION((reactor_.context(), *p.p, "socket", &impl, impl.socket_, "async_wait"));

		int op_type;
		switch (w)
		{
			case socket_base::wait_read:
				op_type = reactor::read_op;
			break;
			case socket_base::wait_write:
				op_type = reactor::write_op;
			break;
			case socket_base::wait_error:
				op_type = reactor::except_op;
			break;
			default:
				p.p->ec_ = boost::asio::error::invalid_argument;
				reactor_.post_immediate_completion(p.p, is_continuation);
				p.v = p.p = 0;
			return;
		}

		start_op(impl, op_type, p.p, is_continuation, false, false);
		p.v = p.p = 0;
	}

	// Send the given data to the peer.
	template <typename ConstBufferSequence>
	size_t send(base_implementation_type& impl,
		const ConstBufferSequence& buffers,
		socket_base::message_flags flags, boost::system::error_code& ec)
	{
		return sctp_socket_ops::sync_send(impl.socket_, impl.state_, impl.ppid_,
			buffers.data(), buffers.size(), flags, buffers.size() <= 0, ec);
	}

	// Wait until data can be sent without blocking.
	size_t send(base_implementation_type& impl, const null_buffers&,
		socket_base::message_flags, boost::system::error_code& ec)
	{
		// Wait for socket to become ready.
		socket_ops::poll_write(impl.socket_, impl.state_, -1, ec);
		return 0;
	}

	// Start an asynchronous send. The data being sent must be valid for the
	// lifetime of the asynchronous operation.
	template <typename ConstBufferSequence, typename Handler>
	void async_send(base_implementation_type& impl,
		const ConstBufferSequence& buffers,
		socket_base::message_flags flags, Handler& handler)
	{
		bool is_continuation =boost_asio_handler_cont_helpers::is_continuation(handler);

		// Allocate and construct an operation to wrap the handler.
		typedef sctp_reactive_socket_send_op<ConstBufferSequence, Handler> op;
		typename op::ptr p = { boost::asio::detail::addressof(handler),  op::ptr::allocate(handler), 0 };
		p.p = new (p.v) op(impl.socket_, impl.ppid_,  impl.state_, buffers, flags, handler);

		BOOST_ASIO_HANDLER_CREATION((reactor_.context(), *p.p, "socket", &impl, impl.socket_, "async_send"));

		start_op(impl, reactor::write_op, p.p, is_continuation, true,
				  ((impl.state_ & socket_ops::stream_oriented)  
				   && buffer_sequence_adapter<boost::asio::const_buffer,ConstBufferSequence>::all_empty(buffers)));
		p.v = p.p = 0;
	}

	// Start an asynchronous wait until data can be sent without blocking.
	template <typename Handler>
	void async_send(base_implementation_type& impl, const null_buffers&,
		socket_base::message_flags, Handler& handler)
	{
		bool is_continuation = boost_asio_handler_cont_helpers::is_continuation(handler);

		// Allocate and construct an operation to wrap the handler.
		typedef reactive_null_buffers_op<Handler> op;
		typename op::ptr p = { boost::asio::detail::addressof(handler), op::ptr::allocate(handler), 0 };
		p.p = new (p.v) op(handler);

		BOOST_ASIO_HANDLER_CREATION((reactor_.context(), *p.p, "socket",&impl, impl.socket_, "async_send(null_buffers)"));
		start_op(impl, reactor::write_op, p.p, is_continuation, false, false);
		p.v = p.p = 0;
	}

	// Receive some data from the peer. Returns the number of bytes received.
	template <typename MutableBufferSequence>
	size_t receive(base_implementation_type& impl,
		const MutableBufferSequence& buffers,
		socket_base::message_flags, boost::system::error_code& ec)
	{
		int flags = 0;
		uint16_t stream_no = 0;
		char buffer[8192] = {0};
		for (; ;) {
			size_t n = sctp_socket_ops::sync_recv(impl.socket_, impl.state_, (void*)buffer, sizeof(buffer), stream_no, flags, ec);
			if ( n <= 0 || ec || (flags & MSG_EOR) ) {
				return mergeMessage(stream_no, buffer, n, buffers.data(), buffers.size());
			} 
			appendMessage(stream_no, (void*)buffer,n);
		}
	}

	// Wait until data can be received without blocking.
	size_t receive(base_implementation_type& impl, const null_buffers&,
		socket_base::message_flags, boost::system::error_code& ec)
	{
		// Wait for socket to become ready.
		socket_ops::poll_read(impl.socket_, impl.state_, -1, ec);
		return 0;
	}


	// Start an asynchronous receive. The buffer for the data being received
	// must be valid for the lifetime of the asynchronous operation.
	template <typename MutableBufferSequence, typename Handler>
	void async_receive(base_implementation_type& impl,
	  const MutableBufferSequence& buffers,
	  socket_base::message_flags flags, Handler& handler)
	{
		bool is_continuation = boost_asio_handler_cont_helpers::is_continuation(handler);

		// Allocate and construct an operation to wrap the handler.
		typedef sctp_reactive_socket_recv_op<MutableBufferSequence, Handler> op;
		typename op::ptr p = { boost::asio::detail::addressof(handler), op::ptr::allocate(handler), 0 };
		p.p = new (p.v) op(impl.socket_, impl.state_, buffers, flags, handler);

		BOOST_ASIO_HANDLER_CREATION((reactor_.context(), *p.p, "socket", &impl, impl.socket_, "async_receive"));

		 start_op(impl, reactor::read_op, p.p, is_continuation, true,
		        ((impl.state_ & socket_ops::stream_oriented)
		          && buffer_sequence_adapter<boost::asio::mutable_buffer,
		            MutableBufferSequence>::all_empty(buffers)));
		p.v = p.p = 0;
	}

	// Wait until data can be received without blocking.
	template <typename Handler>
	void async_receive(base_implementation_type& impl, const null_buffers&,
	  socket_base::message_flags flags, Handler& handler)
	{
		bool is_continuation = boost_asio_handler_cont_helpers::is_continuation(handler);

		// Allocate and construct an operation to wrap the handler.
		typedef reactive_null_buffers_op<Handler> op;
		typename op::ptr p = { boost::asio::detail::addressof(handler), op::ptr::allocate(handler), 0 };
		p.p = new (p.v) op(handler);

		BOOST_ASIO_HANDLER_CREATION((reactor_.context(), *p.p, "socket", &impl, impl.socket_, "async_receive(null_buffers)"));

		start_op(impl, reactor::read_op, p.p, is_continuation, false, false);
		p.v = p.p = 0;
	}
  
protected:
	size_t mergeMessage(uint16_t stream_no,  void *recvBuf, size_t n, void* dst, size_t size) {
		std::string tmp;
		getAndRemoveMessage(stream_no, tmp);
		size_t len = std::min(size, tmp.length());
		if (len > 0) memcpy(dst, tmp.c_str(),  len);
		if (len < size && n > 0) {
			size_t len1 = std::min(size - len, n);
			memcpy(static_cast<void*>(static_cast<char*>(dst) + len), recvBuf, len1);	
			len += len1;
		}
		return len;
	}
	
	void appendMessage(uint16_t stream_no, const void* data, size_t size) {
		boost::mutex::scoped_lock guard(mutex_);
		streamMsgs_[ stream_no ].append(static_cast<const char*>(data),  size);   
	}

	void getAndRemoveMessage(uint16_t stream_no, std::string &dst) {
		boost::mutex::scoped_lock guard(mutex_);
		#if 0
		std::map<uint16_t, std::string>::const_iterator iter =  streamMsgs_.find(stream_no);
		if (iter != streamMsgs_.end()) {
			dst = iter->second;
			streamMsgs_.erase(iter);
		}
		#endif
	}
	
	// Open a new socket implementation.
	BOOST_ASIO_DECL boost::system::error_code do_open(base_implementation_type& impl, int af,
	  int type, int protocol, boost::system::error_code& ec) 
	{
		if (is_open(impl))
		{
			ec = boost::asio::error::already_open;
			return ec;
		}

		socket_holder sock(socket_ops::socket(af, type, protocol, ec));
		if (sock.get() == invalid_socket)
			return ec;

		if (int err = reactor_.register_descriptor(sock.get(), impl.reactor_data_))
		{
			ec = boost::system::error_code(err, boost::asio::error::get_system_category());
			return ec;
		}

		impl.socket_ = sock.release();
		switch (type)
		{
			case SOCK_STREAM: impl.state_ = socket_ops::stream_oriented; break;
			case SOCK_DGRAM: impl.state_ = socket_ops::datagram_oriented; break;
			default: impl.state_ = 0; break;
		}
		ec = boost::system::error_code();
		return ec;
	}

	// Assign a native socket to a socket implementation.
	BOOST_ASIO_DECL boost::system::error_code do_assign(base_implementation_type& impl, int type,
	  const native_handle_type& native_socket, boost::system::error_code& ec)
	{
		if (is_open(impl))
		{
			ec = boost::asio::error::already_open;
			return ec;
		}

		if (int err = reactor_.register_descriptor(native_socket, impl.reactor_data_))
		{
			ec = boost::system::error_code(err,
			boost::asio::error::get_system_category());
			return ec;
		}

		impl.socket_ = native_socket;
		switch (type)
		{
			case SOCK_STREAM: impl.state_ = socket_ops::stream_oriented; break;
			case SOCK_DGRAM: impl.state_ = socket_ops::datagram_oriented; break;
			default: impl.state_ = 0; break;
		}
		impl.state_ |= socket_ops::possible_dup;
		ec = boost::system::error_code();
		return ec;
	}

	// Start the asynchronous read or write operation.
	BOOST_ASIO_DECL void start_op(base_implementation_type& impl, int op_type,
		reactor_op* op, bool is_continuation, bool is_non_blocking, bool noop)
	{
		if (!noop)
		{
			if ((impl.state_ & socket_ops::non_blocking)
				|| socket_ops::set_internal_non_blocking(impl.socket_, impl.state_, true, op->ec_))
			{
				reactor_.start_op(op_type, impl.socket_, impl.reactor_data_, op, is_continuation, is_non_blocking);
				return;
			}
		}

		reactor_.post_immediate_completion(op, is_continuation);
	}

	// Start the asynchronous accept operation.
	BOOST_ASIO_DECL void start_accept_op(base_implementation_type& impl,
		reactor_op* op, bool is_continuation, bool peer_is_open)
	{
		if (!peer_is_open) {
			start_op(impl, reactor::read_op, op, is_continuation, true, false);
		} else {
			op->ec_ = boost::asio::error::already_open;
			reactor_.post_immediate_completion(op, is_continuation);
		}
	}

	// Start the asynchronous connect operation.
	BOOST_ASIO_DECL void start_connect_op(base_implementation_type& impl,
	  reactor_op* op, bool is_continuation, const socket_addr_type* addr, int addrcnt)
	{
		if (addrcnt <= 0) {
			op->ec_ = boost::asio::error::invalid_argument;
		} else if ((impl.state_ & socket_ops::non_blocking)
				    || socket_ops::set_internal_non_blocking(impl.socket_, impl.state_, true, op->ec_)) {
			if (sctp_socket_ops::connect(impl.socket_, addr, addrcnt, op->ec_) != 0)
			{
				if (op->ec_ == boost::asio::error::in_progress
					|| op->ec_ == boost::asio::error::would_block)
				{
					op->ec_ = boost::system::error_code();
					reactor_.start_op(reactor::connect_op, impl.socket_, impl.reactor_data_, op, is_continuation, false);
					return;
				}
			}
		}

		reactor_.post_immediate_completion(op, is_continuation);
	}

	
protected:
	// The io_context that owns this socket service.
	io_context& io_context_;
	
	// The selector that performs event demultiplexing for the service.
	reactor& reactor_;

	boost::mutex mutex_;
	std::map<uint16_t, std::string> streamMsgs_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // !defined(BOOST_ASIO_HAS_IOCP)
       //   && !defined(BOOST_ASIO_WINDOWS_RUNTIME)
#endif // BOOST_ASIO_DETAIL_SCTP_REACTIVE_SOCKET_SERVICE_BASE_HPP
