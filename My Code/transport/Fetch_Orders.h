// Fetch_Orders.h
//this is designed so that the websocket is pluggable and message direction is fixed - designed as a general websockets over TCP Solution
//using templates because it lets the compiler inline call instead of runtime lookup (few nanoseconds difference), also good shape for pluggability/versatility of fetch_orders tool

/*
 * PROCESS:
 * setting up the connection {
     * DNS resolution
     * TCP handshake
     * Initiate SNI handshake
     * Initiate TLS handshake
     * Initiate SSL handshake
     * websocket handshake to send data along the websocket layer for lower latency
     * send timeout control to the websocket layer from tcp for lower latency
 * }
 * starting the loop to keep the connection alive
 * actually sending the data finally
 */

#pragma once

#include <boost/beast/ssl.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;
namespace http = beast::http;

struct FeedConfig {
    std::string host;
    std::string port;
    std::string target;
    std::string subscribe_message;
};

template <typename MessageHandler>
class WebSocketSession : public std::enable_shared_from_this<WebSocketSession<MessageHandler>> { //messageHandler is the parameter for the tunnel, aka. the template you want to use.
public:
    WebSocketSession(net::io_context& ioc, ssl::context& ctx, MessageHandler handler) //shape of the websocket
        : resolver_(net::make_strand(ioc))
        , ws_(net::make_strand(ioc), ctx)
        , handler_(std::move(handler))
    {}

    // host/port: where to connect. target: the WebSocket path (e.g. "/ws/market").
    // subscribe_message: raw JSON string sent immediately after the WebSocket
    // handshake completes -- this is what tells the server which data you want.
    void run(std::string const& host, std::string const& port,
             std::string const& target, std::string const& subscribe_message) {
        host_ = host;
        target_ = target;
        subscribe_message_ = subscribe_message;

        resolver_.async_resolve( //DNS resolution
            host_, port,
            beast::bind_front_handler(&WebSocketSession::on_resolve, this->shared_from_this()));
    }

private:
    tcp::resolver resolver_; //service object to hold the ip from dns loopkup (heap)
    websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string target_;
    std::string subscribe_message_;
    MessageHandler handler_; //the websocket service object (heap)

    websocket::permessage_deflate pmd;
    websocket::response_type handshake_response;



    //TCP connection
    void on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
        if (ec) return fail(ec, "resolve");

        beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(15)); //lowest_layer peels to the tcp layer (lowest before physical). certain features are abstracted on upper levels and can only be accessed on the tcp layer
        beast::get_lowest_layer(ws_).async_connect( //TCP connection
            results, //DNS (list of IPs to try with happy eyes)
            beast::bind_front_handler(&WebSocketSession::on_connect, //the function to run during the loop
                                                this->shared_from_this())); //ptr back to the object (the handler aka. the websocket itself)
    }

    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
        if (ec) return fail(ec, "connect"); //another test case whereby the failure occurs on connection

        // SNI: some servers have different domains and need to clarify what domain to route you to; some TLS servers refuse the handshake without it.
        if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host_.c_str())) {
            beast::error_code ssl_ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
            return fail(ssl_ec, "SNI setup");
        }

        beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(15)); //double check this expiry later
        ws_.next_layer().async_handshake(
            ssl::stream_base::client,
            beast::bind_front_handler(&WebSocketSession::on_ssl_handshake, this->shared_from_this()));
    }

    void on_ssl_handshake(beast::error_code ec) {
        if (ec) return fail(ec, "ssl_handshake");

        // Timeout control passes from the raw TCP layer to the websocket stream itself once we're past the TLS handshake to decrease latency.
        beast::get_lowest_layer(ws_).expires_never();
        pmd.client_enable = true;
        pmd.compLevel = 6;
        ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
        ws_.set_option(pmd);

        // The WebSocket handshake is an HTTP Upgrade request from https to websocket connection
        ws_.async_handshake(
            handshake_response, //test to see if compression worked
            host_, target_,
            beast::bind_front_handler(&WebSocketSession::on_handshake, this->shared_from_this()));
    }

    void on_handshake(beast::error_code ec) {
        if (ec) return fail(ec, "websocket_handshake");

        //test for whether the compression was accepted by server
        auto ext = handshake_response[http::field::sec_websocket_extensions];
        if (ext.find("permessage-deflate") != decltype(ext)::npos) {
            std::cout << "Compression accepted: " << ext << "\n";
        } else {
            std::cout << "Compression NOT accepted (header value: \"" << ext << "\")\n";
        }

        // Now that we're a WebSocket connection, send the subscribe message to tell the server which data we want.
        ws_.async_write(
            net::buffer(subscribe_message_), //subscribe message is the real shape of the req (?)
            beast::bind_front_handler(&WebSocketSession::on_write, this->shared_from_this()));
    }

    void on_write(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);
        if (ec) return fail(ec, "write");

        // Kick off the continuous read loop.
        do_read();
    }

    void do_read() {
        ws_.async_read(
            buffer_,
            beast::bind_front_handler(&WebSocketSession::on_read, this->shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred) { //parameteres mark the shape of the websocket message
        boost::ignore_unused(bytes_transferred);
        if (ec) return fail(ec, "read");

        // Hand the raw message off to whatever handler was supplied.
        // Fully resolved/inlinable at compile time since MessageHandler is a concrete type, not a runtime-erased std::function.
        handler_(beast::buffers_to_string(buffer_.data()));
        buffer_.consume(buffer_.size());

        // queue the next read to make the call continuous
        do_read();
    }

    void fail(beast::error_code ec, char const* what) {
        std::cerr << what << ": " << ec.message() << "\n";
    }
};

// Convenience factory so callers don't have to spell out the template
// argument explicitly -- MessageHandler is deduced from the argument.
// (aka. where the magic happens)
template <typename MessageHandler>
std::shared_ptr<WebSocketSession<MessageHandler>>
make_websocket_session(net::io_context& ioc, ssl::context& ctx, MessageHandler handler) {
    return std::make_shared<WebSocketSession<MessageHandler>>(ioc, ctx, std::move(handler));
}
