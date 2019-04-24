#pragma once
#ifndef HTTPCLIENT_H_
#define HTTPCLIENT_H_

#include <iosfwd>
#include <atomic>
#include <memory>
#include <string>

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

namespace boost::beast::http
{
	template<bool isRequest, class Body, class Fields>
	struct message;

	template<class Allocator>
	class basic_fields;
	using fields = basic_fields<std::allocator<char>>;

	template<class Body, class Fields = fields>
	using request = message<true, Body, Fields>;

	template<class Body, class Fields = fields>
	using response = message<false, Body, Fields>;

	template<class CharT, class Traits, class Allocator>
	struct basic_string_body;
	using string_body = basic_string_body<char, std::char_traits<char>, std::allocator<char>>;
}
namespace http = boost::beast::http;


namespace dividend
{
	class CoroutinePool;
	class HttpClient
	{
	private:
		std::atomic<bool> run_;
		boost::asio::io_context& ioc_;
		boost::asio::yield_context& yield_;
		std::shared_ptr<CoroutinePool> coro_pool_ptr_;
		std::string ip_;
		std::string port_;
		struct Impl;
		std::unique_ptr<Impl> impl_;
	public:
		HttpClient(boost::asio::io_context& ioc,
			boost::asio::yield_context& yield, std::shared_ptr<CoroutinePool> coro_pool_ptr);
		~HttpClient();
		bool sendHttpRequest(const http::request<http::string_body>& req,
			http::response<http::string_body>& res);
		void getResponseUntilOk(const http::request<http::string_body>& req,
			http::response<http::string_body>& res);
		void closeConnection();
		bool connect(std::string_view ip, std::string_view port);
		void reconnect();
	};
}

#endif