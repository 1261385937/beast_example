#include "log.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/async.h"

void logging::SpdLog::createSpdLogging()
{
	spdlog::init_thread_pool(8192 * 2, 1); // 16k
	auto logger_ = spdlog::create_async_nb<spdlog::sinks::rotating_file_sink_mt>
		("async_rotating_file_logger", "my_async_rotating_log.txt", 1024 * 1024 * 10, 5);
	logger_->flush_on(spdlog::level::warn);
	logger_->set_pattern("[%Y-%m-%d %T.%e][tid:%t %@] [%l] %v");
	spdlog::flush_every(std::chrono::seconds(3));
	spdlog::set_default_logger(logger_);
}