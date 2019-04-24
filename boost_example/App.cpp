
#include "boost/beast/http.hpp"
#include "boost/beast/version.hpp"
#include "CoroutinePool.h"
#include "log.h"
#include "HttpClient.h"
#include "TableData.h"

//static std::string gbkToUtf8(const std::string& strGBK)
//{
//	int n = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
//	wchar_t * str1 = new wchar_t[n];
//	MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, str1, n);
//	n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
//	char * str2 = new char[n];
//	WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
//	std::string strOutUTF8 = str2;
//	delete[]str1;
//	delete[]str2;
//	return strOutUTF8;
//}
//
//static std::string utf8ToGbk(const std::string& utf8)
//{
//	int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
//	wchar_t* wstr = new wchar_t[len + 1];
//	MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wstr, len);
//	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
//	char* str = new char[len + 1];
//	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
//	std::string strOutGbk = str;
//	delete[] str;
//	delete[] wstr;
//	return strOutGbk;
//}


int main()
{
	logging::SpdLog::createSpdLogging();
	SPDLOG_INFO("App start");

	auto coro_pool_ptr = std::make_shared<dividend::CoroutinePool>(1, 1);
	coro_pool_ptr->start();

	coro_pool_ptr->addCoroutineTask(
		[coro_pool_ptr](boost::asio::io_context& ioc, const boost::asio::yield_context& yield)
	{
		auto http_client_ptr = std::make_unique<dividend::HttpClient>(ioc,
			const_cast<boost::asio::yield_context&>(yield), coro_pool_ptr);
		http_client_ptr->connect("139.198.121.87", "8088");
		http::request<http::string_body> req{ http::verb::post, "/gettablerows",11 };
		auto table_date_ptr = std::make_unique<dividend::TableData>();
		auto setTableRequest = [&req, &table_date_ptr](std::string_view code,
			std::string_view scope, std::string_view table)
		{
			req.set(http::field::host, "139.198.121.87");
			req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
			std::unordered_map <std::string_view, std::string_view> json
			{
				{"json","true"},{"code",code},
				{"scope",scope},{"table",table},
				{"lower_bound","0"},{"upper_bound","-1"}
			};
			std::string json_str;
			table_date_ptr->convertToJsonstring(std::move(json), json_str);
			req.body() = json_str;
			req.set(http::field::content_type, "application/json");
			req.set(http::field::content_length, json_str.length());
			req.prepare_payload();
		};

		{
			http::response<http::string_body> res;
			setTableRequest("zealbancor11", "zealbancor11", "player");
			http_client_ptr->getResponseUntilOk(req, res);
			printf("%s\n", res.body().c_str());
			table_date_ptr->parseJson(res.body());
			auto size = table_date_ptr->getArraySize();
			for (auto i = 0; i < size; ++i)
			{
				printf("%s\n", table_date_ptr->getArrayKeyValue(i, "player").c_str());
				printf("%s\n", table_date_ptr->getArrayKeyValue(i, "balance").c_str());
			}
			printf("\n");
		}
		{
			http::response<http::string_body> res;
			setTableRequest("zealbounce11", "zealbounce11", "pool");
			http_client_ptr->getResponseUntilOk(req, res);
			printf("%s\n", res.body().c_str());
			table_date_ptr->parseJson(res.body());
			printf("%s\n", table_date_ptr->getArrayKeyValue(0, "history_pool").c_str());
			printf("\n");
		}
		{
			http::response<http::string_body> res;
			setTableRequest("zealbancor11", "EOZ", "status");
			http_client_ptr->getResponseUntilOk(req, res);
			table_date_ptr->parseJson(res.body());
			printf("%s\n", table_date_ptr->getArrayKeyValue(0, "eoz_circulation").c_str());
			printf("%s\n", table_date_ptr->getArrayKeyValue(0, "eos_balance").c_str());
			printf("%s\n", table_date_ptr->getArrayKeyValue(0, "connector_weight").c_str());
			printf("\n");
		}
		http_client_ptr->closeConnection();
	});
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	coro_pool_ptr->stop();
	SPDLOG_INFO("App exit");
	return 0;
}