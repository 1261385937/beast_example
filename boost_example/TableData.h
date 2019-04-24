#pragma once
#ifndef DIVIDEND_H_
#define DIVIDEND_H_

#include <unordered_map>
#include <string>
#include <memory>

namespace dividend
{
	class CoroutinePool;
	class TableData
	{
	private:
		struct Impl;
		std::unique_ptr<Impl> impl_;
	public:
		TableData();
		~TableData();
		void convertToJsonstring(std::unordered_map<std::string_view, std::string_view>&&,
			std::string& json_string);
		double getValidNumber(std::string_view value, std::string_view remove_str);
		void parseJson(std::string_view json_need_parse);
		std::size_t getArraySize();
		std::string getArrayKeyValue(int array_index, const std::string& key);
	};
}

#endif // DIVIDEND_H_
