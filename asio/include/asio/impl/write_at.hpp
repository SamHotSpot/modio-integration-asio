//
// impl/write_at.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IMPL_WRITE_AT_HPP
#define ASIO_IMPL_WRITE_AT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/associator.hpp"
#include "asio/buffer.hpp"
#include "asio/detail/array_fwd.hpp"
#include "asio/detail/base_from_cancellation_state.hpp"
#include "asio/detail/base_from_completion_cond.hpp"
#include "asio/detail/bind_handler.hpp"
#include "asio/detail/consuming_buffers.hpp"
#include "asio/detail/dependent_type.hpp"
#include "asio/detail/handler_alloc_helpers.hpp"
#include "asio/detail/handler_cont_helpers.hpp"
#include "asio/detail/handler_invoke_helpers.hpp"
#include "asio/detail/handler_tracking.hpp"
#include "asio/detail/handler_type_requirements.hpp"
#include "asio/detail/non_const_lvalue.hpp"
#include "asio/detail/throw_error.hpp"

#include "asio/detail/push_options.hpp"

namespace ASIO_LIBNS {

namespace detail
{
  template <typename SyncRandomAccessWriteDevice, typename ConstBufferSequence,
      typename ConstBufferIterator, typename CompletionCondition>
  std::size_t write_at_buffer_sequence(SyncRandomAccessWriteDevice& d,
      uint64_t offset, const ConstBufferSequence& buffers,
      const ConstBufferIterator&, CompletionCondition completion_condition,
      ASIO_LIBNS::error_code& ec)
  {
    ec = ASIO_LIBNS::error_code();
    ASIO_LIBNS::detail::consuming_buffers<const_buffer,
        ConstBufferSequence, ConstBufferIterator> tmp(buffers);
    while (!tmp.empty())
    {
      if (std::size_t max_size = detail::adapt_completion_condition_result(
            completion_condition(ec, tmp.total_consumed())))
      {
        tmp.consume(d.write_some_at(offset + tmp.total_consumed(),
              tmp.prepare(max_size), ec));
      }
      else
        break;
    }
    return tmp.total_consumed();
  }
} // namespace detail

template <typename SyncRandomAccessWriteDevice, typename ConstBufferSequence,
    typename CompletionCondition>
std::size_t write_at(SyncRandomAccessWriteDevice& d,
    uint64_t offset, const ConstBufferSequence& buffers,
    CompletionCondition completion_condition, ASIO_LIBNS::error_code& ec)
{
  return detail::write_at_buffer_sequence(d, offset, buffers,
      ASIO_LIBNS::buffer_sequence_begin(buffers),
      ASIO_MOVE_CAST(CompletionCondition)(completion_condition), ec);
}

template <typename SyncRandomAccessWriteDevice, typename ConstBufferSequence>
inline std::size_t write_at(SyncRandomAccessWriteDevice& d,
    uint64_t offset, const ConstBufferSequence& buffers)
{
  ASIO_LIBNS::error_code ec;
  std::size_t bytes_transferred = write_at(
      d, offset, buffers, transfer_all(), ec);
  ASIO_LIBNS::detail::throw_error(ec, "write_at");
  return bytes_transferred;
}

template <typename SyncRandomAccessWriteDevice, typename ConstBufferSequence>
inline std::size_t write_at(SyncRandomAccessWriteDevice& d,
    uint64_t offset, const ConstBufferSequence& buffers,
    ASIO_LIBNS::error_code& ec)
{
  return write_at(d, offset, buffers, transfer_all(), ec);
}

template <typename SyncRandomAccessWriteDevice, typename ConstBufferSequence,
    typename CompletionCondition>
inline std::size_t write_at(SyncRandomAccessWriteDevice& d,
    uint64_t offset, const ConstBufferSequence& buffers,
    CompletionCondition completion_condition)
{
  ASIO_LIBNS::error_code ec;
  std::size_t bytes_transferred = write_at(d, offset, buffers,
      ASIO_MOVE_CAST(CompletionCondition)(completion_condition), ec);
  ASIO_LIBNS::detail::throw_error(ec, "write_at");
  return bytes_transferred;
}

#if !defined(ASIO_NO_EXTENSIONS)
#if !defined(ASIO_NO_IOSTREAM)

template <typename SyncRandomAccessWriteDevice, typename Allocator,
    typename CompletionCondition>
std::size_t write_at(SyncRandomAccessWriteDevice& d,
    uint64_t offset, ASIO_LIBNS::basic_streambuf<Allocator>& b,
    CompletionCondition completion_condition, ASIO_LIBNS::error_code& ec)
{
  std::size_t bytes_transferred = write_at(d, offset, b.data(),
      ASIO_MOVE_CAST(CompletionCondition)(completion_condition), ec);
  b.consume(bytes_transferred);
  return bytes_transferred;
}

template <typename SyncRandomAccessWriteDevice, typename Allocator>
inline std::size_t write_at(SyncRandomAccessWriteDevice& d,
    uint64_t offset, ASIO_LIBNS::basic_streambuf<Allocator>& b)
{
  ASIO_LIBNS::error_code ec;
  std::size_t bytes_transferred = write_at(d, offset, b, transfer_all(), ec);
  ASIO_LIBNS::detail::throw_error(ec, "write_at");
  return bytes_transferred;
}

template <typename SyncRandomAccessWriteDevice, typename Allocator>
inline std::size_t write_at(SyncRandomAccessWriteDevice& d,
    uint64_t offset, ASIO_LIBNS::basic_streambuf<Allocator>& b,
    ASIO_LIBNS::error_code& ec)
{
  return write_at(d, offset, b, transfer_all(), ec);
}

template <typename SyncRandomAccessWriteDevice, typename Allocator,
    typename CompletionCondition>
inline std::size_t write_at(SyncRandomAccessWriteDevice& d,
    uint64_t offset, ASIO_LIBNS::basic_streambuf<Allocator>& b,
    CompletionCondition completion_condition)
{
  ASIO_LIBNS::error_code ec;
  std::size_t bytes_transferred = write_at(d, offset, b,
      ASIO_MOVE_CAST(CompletionCondition)(completion_condition), ec);
  ASIO_LIBNS::detail::throw_error(ec, "write_at");
  return bytes_transferred;
}

#endif // !defined(ASIO_NO_IOSTREAM)
#endif // !defined(ASIO_NO_EXTENSIONS)

namespace detail
{
  template <typename AsyncRandomAccessWriteDevice,
      typename ConstBufferSequence, typename ConstBufferIterator,
      typename CompletionCondition, typename WriteHandler>
  class write_at_op
    : public base_from_cancellation_state<WriteHandler>,
      base_from_completion_cond<CompletionCondition>
  {
  public:
    write_at_op(AsyncRandomAccessWriteDevice& device,
        uint64_t offset, const ConstBufferSequence& buffers,
        CompletionCondition& completion_condition, WriteHandler& handler)
      : base_from_cancellation_state<WriteHandler>(
          handler, enable_partial_cancellation()),
        base_from_completion_cond<CompletionCondition>(completion_condition),
        device_(device),
        offset_(offset),
        buffers_(buffers),
        start_(0),
        handler_(ASIO_MOVE_CAST(WriteHandler)(handler))
    {
    }

#if defined(ASIO_HAS_MOVE)
    write_at_op(const write_at_op& other)
      : base_from_cancellation_state<WriteHandler>(other),
        base_from_completion_cond<CompletionCondition>(other),
        device_(other.device_),
        offset_(other.offset_),
        buffers_(other.buffers_),
        start_(other.start_),
        handler_(other.handler_)
    {
    }

    write_at_op(write_at_op&& other)
      : base_from_cancellation_state<WriteHandler>(
          ASIO_MOVE_CAST(base_from_cancellation_state<
            WriteHandler>)(other)),
        base_from_completion_cond<CompletionCondition>(
          ASIO_MOVE_CAST(base_from_completion_cond<
            CompletionCondition>)(other)),
        device_(other.device_),
        offset_(other.offset_),
        buffers_(ASIO_MOVE_CAST(buffers_type)(other.buffers_)),
        start_(other.start_),
        handler_(ASIO_MOVE_CAST(WriteHandler)(other.handler_))
    {
    }
#endif // defined(ASIO_HAS_MOVE)

    void operator()(ASIO_LIBNS::error_code ec,
        std::size_t bytes_transferred, int start = 0)
    {
      std::size_t max_size;
      switch (start_ = start)
      {
        case 1:
        max_size = this->check_for_completion(ec, buffers_.total_consumed());
        for (;;)
        {
          {
            ASIO_HANDLER_LOCATION((__FILE__, __LINE__, "async_write_at"));
            device_.async_write_some_at(
                offset_ + buffers_.total_consumed(), buffers_.prepare(max_size),
                ASIO_MOVE_CAST(write_at_op)(*this));
          }
          return; default:
          buffers_.consume(bytes_transferred);
          if ((!ec && bytes_transferred == 0) || buffers_.empty())
            break;
          max_size = this->check_for_completion(ec, buffers_.total_consumed());
          if (max_size == 0)
            break;
          if (this->cancelled() != cancellation_type::none)
          {
            ec = ASIO_LIBNS::error::operation_aborted;
            break;
          }
        }

        ASIO_MOVE_OR_LVALUE(WriteHandler)(handler_)(
            static_cast<const ASIO_LIBNS::error_code&>(ec),
            static_cast<const std::size_t&>(buffers_.total_consumed()));
      }
    }

  //private:
    typedef ASIO_LIBNS::detail::consuming_buffers<const_buffer,
        ConstBufferSequence, ConstBufferIterator> buffers_type;

    AsyncRandomAccessWriteDevice& device_;
    uint64_t offset_;
    buffers_type buffers_;
    int start_;
    WriteHandler handler_;
  };

  template <typename AsyncRandomAccessWriteDevice,
      typename ConstBufferSequence, typename ConstBufferIterator,
      typename CompletionCondition, typename WriteHandler>
  inline asio_handler_allocate_is_deprecated
  asio_handler_allocate(std::size_t size,
      write_at_op<AsyncRandomAccessWriteDevice, ConstBufferSequence,
        ConstBufferIterator, CompletionCondition, WriteHandler>* this_handler)
  {
#if defined(ASIO_NO_DEPRECATED)
    asio_handler_alloc_helpers::allocate(size, this_handler->handler_);
    return asio_handler_allocate_is_no_longer_used();
#else // defined(ASIO_NO_DEPRECATED)
    return asio_handler_alloc_helpers::allocate(
        size, this_handler->handler_);
#endif // defined(ASIO_NO_DEPRECATED)
  }

  template <typename AsyncRandomAccessWriteDevice,
      typename ConstBufferSequence, typename ConstBufferIterator,
      typename CompletionCondition, typename WriteHandler>
  inline asio_handler_deallocate_is_deprecated
  asio_handler_deallocate(void* pointer, std::size_t size,
      write_at_op<AsyncRandomAccessWriteDevice, ConstBufferSequence,
        ConstBufferIterator, CompletionCondition, WriteHandler>* this_handler)
  {
    asio_handler_alloc_helpers::deallocate(
        pointer, size, this_handler->handler_);
#if defined(ASIO_NO_DEPRECATED)
    return asio_handler_deallocate_is_no_longer_used();
#endif // defined(ASIO_NO_DEPRECATED)
  }

  template <typename AsyncRandomAccessWriteDevice,
      typename ConstBufferSequence, typename ConstBufferIterator,
      typename CompletionCondition, typename WriteHandler>
  inline bool asio_handler_is_continuation(
      write_at_op<AsyncRandomAccessWriteDevice, ConstBufferSequence,
        ConstBufferIterator, CompletionCondition, WriteHandler>* this_handler)
  {
    return this_handler->start_ == 0 ? true
      : asio_handler_cont_helpers::is_continuation(
          this_handler->handler_);
  }

  template <typename Function, typename AsyncRandomAccessWriteDevice,
      typename ConstBufferSequence, typename ConstBufferIterator,
      typename CompletionCondition, typename WriteHandler>
  inline asio_handler_invoke_is_deprecated
  asio_handler_invoke(Function& function,
      write_at_op<AsyncRandomAccessWriteDevice, ConstBufferSequence,
        ConstBufferIterator, CompletionCondition, WriteHandler>* this_handler)
  {
    asio_handler_invoke_helpers::invoke(
        function, this_handler->handler_);
#if defined(ASIO_NO_DEPRECATED)
    return asio_handler_invoke_is_no_longer_used();
#endif // defined(ASIO_NO_DEPRECATED)
  }

  template <typename Function, typename AsyncRandomAccessWriteDevice,
      typename ConstBufferSequence, typename ConstBufferIterator,
      typename CompletionCondition, typename WriteHandler>
  inline asio_handler_invoke_is_deprecated
  asio_handler_invoke(const Function& function,
      write_at_op<AsyncRandomAccessWriteDevice, ConstBufferSequence,
        ConstBufferIterator, CompletionCondition, WriteHandler>* this_handler)
  {
    asio_handler_invoke_helpers::invoke(
        function, this_handler->handler_);
#if defined(ASIO_NO_DEPRECATED)
    return asio_handler_invoke_is_no_longer_used();
#endif // defined(ASIO_NO_DEPRECATED)
  }

  template <typename AsyncRandomAccessWriteDevice,
      typename ConstBufferSequence, typename ConstBufferIterator,
      typename CompletionCondition, typename WriteHandler>
  inline void start_write_at_op(AsyncRandomAccessWriteDevice& d,
      uint64_t offset, const ConstBufferSequence& buffers,
      const ConstBufferIterator&, CompletionCondition& completion_condition,
      WriteHandler& handler)
  {
    detail::write_at_op<AsyncRandomAccessWriteDevice, ConstBufferSequence,
      ConstBufferIterator, CompletionCondition, WriteHandler>(
        d, offset, buffers, completion_condition, handler)(
          ASIO_LIBNS::error_code(), 0, 1);
  }

  template <typename AsyncRandomAccessWriteDevice>
  class initiate_async_write_at
  {
  public:
    typedef typename AsyncRandomAccessWriteDevice::executor_type executor_type;

    explicit initiate_async_write_at(AsyncRandomAccessWriteDevice& device)
      : device_(device)
    {
    }

    executor_type get_executor() const ASIO_NOEXCEPT
    {
      return device_.get_executor();
    }

    template <typename WriteHandler, typename ConstBufferSequence,
        typename CompletionCondition>
    void operator()(ASIO_MOVE_ARG(WriteHandler) handler,
        uint64_t offset, const ConstBufferSequence& buffers,
        ASIO_MOVE_ARG(CompletionCondition) completion_cond) const
    {
      // If you get an error on the following line it means that your handler
      // does not meet the documented type requirements for a WriteHandler.
      ASIO_WRITE_HANDLER_CHECK(WriteHandler, handler) type_check;

      non_const_lvalue<WriteHandler> handler2(handler);
      non_const_lvalue<CompletionCondition> completion_cond2(completion_cond);
      start_write_at_op(device_, offset, buffers,
          ASIO_LIBNS::buffer_sequence_begin(buffers),
          completion_cond2.value, handler2.value);
    }

