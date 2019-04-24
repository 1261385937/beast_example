#pragma once
#ifndef COROUTINRPOOL_H
#define COROUTINRPOOL_H

#include <memory>
#include <functional>

namespace boost::asio
{
	template <typename Handler>
	class basic_yield_context;

	template <typename T, typename Executor>
	class executor_binder;

	class executor;
	class io_context;
	using yield_context = basic_yield_context<executor_binder<void(*)(), executor>>;
}

namespace dividend
{
	class IoContextPool;
	class CoroutinePool :public std::enable_shared_from_this<CoroutinePool>
	{
	public:
		using CoroutineContext = std::function<void(boost::asio::io_context&, const boost::asio::yield_context&)>;
	private:
		int pool_size_;
		std::shared_ptr<IoContextPool> io_context_pool_ptr_;
		struct Impl;
		std::unique_ptr<Impl> impl_;
	public:
		explicit CoroutinePool(int coroutine_pool_size, int thread_pool_size);
		~CoroutinePool();
		void start();
		void stop();
		void setCoroutineSleep(boost::asio::io_context& ioc,
			const boost::asio::yield_context& yield, uint32_t milliseconds);
		bool addCoroutineTask(const CoroutineContext& excute_context);
		bool addCoroutineTask(CoroutineContext&& excute_context);
	private:
		CoroutinePool(const CoroutinePool&) = delete;
		CoroutinePool& operator=(const CoroutinePool&) = delete;
	};
}

#endif