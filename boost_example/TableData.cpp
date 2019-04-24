#include "TableData.h"
#include "nlohmann/json.hpp"
#include "log.h"

struct dividend::TableData::Impl
{
	nlohmann::json table_json;
};

dividend::TableData::TableData()
	:impl_(std::make_unique<Impl>())
{
}

dividend::TableData::~TableData() = default;

void dividend::TableData::convertToJsonstring(std::unordered_map<std::string_view, std::string_view>&& map,
	std::string & json_string)
{
	nlohmann::json table_json(map);
	json_string = table_json.dump();
}

double dividend::TableData::getValidNumber(std::string_view value, std::string_view remove_str)
{
	auto pos1 = value.find_first_of(R"(")");
	auto pos2 = value.find_last_of(R"(")");
	auto str = value.substr(pos1 + 1, pos2 - pos1 - 1);
	auto pos3 = str.find(remove_str);
	auto num_str = str.substr(0, pos3);
	return std::atof(num_str.data());
}

void dividend::TableData::parseJson(std::string_view json_need_parse)
{
	impl_->table_json = nlohmann::json::parse(json_need_parse);
}

std::size_t dividend::TableData::getArraySize()
{
	return impl_->table_json.at("rows").size();
}

std::string dividend::TableData::getArrayKeyValue(int array_index, const std::string& key)
{
	return impl_->table_json.at("rows").at(array_index).at(key).dump();
}