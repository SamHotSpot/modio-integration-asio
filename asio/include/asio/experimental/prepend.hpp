//
// experimental/prepend.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXPERIMENTAL_PREPEND_HPP
#define ASIO_EXPERIMENTAL_PREPEND_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/prepend.hpp"

#include "asio/detail/push_options.hpp"

namespace ASIO_LIBNS {
namespace experimental {

#if !defined(ASIO_NO_DEPRECATED)
using ASIO_LIBNS::prepend_t;
using ASIO_LIBNS::prepend;
#endif // !defined(ASIO_NO_DEPRECATED)

} // namespace experimental
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_EXPERIMENTAL_PREPEND_HPP
