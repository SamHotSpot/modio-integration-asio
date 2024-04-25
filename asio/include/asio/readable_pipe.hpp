//
// readable_pipe.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_READABLE_PIPE_HPP
#define ASIO_READABLE_PIPE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_HAS_PIPE) \
  || defined(GENERATING_DOCUMENTATION)

#include "asio/basic_readable_pipe.hpp"

namespace ASIO_LIBNS {

/// Typedef for the typical usage of a readable pipe.
typedef basic_readable_pipe<> readable_pipe;

} // namespace asio

#endif // defined(ASIO_HAS_PIPE)
       //   || defined(GENERATING_DOCUMENTATION)

#endif // ASIO_READABLE_PIPE_HPP