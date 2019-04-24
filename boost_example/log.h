#pragma once
#ifndef LOG_H_
#define LOG_H_

#include "spdlog/spdlog.h"

namespace logging
{
	class SpdLog
	{
	public:
		static void createSpdLogging();
	};
}
#endif
