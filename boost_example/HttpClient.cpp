#include "HttpClient.h"
#include "boost/asio/ip/tcp.hpp"
#include "boost/beast/http.hpp"
#include "boost/beast/core.hpp"
#include "boost/asio/connect.hpp"
#include "boost/asio/spawn.hpp"
#include "log.h"
#include "CoroutinePool.h"

struct dividend::HttpClient::Impl
{
	boost::asio::ip::tcp::socket socket;
	Impl(boost::asio::io_context & ioc) :socket{ ioc } {}
};

dividend::HttpClient::HttpClient(boost::asio::io_context & ioc,
	boost::asio::yield_context & yield, std::shared_ptr<CoroutinePool> coro_pool_ptr)
	:run_(true),
	ioc_(ioc),
	yield_(yield),
	coro_pool_ptr_(coro_pool_ptr),
	impl_(std::make_unique<Impl>(ioc))
{
}

dividend::HttpClient::~HttpClient() = default;

bool dividend::HttpClient::sendHttpRequest(const http::request<http::string_body>& req,
	http::response<http::string_body>& res)
{
	boost::system::error_code ec;
	http::async_write(impl_->socket, req, yield_[ec]);
	if (ec)
	{
		SPDLOG_ERROR("async_write error: {}", ec.message());
		return false;
	}
	boost::beast::flat_buffer b; // this buffer is used for reading and must be persisted
	http::async_read(impl_->socket, b, res, yield_[ec]);
	if (ec)
	{
		SPDLOG_ERROR("async_read error: {}", ec.message());
		return false;
	}
	return true;
}

void dividend::HttpClient::getResponseUntilOk(const http::request<http::string_body>& req,
	http::response<http::string_body>& res)
{
	while (run_)
	{
		auto ok = sendHttpRequest(req, res);
		if (ok)
			break;
		else
			reconnect();
	}
}

void dividend::HttpClient::closeConnection()
{
	boost::system::error_code ec;
	impl_->socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec); //gracefully close the socket
	if (ec && ec != boost::system::errc::not_connected) // not_connected happens sometimes, so don't bother reporting it.
		SPDLOG_WARN("shutdown_both error: {}", ec.message());
}

bool dividend::HttpClient::connect(std::string_view ip, std::string_view port)
{
	ip_ = ip;
	port_ = port;
	boost::asio::ip::tcp::resolver resolver{ ioc_ };
	auto const results = resolver.async_resolve(ip_, port, yield_);
	boost::system::error_code ec;
	while (run_)
	{
		boost::asio::async_connect(impl_->socket, results.begin(), results.end(), yield_[ec]);
		if (ec)
		{
			SPDLOG_ERROR("async_connect error: {}", ec.message());
			coro_pool_ptr_->setCoroutineSleep(ioc_, yield_, 10000);
			continue;
		}
		return true;
	}
	return false;
}

void dividend::HttpClient::reconnect()
{
	closeConnection();
	connect(ip_, port_);
}


