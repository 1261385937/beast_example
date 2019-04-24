#include "CoroutinePool.h"
#include <atomic>
#include "boost/asio/io_context.hpp"
#include "boost/asio/spawn.hpp"
#include "boost/asio/steady_timer.hpp"
#include "concurrentqueue.h"
#include "log.h"
#include "IoContextPool.h"

struct dividend::CoroutinePool::Impl
{
	moodycamel::ConcurrentQueue<CoroutineContext> queue;
	std::atomic<bool> run;
	Impl() :queue(2 << 9), run(true)
	{}
};

dividend::CoroutinePool::CoroutinePool(int coroutine_pool_size, int thread_pool_size)
	: pool_size_(coroutine_pool_size)
	, impl_(std::make_unique<Impl>())
{
	io_context_pool_ptr_ = std::make_shared<IoContextPool>(thread_pool_size);
	io_context_pool_ptr_->start();
}

dividend::CoroutinePool::~CoroutinePool() = default;

void dividend::CoroutinePool::start()
{
	for (auto i = 0; i < pool_size_; ++i)
	{
		auto& ioc = io_context_pool_ptr_->get_io_context();
		boost::asio::spawn(ioc,
			[&ioc, self = shared_from_this(), this](const boost::asio::yield_context& yield)
		{
			CoroutineContext excute_context;
			while (impl_->run)
			{
				auto has_work = impl_->queue.try_dequeue(excute_context);
				if (has_work)
				{
					try
					{
						excute_context(ioc, yield);
					}
					catch (const std::exception& e)
					{
						SPDLOG_ERROR("excute coroutine context failed:{}", e.what());
					}
					catch (...)
					{
						SPDLOG_ERROR("excute_context failed with unkown exception");
					}
				}
				else
					setCoroutineSleep(ioc, yield, 5);
			}
		});
	}
}

void dividend::CoroutinePool::stop()
{
	impl_->run = false;
	io_context_pool_ptr_->stop();
}

void dividend::CoroutinePool::setCoroutineSleep(boost::asio::io_context & ioc,
	const boost::asio::yield_context & yield, uint32_t milliseconds)
{
	boost::asio::steady_timer timer(ioc);
	boost::system::error_code ec;
	timer.expires_from_now(std::chrono::milliseconds(milliseconds), ec);
	if (!ec)
		timer.async_wait(yield[ec]);
	else
		SPDLOG_ERROR("set boost.timer failed:{}", ec.message().c_str());
}

bool dividend::CoroutinePool::addCoroutineTask(const CoroutineContext & excute_context)
{
	return impl_->queue.try_enqueue(excute_context);
}

bool dividend::CoroutinePool::addCoroutineTask(CoroutineContext && excute_context)
{
	return false;
}

bool dividend::CoroutinePool::addCoroutineTask(CoroutineContext && excute_context)
{
	return impl_->queue.try_enqueue(std::forward<CoroutineContext>(excute_context));
}
