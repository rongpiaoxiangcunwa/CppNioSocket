#ifndef BOOST_ASIO_DETAIL_SCTP_SOCKET_TYPES_HPP
#define BOOST_ASIO_DETAIL_SCTP_SOCKET_TYPES_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/socket_types.hpp>

#include <boost/asio/detail/push_options.hpp>
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
#include <ws2sctp.h>  // use Bruce Cran's SctpDrv implementation
#else
#include <netinet/sctp.h>
#endif

namespace boost {
namespace asio {
namespace detail {

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif