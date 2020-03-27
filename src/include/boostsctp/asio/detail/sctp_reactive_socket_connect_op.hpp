#ifndef BOOST_ASIO_DETAIL_SCTP_REACTIVE_SOCKET_CONNECT_OP_HPP
#define BOOST_ASIO_DETAIL_SCTP_REACTIVE_SOCKET_CONNECT_OP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <iostream>
#include <boost/asio/detail/config.hpp>
#include <boost/asio/detail/bind_handler.hpp>
#include <boost/asio/detail/buffer_sequence_adapter.hpp>
#include <boost/asio/detail/fenced_block.hpp>
#include <boost/asio/detail/memory.hpp>
#include <boost/asio/detail/reactor_op.hpp>
#include <boost/asio/detail/socket_ops.hpp>
#include <boostsctp/asio/detail/sctp_socket_ops.hpp>
#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace detail {

class sctp_reactive_socket_connect_op_base : public reactor_op
{
public:
	sctp_reactive_socket_connect_op_base(socket_type socket, func_type complete_func, 
	  const socket_addr_type* addr, int addrcnt, bool free_addr)
	: reactor_op(&sctp_reactive_socket_connect_op_base::do_perform, complete_func),
	  socket_(socket), addr_(addr), addrcnt_(addrcnt), free_addr_(free_addr)
	{
	}

	virtual ~sctp_reactive_socket_connect_op_base() {
		if (free_addr_) {
			std::cout << "sctp_reactive_socket_connect_op_base::~sctp_reactive_socket_connect_op_base()" << std::endl;
		//	free((void*)addr_);
			addr_ = 0;
		}
	}

	static status do_perform(reactor_op* base)
	{
		sctp_reactive_socket_connect_op_base* o(static_cast<sctp_reactive_socket_connect_op_base*>(base));
		status result = sctp_socket_ops::non_blocking_connect(o->socket_, o->ec_) ? done : not_done;
		BOOST_ASIO_HANDLER_REACTOR_OPERATION((*o, "non_blocking_connect", o->ec_));
		return result;
	}

private:
	socket_type socket_;
	const socket_addr_type* addr_;
	int addrcnt_;
	volatile bool free_addr_;
};

template <typename Handler>
class sctp_reactive_socket_connect_op : public sctp_reactive_socket_connect_op_base
{
public:
	BOOST_ASIO_DEFINE_HANDLER_PTR(sctp_reactive_socket_connect_op);

	sctp_reactive_socket_connect_op(socket_type socket, Handler& handler, const socket_addr_type* addr, int addrcnt, bool free_addr)
	: sctp_reactive_socket_connect_op_base(socket, &sctp_reactive_socket_connect_op::do_complete, addr, addrcnt, free_addr),
	  handler_(BOOST_ASIO_MOVE_CAST(Handler)(handler))
	{
		handler_work<Handler>::start(handler_);
	}

	static void do_complete(void* owner, operation* base,
	  const boost::system::error_code& /*ec*/,
	  std::size_t /*bytes_transferred*/)
	{
		// Take ownership of the handler object.
		sctp_reactive_socket_connect_op* o(static_cast<sctp_reactive_socket_connect_op*>(base));
		ptr p = { boost::asio::detail::addressof(o->handler_), o, o };
		handler_work<Handler> w(o->handler_);

		BOOST_ASIO_HANDLER_COMPLETION((*o));

		// Make a copy of the handler so that the memory can be deallocated before
		// the upcall is made. Even if we're not about to make an upcall, a
		// sub-object of the handler may be the true owner of the memory associated
		// with the handler. Consequently, a local copy of the handler is required
		// to ensure that any owning sub-object remains valid until after we have
		// deallocated the memory here.
		detail::binder1<Handler, boost::system::error_code> handler(o->handler_, o->ec_);
		p.h = boost::asio::detail::addressof(handler.handler_);
		p.reset();

		// Make the upcall if required.
		if (owner)
		{
			fenced_block b(fenced_block::half);
			BOOST_ASIO_HANDLER_INVOCATION_BEGIN((handler.arg1_));
			w.complete(handler, handler.handler_);
			BOOST_ASIO_HANDLER_INVOCATION_END;
		}
	}

private:
	Handler handler_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_REACTIVE_SOCKET_CONNECT_OP_HPP