  private:
    AsyncRandomAccessWriteDevice& device_;
  };
} // namespace detail

#if !defined(GENERATING_DOCUMENTATION)

template <template <typename, typename> class Associator,
    typename AsyncRandomAccessWriteDevice, typename ConstBufferSequence,
    typename ConstBufferIterator, typename CompletionCondition,
    typename WriteHandler, typename DefaultCandidate>
struct associator<Associator,
    detail::write_at_op<AsyncRandomAccessWriteDevice, ConstBufferSequence,
      ConstBufferIterator, CompletionCondition, WriteHandler>,
    DefaultCandidate>
  : Associator<WriteHandler, DefaultCandidate>
{
  static typename Associator<WriteHandler, DefaultCandidate>::type get(
      const detail::write_at_op<AsyncRandomAccessWriteDevice,
        ConstBufferSequence, ConstBufferIterator,
        CompletionCondition, WriteHandler>& h,
      const DefaultCandidate& c = DefaultCandidate()) ASIO_NOEXCEPT
  {
    return Associator<WriteHandler, DefaultCandidate>::get(h.handler_, c);
  }
};

#endif // !defined(GENERATING_DOCUMENTATION)

template <typename AsyncRandomAccessWriteDevice,
    typename ConstBufferSequence, typename CompletionCondition,
    ASIO_COMPLETION_TOKEN_FOR(void (ASIO_LIBNS::error_code,
      std::size_t)) WriteToken>
inline ASIO_INITFN_AUTO_RESULT_TYPE_PREFIX(WriteToken,
    void (ASIO_LIBNS::error_code, std::size_t))
async_write_at(AsyncRandomAccessWriteDevice& d,
    uint64_t offset, const ConstBufferSequence& buffers,
    CompletionCondition completion_condition,
    ASIO_MOVE_ARG(WriteToken) token)
  ASIO_INITFN_AUTO_RESULT_TYPE_SUFFIX((
    async_initiate<WriteToken,
      void (ASIO_LIBNS::error_code, std::size_t)>(
        declval<detail::initiate_async_write_at<
          AsyncRandomAccessWriteDevice> >(),
        token, offset, buffers,
        ASIO_MOVE_CAST(CompletionCondition)(completion_condition))))
{
  return async_initiate<WriteToken,
    void (ASIO_LIBNS::error_code, std::size_t)>(
      detail::initiate_async_write_at<AsyncRandomAccessWriteDevice>(d),
      token, offset, buffers,
      ASIO_MOVE_CAST(CompletionCondition)(completion_condition));
}

template <typename AsyncRandomAccessWriteDevice, typename ConstBufferSequence,
    ASIO_COMPLETION_TOKEN_FOR(void (ASIO_LIBNS::error_code,
      std::size_t)) WriteToken>
inline ASIO_INITFN_AUTO_RESULT_TYPE_PREFIX(WriteToken,
    void (ASIO_LIBNS::error_code, std::size_t))
async_write_at(AsyncRandomAccessWriteDevice& d,
    uint64_t offset, const ConstBufferSequence& buffers,
    ASIO_MOVE_ARG(WriteToken) token)
  ASIO_INITFN_AUTO_RESULT_TYPE_SUFFIX((
    async_initiate<WriteToken,
      void (ASIO_LIBNS::error_code, std::size_t)>(
        declval<detail::initiate_async_write_at<
          AsyncRandomAccessWriteDevice> >(),
        token, offset, buffers, transfer_all())))
{
  return async_initiate<WriteToken,
    void (ASIO_LIBNS::error_code, std::size_t)>(
      detail::initiate_async_write_at<AsyncRandomAccessWriteDevice>(d),
      token, offset, buffers, transfer_all());
}

#if !defined(ASIO_NO_EXTENSIONS)
#if !defined(ASIO_NO_IOSTREAM)

namespace detail
{
  template <typename Allocator, typename WriteHandler>
  class write_at_streambuf_op
  {
  public:
    write_at_streambuf_op(
        ASIO_LIBNS::basic_streambuf<Allocator>& streambuf,
        WriteHandler& handler)
      : streambuf_(streambuf),
        handler_(ASIO_MOVE_CAST(WriteHandler)(handler))
    {
    }

#if defined(ASIO_HAS_MOVE)
    write_at_streambuf_op(const write_at_streambuf_op& other)
      : streambuf_(other.streambuf_),
        handler_(other.handler_)
    {
    }

