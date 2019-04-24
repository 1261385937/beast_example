#pragma once
#ifndef IOCONTEXTPOOL_H
#define IOCONTEXTPOOL_H

#include <memory>

namespace boost::asio
{
	class io_context;
}

namespace dividend
{
	class IoContextPool :public std::enable_shared_from_this<IoContextPool>
	{
	private:
		int io_context_index_;
		struct Impl;
		std::unique_ptr<Impl> impl_;
	public:
		explicit IoContextPool(int pool_size);
		~IoContextPool();
		void start();
		void stop();
		boost::asio::io_context& get_io_context();
	private:
		IoContextPool(const IoContextPool&) = delete;
		IoContextPool& operator=(const IoContextPool&) = delete;
	};
}
#endif