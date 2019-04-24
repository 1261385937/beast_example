#include "IoContextPool.h"
#include <vector>
#include <thread>
#include "boost/asio/io_context.hpp"
#include "log.h"

struct dividend::IoContextPool::Impl
{
	std::vector<std::unique_ptr<std::thread>> thread_vec;
	std::vector<std::unique_ptr<boost::asio::io_context>> io_context_vec;
	std::vector<std::unique_ptr<boost::asio::io_context::work>> work_vec; //keep io_context.run() alive
};

dividend::IoContextPool::IoContextPool(int pool_size)
	: io_context_index_(0)
	, impl_(std::make_unique<Impl>())
{
	for (int i = 0; i < pool_size; ++i)
	{
		auto io_context_ptr = std::make_unique<boost::asio::io_context>();
		auto work_ptr = std::make_unique<boost::asio::io_context::work>(*io_context_ptr);
		impl_->io_context_vec.emplace_back(std::move(io_context_ptr));
		impl_->work_vec.emplace_back(std::move(work_ptr));
	}
}

dividend::IoContextPool::~IoContextPool() = default;

void dividend::IoContextPool::start()
{
	auto size = impl_->io_context_vec.size();
	for (auto i = 0; i < static_cast<int>(size); ++i)
	{
		auto thread_ptr = std::make_unique<std::thread>([self = shared_from_this(), this, i]()
		{
			try
			{
				impl_->io_context_vec[i]->run();
			}
			catch (std::exception& e)
			{
				SPDLOG_ERROR("io_service thread exit with {}", e.what());
			}
			catch (...)
			{
				SPDLOG_ERROR("io_service thread exit with unknown exception\n");
			}
		});
		impl_->thread_vec.emplace_back(std::move(thread_ptr));
	}
}

void dividend::IoContextPool::stop()
{
	auto work_vec_size = impl_->work_vec.size();
	for (auto i = 0; i < static_cast<int>(work_vec_size); ++i)
	{
		//work_vec_[i]->~work();
		impl_->work_vec[i].reset();
	}

	auto size = impl_->thread_vec.size();
	for (int i = 0; i < static_cast<int>(size); ++i)
	{
		if (impl_->thread_vec[i]->joinable())
			impl_->thread_vec[i]->join();
	}
}

boost::asio::io_context& dividend::IoContextPool::get_io_context()
{
	auto& io_context = *(impl_->io_context_vec[io_context_index_]);
	++io_context_index_;
	if (io_context_index_ == static_cast<int>(impl_->io_context_vec.size()))
		io_context_index_ = 0;
	return io_context;
}