    write_at_streambuf_op(write_at_streambuf_op&& other)
      : streambuf_(other.streambuf_),
        handler_(ASIO_MOVE_CAST(WriteHandler)(other.handler_))
    {
    }
#endif // defined(ASIO_HAS_MOVE)

    void operator()(const ASIO_LIBNS::error_code& ec,
        const std::size_t bytes_transferred)
    {
      streambuf_.consume(bytes_transferred);
      ASIO_MOVE_OR_LVALUE(WriteHandler)(handler_)(ec, bytes_transferred);
    }

  //private:
    ASIO_LIBNS::basic_streambuf<Allocator>& streambuf_;
    WriteHandler handler_;
  };

  template <typename Allocator, typename WriteHandler>
  inline asio_handler_allocate_is_deprecated
  asio_handler_allocate(std::size_t size,
      write_at_streambuf_op<Allocator, WriteHandler>* this_handler)
  {
#if defined(ASIO_NO_DEPRECATED)
    asio_handler_alloc_helpers::allocate(size, this_handler->handler_);
    return asio_handler_allocate_is_no_longer_used();
#else // defined(ASIO_NO_DEPRECATED)
    return asio_handler_alloc_helpers::allocate(
        size, this_handler->handler_);
#endif // defined(ASIO_NO_DEPRECATED)
  }

  template <typename Allocator, typename WriteHandler>
  inline asio_handler_deallocate_is_deprecated
  asio_handler_deallocate(void* pointer, std::size_t size,
      write_at_streambuf_op<Allocator, WriteHandler>* this_handler)
  {
    asio_handler_alloc_helpers::deallocate(
        pointer, size, this_handler->handler_);
#if defined(ASIO_NO_DEPRECATED)
    return asio_handler_deallocate_is_no_longer_used();
#endif // defined(ASIO_NO_DEPRECATED)
  }

  template <typename Allocator, typename WriteHandler>
  inline bool asio_handler_is_continuation(
      write_at_streambuf_op<Allocator, WriteHandler>* this_handler)
  {
    return asio_handler_cont_helpers::is_continuation(
        this_handler->handler_);
  }

  template <typename Function, typename Allocator, typename WriteHandler>
  inline asio_handler_invoke_is_deprecated
  asio_handler_invoke(Function& function,
      write_at_streambuf_op<Allocator, WriteHandler>* this_handler)
  {
    asio_handler_invoke_helpers::invoke(
        function, this_handler->handler_);
#if defined(ASIO_NO_DEPRECATED)
    return asio_handler_invoke_is_no_longer_used();
#endif // defined(ASIO_NO_DEPRECATED)
  }

  template <typename Function, typename Allocator, typename WriteHandler>
  inline asio_handler_invoke_is_deprecated
  asio_handler_invoke(const Function& function,
      write_at_streambuf_op<Allocator, WriteHandler>* this_handler)
  {
    asio_handler_invoke_helpers::invoke(
        function, this_handler->handler_);
#if defined(ASIO_NO_DEPRECATED)
    return asio_handler_invoke_is_no_longer_used();
#endif // defined(ASIO_NO_DEPRECATED)
  }

  template <typename AsyncRandomAccessWriteDevice>
  class initiate_async_write_at_streambuf
  {
  public:
    typedef typename AsyncRandomAccessWriteDevice::executor_type executor_type;

    explicit initiate_async_write_at_streambuf(
        AsyncRandomAccessWriteDevice& device)
      : device_(device)
    {
    }

    executor_type get_executor() const ASIO_NOEXCEPT
    {
      return device_.get_executor();
    }

    template <typename WriteHandler,
        typename Allocator, typename CompletionCondition>
    void operator()(ASIO_MOVE_ARG(WriteHandler) handler,
        uint64_t offset, basic_streambuf<Allocator>* b,
        ASIO_MOVE_ARG(CompletionCondition) completion_condition) const
    {
      // If you get an error on the following line it means that your handler
      // does not meet the documented type requirements for a WriteHandler.
      ASIO_WRITE_HANDLER_CHECK(WriteHandler, handler) type_check;

      non_const_lvalue<WriteHandler> handler2(handler);
      async_write_at(device_, offset, b->data(),
          ASIO_MOVE_CAST(CompletionCondition)(completion_condition),
          write_at_streambuf_op<Allocator, typename decay<WriteHandler>::type>(
            *b, handler2.value));
    }

