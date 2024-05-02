#pragma once

#include <string>
#include <iostream>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/lexical_cast.hpp>
#include <crawler/config.hpp>
#include "resource_loader.hpp"

namespace se {

namespace crawler {

class HttpLoader final : public ResourceLoader {

    using ResourceLoader::ssl_context_;
    using ResourceLoader::context_;

public:
    explicit HttpLoader(boost::asio::io_context& context, bool secure = false) : 
        ResourceLoader(context),
        secure_{ secure },
        fetch_timeout_ms_{ Config::from_service<size_t>("processor", "fetch_resource_timeout_ms") },
        ssl_stream_{ context, ssl_context_ }
    { }

private:
    void fail(boost::beast::error_code ec, char const* what, const content_handler_t& handler) {
        ResourceLoadResults res;
        res.success = false;
        res.error_message = (std::ostringstream{} << what << " : " << ec.to_string()).str();
        context_.post(std::bind(handler, std::move(res)));
    }

    static std::string resolve_name_with_cache(const boost::url& url) {
        return get_ip(DNSCache::instance()[url.host()]);;
    }

    static std::string get_ip(const dns_result_t& ips) {
        const int sz = ips.size();
        if (!sz)
            return "";
        const int num = std::experimental::randint(0, sz - 1);
        return ips[num];
    }

    void fill_request(const boost::url& url) {
        request_.version(11);
        request_.method(boost::beast::http::verb::get);
        request_.target(url.encoded_resource());
        request_.set(boost::beast::http::field::content_type, "text/html");
        request_.set(boost::beast::http::field::accept_charset, "utf-8");
        request_.set(boost::beast::http::field::user_agent, Config::name());
        request_.set(boost::beast::http::field::host, url.encoded_host());
        request_.keep_alive(false);
    }

    void load_resolved(boost::url url, const content_handler_t& handler) override {
        if(secure_ && !SSL_set_tlsext_host_name(ssl_stream_.native_handle(), url.host().data())) {
            boost::beast::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
            fail(ec, "ssl", handler);
            return;
        }
        auto ep = boost::asio::ip::tcp::endpoint(
            boost::asio::ip::address::from_string(resolve_name_with_cache(url)), 
            secure_ ? 443 : 80
        );
        fill_request(url);
        boost::beast::get_lowest_layer(ssl_stream_).async_connect(
            ep, 
            boost::beast::bind_front_handler(&HttpLoader::on_connect, this, handler)
        );
    }

    void on_connect(
        const content_handler_t& handler,
        boost::system::error_code ec
    ) {
        if (ec) {
            fail(ec, "connection", handler);
            return;
        }
        if (secure_) {
            ssl_stream_.async_handshake(
                boost::asio::ssl::stream_base::client,
                boost::beast::bind_front_handler(&HttpLoader::on_handshake, this, handler)
            );
        }
        else {
            on_handshake(handler, ec);
        }
    }

    void on_handshake(        
        const content_handler_t& handler,
        boost::system::error_code ec
    ) {
        if (ec) {
            fail(ec, "handshake", handler);
            return;
        }
        auto& raw_stream = boost::beast::get_lowest_layer(ssl_stream_);
        raw_stream.expires_after(fetch_timeout_ms_);
        if (secure_) {
            boost::beast::http::async_write(
                ssl_stream_, request_,
                boost::beast::bind_front_handler(&HttpLoader::on_request, this, handler)
            );
        }
        else {
            boost::beast::http::async_write(
                raw_stream, request_,
                boost::beast::bind_front_handler(&HttpLoader::on_request, this, handler)
            );
        }
    }

    void on_request(const content_handler_t& handler, const boost::system::error_code& ec, size_t transferred) {
        boost::ignore_unused(transferred);
        if (ec) {
            fail(ec, "request", handler);
            return;
        }
        auto& raw_stream = boost::beast::get_lowest_layer(ssl_stream_);
        raw_stream.expires_after(fetch_timeout_ms_);
        if (secure_) {
            boost::beast::http::async_read(
                ssl_stream_, buffer_, response_,
                boost::beast::bind_front_handler(&HttpLoader::on_response, this, handler)
            );
        }
        else {
            boost::beast::http::async_read(
                raw_stream, buffer_, response_,
                boost::beast::bind_front_handler(&HttpLoader::on_response, this, handler)
            );            
        }
    }

    void on_response(const content_handler_t& handler, boost::system::error_code ec, size_t transferred) {
        boost::ignore_unused(transferred);
        if (ec) {
            fail(ec, "response", handler);
            return;
        }
        if (secure_) {
            ssl_stream_.async_shutdown(
                boost::beast::bind_front_handler(&HttpLoader::on_shutdown, this, handler)
            );
        }
        else {
            on_shutdown(handler, {});
        }
    }

    void on_shutdown(const content_handler_t& handler, boost::beast::error_code ec) {
        if (!secure_) {
            boost::beast::get_lowest_layer(ssl_stream_)
                .socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            return;
        }
        if (ec.category() == boost::asio::error::get_ssl_category() && 
            SSL_R_PROTOCOL_IS_SHUTDOWN == ERR_GET_REASON(ec.value())) {
            boost::beast::get_lowest_layer(ssl_stream_).close();
        }
        ResourceLoadResults res;
        res.success = true;
        res.metadata = boost::lexical_cast<std::string>(response_.base());
        res.content = std::move(response_.body());
        context_.post(std::bind(handler, std::move(res)));
    }

private:
    bool secure_;
    std::chrono::milliseconds fetch_timeout_ms_;
    boost::beast::ssl_stream<boost::beast::tcp_stream> ssl_stream_;
    boost::beast::flat_buffer buffer_;
    boost::beast::http::request<boost::beast::http::string_body> request_;
    boost::beast::http::response<boost::beast::http::string_body> response_;
};

} // namespace crawler

} // namespace se