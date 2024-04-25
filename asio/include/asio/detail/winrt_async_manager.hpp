//
// detail/winrt_async_manager.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_WINRT_ASYNC_MANAGER_HPP
#define ASIO_DETAIL_WINRT_ASYNC_MANAGER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"

#if defined(ASIO_WINDOWS_RUNTIME)

#include <future>
#include "asio/detail/atomic_count.hpp"
#include "asio/detail/winrt_async_op.hpp"
#include "asio/error.hpp"
#include "asio/execution_context.hpp"

#if defined(ASIO_HAS_IOCP)
# include "asio/detail/win_iocp_io_context.hpp"
#else // defined(ASIO_HAS_IOCP)
# include "asio/detail/scheduler.hpp"
#endif // defined(ASIO_HAS_IOCP)

#include "asio/detail/push_options.hpp"

namespace ASIO_LIBNS {
namespace detail {

class winrt_async_manager
  : public execution_context_service_base<winrt_async_manager>
{
public:
  // Constructor.
  winrt_async_manager(execution_context& context)
    : execution_context_service_base<winrt_async_manager>(context),
      scheduler_(use_service<scheduler_impl>(context)),
      outstanding_ops_(1)
  {
  }

  // Destructor.
  ~winrt_async_manager()
  {
  }

  // Destroy all user-defined handler objects owned by the service.
  void shutdown()
  {
    if (--outstanding_ops_ > 0)
    {
      // Block until last operation is complete.
      std::future<void> f = promise_.get_future();
      f.wait();
    }
  }

  void sync(winrt::Windows::Foundation::IAsyncAction action,
      ASIO_LIBNS::error_code& ec)
  {
    using namespace winrt::Windows::Foundation;
    using winrt::Windows::Foundation::AsyncStatus;

    auto promise = std::make_shared<std::promise<ASIO_LIBNS::error_code>>();
    auto future = promise->get_future();

    action.Completed() = AsyncActionCompletedHandler(
      [promise](IAsyncAction action, AsyncStatus status)
      {
        switch (status)
        {
        case AsyncStatus::Canceled:
          promise->set_value(ASIO_LIBNS::error::operation_aborted);
          break;
        case AsyncStatus::Error:
        case AsyncStatus::Completed:
        default:
          ASIO_LIBNS::error_code ec(
              action.ErrorCode(),
              ASIO_LIBNS::system_category());
          promise->set_value(ec);
          break;
        }
      });

    ec = future.get();
  }

  template <typename TResult>
  TResult sync(winrt::Windows::Foundation::IAsyncOperation<TResult> operation,
      ASIO_LIBNS::error_code& ec)
  {
    using namespace winrt::Windows::Foundation;
    using winrt::Windows::Foundation::AsyncStatus;

    auto promise = std::make_shared<std::promise<ASIO_LIBNS::error_code>>();
    auto future = promise->get_future();

    operation->Completed = ref new AsyncOperationCompletedHandler<TResult>(
      [promise](IAsyncOperation<TResult> operation, AsyncStatus status)
      {
        switch (status)
        {
        case AsyncStatus::Canceled:
          promise->set_value(ASIO_LIBNS::error::operation_aborted);
          break;
        case AsyncStatus::Error:
        case AsyncStatus::Completed:
        default:
          ASIO_LIBNS::error_code ec(
              operation->ErrorCode(),
              ASIO_LIBNS::system_category());
          promise->set_value(ec);
          break;
        }
      });

    ec = future.get();
    return operation->GetResults();
  }

  template <typename TResult, typename TProgress>
  TResult sync(
      winrt::Windows::Foundation::IAsyncOperationWithProgress<
        TResult, TProgress> operation,
      ASIO_LIBNS::error_code& ec)
  {
    using namespace winrt::Windows::Foundation;
    using winrt::Windows::Foundation::AsyncStatus;

    auto promise = std::make_shared<std::promise<ASIO_LIBNS::error_code>>();
    auto future = promise->get_future();

    operation->Completed
      = ref new AsyncOperationWithProgressCompletedHandler<TResult, TProgress>(
        [promise](IAsyncOperationWithProgress<TResult, TProgress> operation,
          AsyncStatus status)
        {
          switch (status)
          {
          case AsyncStatus::Canceled:
            promise->set_value(ASIO_LIBNS::error::operation_aborted);
            break;
          case AsyncStatus::Started:
            break;
          case AsyncStatus::Error:
          case AsyncStatus::Completed:
          default:
            ASIO_LIBNS::error_code ec(
                operation->ErrorCode(),
                ASIO_LIBNS::system_category());
            promise->set_value(ec);
            break;
          }
        });

    ec = future.get();
    return operation->GetResults();
  }

  void async(winrt::Windows::Foundation::IAsyncAction action,
      winrt_async_op<void>* handler)
  {
    using namespace winrt::Windows::Foundation;
    using winrt::Windows::Foundation::AsyncStatus;

    auto on_completed = AsyncActionCompletedHandler(
      [this, handler](IAsyncAction action, AsyncStatus status)
      {
        switch (status)
        {
        case AsyncStatus::Canceled:
          handler->ec_ = ASIO_LIBNS::error::operation_aborted;
          break;
        case AsyncStatus::Started:
          return;
        case AsyncStatus::Completed:
        case AsyncStatus::Error:
        default:
          handler->ec_ = ASIO_LIBNS::error_code(
              action.ErrorCode(),
              ASIO_LIBNS::system_category());
          break;
        }
        scheduler_.post_deferred_completion(handler);
        if (--outstanding_ops_ == 0)
          promise_.set_value();
      });

    scheduler_.work_started();
    ++outstanding_ops_;
    action.Completed() = on_completed;
  }

  template <typename TResult>
  void async(winrt::Windows::Foundation::IAsyncOperation<TResult> operation,
      winrt_async_op<TResult>* handler)
  {
    using namespace winrt::Windows::Foundation;
    using winrt::Windows::Foundation::AsyncStatus;

    auto on_completed = AsyncOperationCompletedHandler<TResult>(
      [this, handler](IAsyncOperation<TResult> operation, AsyncStatus status)
      {
        switch (status)
        {
        case AsyncStatus::Canceled:
          handler->ec_ = ASIO_LIBNS::error::operation_aborted;
          break;
        case AsyncStatus::Started:
          return;
        case AsyncStatus::Completed:
          handler->result_ = operation->GetResults();
          // Fall through.
        case AsyncStatus::Error:
        default:
          handler->ec_ = ASIO_LIBNS::error_code(
              operation.ErrorCode(),
              ASIO_LIBNS::system_category());
          break;
        }
        scheduler_.post_deferred_completion(handler);
        if (--outstanding_ops_ == 0)
          promise_.set_value();
      });

    scheduler_.work_started();
    ++outstanding_ops_;
    operation.Completed() = on_completed;
  }

  template <typename TResult, typename TProgress>
  void async(
      winrt::Windows::Foundation::IAsyncOperationWithProgress<
        TResult, TProgress> operation,
      winrt_async_op<TResult>* handler)
  {
    using namespace winrt::Windows::Foundation;
    using winrt::Windows::Foundation::AsyncStatus;

    auto on_completed
      = ref new AsyncOperationWithProgressCompletedHandler<TResult, TProgress>(
        [this, handler](IAsyncOperationWithProgress<
          TResult, TProgress> operation, AsyncStatus status)
        {
          switch (status)
          {
          case AsyncStatus::Canceled:
            handler->ec_ = ASIO_LIBNS::error::operation_aborted;
            break;
          case AsyncStatus::Started:
            return;
          case AsyncStatus::Completed:
            handler->result_ = operation->GetResults();
            // Fall through.
          case AsyncStatus::Error:
          default:
            handler->ec_ = ASIO_LIBNS::error_code(
                operation.ErrorCode(),
                ASIO_LIBNS::system_category());
            break;
          }
          scheduler_.post_deferred_completion(handler);
          if (--outstanding_ops_ == 0)
            promise_.set_value();
        });

    scheduler_.work_started();
    ++outstanding_ops_;
    operation.Completed() = on_completed;
  }

private:
  // The scheduler implementation used to post completed handlers.
#if defined(ASIO_HAS_IOCP)
  typedef class win_iocp_io_context scheduler_impl;
#else
  typedef class scheduler scheduler_impl;
#endif
  scheduler_impl& scheduler_;

  // Count of outstanding operations.
  atomic_count outstanding_ops_;

  // Used to keep wait for outstanding operations to complete.
  std::promise<void> promise_;
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // defined(ASIO_WINDOWS_RUNTIME)

#endif // ASIO_DETAIL_WINRT_ASYNC_MANAGER_HPP