  private:
    AsyncRandomAccessWriteDevice& device_;
  };
} // namespace detail

#if !defined(GENERATING_DOCUMENTATION)

template <template <typename, typename> class Associator,
    typename Executor, typename WriteHandler, typename DefaultCandidate>
struct associator<Associator,
    detail::write_at_streambuf_op<Executor, WriteHandler>,
    DefaultCandidate>
  : Associator<WriteHandler, DefaultCandidate>
{
  static typename Associator<WriteHandler, DefaultCandidate>::type get(
      const detail::write_at_streambuf_op<Executor, WriteHandler>& h,
      const DefaultCandidate& c = DefaultCandidate()) ASIO_NOEXCEPT
  {
    return Associator<WriteHandler, DefaultCandidate>::get(h.handler_, c);
  }
};

#endif // !defined(GENERATING_DOCUMENTATION)

template <typename AsyncRandomAccessWriteDevice,
    typename Allocator, typename CompletionCondition,
    ASIO_COMPLETION_TOKEN_FOR(void (ASIO_LIBNS::error_code,
      std::size_t)) WriteToken>
inline ASIO_INITFN_AUTO_RESULT_TYPE_PREFIX(WriteToken,
    void (ASIO_LIBNS::error_code, std::size_t))
async_write_at(AsyncRandomAccessWriteDevice& d,
    uint64_t offset, ASIO_LIBNS::basic_streambuf<Allocator>& b,
    CompletionCondition completion_condition,
    ASIO_MOVE_ARG(WriteToken) token)
  ASIO_INITFN_AUTO_RESULT_TYPE_SUFFIX((
    async_initiate<WriteToken,
      void (ASIO_LIBNS::error_code, std::size_t)>(
        declval<detail::initiate_async_write_at_streambuf<
          AsyncRandomAccessWriteDevice> >(),
        token, offset, &b,
        ASIO_MOVE_CAST(CompletionCondition)(completion_condition))))
{
  return async_initiate<WriteToken,
    void (ASIO_LIBNS::error_code, std::size_t)>(
      detail::initiate_async_write_at_streambuf<
        AsyncRandomAccessWriteDevice>(d),
      token, offset, &b,
      ASIO_MOVE_CAST(CompletionCondition)(completion_condition));
}

template <typename AsyncRandomAccessWriteDevice, typename Allocator,
    ASIO_COMPLETION_TOKEN_FOR(void (ASIO_LIBNS::error_code,
      std::size_t)) WriteToken>
inline ASIO_INITFN_AUTO_RESULT_TYPE_PREFIX(WriteToken,
    void (ASIO_LIBNS::error_code, std::size_t))
async_write_at(AsyncRandomAccessWriteDevice& d,
    uint64_t offset, ASIO_LIBNS::basic_streambuf<Allocator>& b,
    ASIO_MOVE_ARG(WriteToken) token)
  ASIO_INITFN_AUTO_RESULT_TYPE_SUFFIX((
    async_initiate<WriteToken,
      void (ASIO_LIBNS::error_code, std::size_t)>(
        declval<detail::initiate_async_write_at_streambuf<
          AsyncRandomAccessWriteDevice> >(),
        token, offset, &b, transfer_all())))
{
  return async_initiate<WriteToken,
    void (ASIO_LIBNS::error_code, std::size_t)>(
      detail::initiate_async_write_at_streambuf<
        AsyncRandomAccessWriteDevice>(d),
      token, offset, &b, transfer_all());
}

#endif // !defined(ASIO_NO_IOSTREAM)
#endif // !defined(ASIO_NO_EXTENSIONS)

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_IMPL_WRITE_AT_HPP
