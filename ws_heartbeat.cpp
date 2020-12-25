// ws_heartbeat.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <boost/beast/core.hpp>
#include <boost/beast/core/ostream.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio.hpp>
#include <cstdlib>
#include <functional>
#include <algorithm>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace std::chrono;
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

#define MESSGE_TO_CLIENT "Hello World"
#define TIME_INTERVAL_MS    20.0

//------------------------------------------------------------------------------
// Echoes back all received WebSocket messages
void ThreadProcSession(tcp::socket& socket) {

    try {
        // Construct the stream by moving in the socket
        websocket::stream<tcp::socket> ws{ std::move(socket) };

        // Accept the websocket handshake
        ws.accept();
        std::cout << "incoming websocket client" << std::endl;
        // Enable text option
        ws.text(true);

        // Create a timer
        boost::asio::io_context io;
        double time_wait = TIME_INTERVAL_MS;

        for (;;) {
            boost::asio::steady_timer t(io, boost::asio::chrono::milliseconds((int)time_wait));
            auto time_begin = system_clock::now();
            t.wait();
            ws.write(boost::asio::buffer(std::string(MESSGE_TO_CLIENT)));
            auto time_end = system_clock::now();
            duration<double> elapsed_duration = time_end - time_begin;
            auto elapsed_milliseconds = elapsed_duration.count();
            time_wait = std::min(TIME_INTERVAL_MS, TIME_INTERVAL_MS - elapsed_milliseconds);
            //std::cout << "wait: " << std::to_string(time_wait) << std::endl;

            // This buffer will hold the incoming message
            //boost::beast::multi_buffer buffer;

            // Read a message
            //ws.read(buffer);

            // Echo the message back
            //ws.text(ws.got_text());
            //ws.write(buffer.data());
            //ws.write(boost::asio::buffer(std::string("Hello from WebSocket Server")));
            //std::cout << boost::beast::buffers_to_string(buffer.data()) << std::endl;
        }

    }
    catch (boost::system::system_error const& se) {
        // This indicates that the session was closed
        if (se.code() != websocket::error::closed)
            std::cerr << "Error: " << se.code().message() << std::endl;
    }
    catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

//------------------------------------------------------------------------------
int main(int argc, char* argv[]) {

    try {
        // Check command line arguments.
        if (argc != 3) {
            std::cerr <<
                "Usage: ws_heartbeat <address> <port>\n" <<
                "Example:\n" <<
                "    ws_heartbeat 0.0.0.0 8080\n";
            return EXIT_FAILURE;
        }
        auto const address = boost::asio::ip::make_address(argv[1]);
        auto const port = static_cast<unsigned short>(std::atoi(argv[2]));

        std::cout << "ws_heartbeat listen on " << address << ":" << std::to_string(port) << std::endl;
        // The io_context is required for all I/O
        boost::asio::io_context ioc{ 1 };

        // The acceptor receives incoming connections
        tcp::acceptor acceptor{ ioc, {address, port} };
        for (;;) {
            // This will receive the new connection
            tcp::socket socket{ ioc };

            // Block until we get a connection
            acceptor.accept(socket);

            // Launch the session, transferring ownership of the socket
            std::thread{ std::bind(
                &ThreadProcSession,
                std::move(socket)) }.detach();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
