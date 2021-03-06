// IntroductionToSockets.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <string>

#include <boost/array.hpp>
#include <boost/asio.hpp>

#include <ctime>





////////////////////////////////////////////////////////////
// Example 1 - A synchronous TCP daytime client
////////////////////////////////////////////////////////////

void run_synchronous_tcp_daytime_client()
{
    using boost::asio::ip::tcp;

    std::string isServiceLocal;
    std::cout << "Is service local bound? (y/n): ";
    std::getline(std::cin, isServiceLocal);

    std::string serverName;
    std::cout << "server name = ";
    std::getline(std::cin, serverName);

    std::string serviceName;
    std::cout << "service name (or port) = ";
    std::getline(std::cin, serviceName);

    try {
        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);

        auto passiveFlag = boost::asio::ip::resolver_base::flags::passive;

        tcp::resolver::results_type endpoints;
        
        if (isServiceLocal == "y") {
            endpoints = resolver.resolve(serverName,
                                         serviceName,
                                         passiveFlag);
        }
        else {
            endpoints = resolver.resolve(serverName,
                                         serviceName);
        }

        // now we create and connect the socket
        // the list of endpoints obtained my contain both IPv4 and PIv6 endpoints
        //     so we need to try each of them until we find one that works
        //     (this keeps client program independent of a specific IP version),
        //     and boost::asio::connect(...) does this for us

        tcp::socket socket(io_context);

        boost::asio::connect(socket, endpoints);

        // the connection is open, now we read the response from the daytime service

        for (;;) {

            // use boost::array to hold the received data
            boost::array<char, 128> buf;
            boost::system::error_code errorCode;

            size_t len = socket.read_some(boost::asio::buffer(buf),
                                          errorCode);

            if (errorCode == boost::asio::error::eof) {
                // connection closed cleanly by peer, now break for loop
                break;
            }
            else if (errorCode) {
                // other error happened
                throw boost::system::system_error(errorCode);
            }

            // print received data
            std::cout.write(buf.data(), len);
        }
    }
    catch (std::exception& e) {
        std::cout << "Caught Exception: " << e.what() << std::endl;
    }
}




////////////////////////////////////////////////////////////
// Example 2 - A synchronous TCP daytime server
////////////////////////////////////////////////////////////

std::string make_daytime_string()
{
    std::time_t now = std::time(0);
    char str[26];
    ctime_s(str, sizeof str, &now);
    return str;
}


void run_synchronous_tcp_daytime_server()
{
    using boost::asio::ip::tcp;

    try {
        boost::asio::io_context io_context;

        int portNumber = 13;
        std::string portStr;
        std::cout << "port number (daytime service port is usually 13) = ";
        std::getline(std::cin, portStr);

        try {
            portNumber = std::stoi(portStr);
        }
        catch (...) {
            std::cout << "Invalide port entry! use port 13\n";
            portNumber = 13;
        }

        tcp::acceptor acceptor(io_context,
                               tcp::endpoint(tcp::v4(), portNumber));

        // handle one connection at a time
        for (;;) {
            // create a socket that will represent the connection to client, and then wait for a connection
            std::cout << "[sync server] DEBUG: create a socket that will represent the connection to client, and then wait for a connection...\n";
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            // to here, a client is accessing our service
            std::cout << "[sync server] DEBUG: a client is accessing our service!\n";
            std::string message = make_daytime_string();

            boost::system::error_code ignored_error_code;

            std::cout << "[sync server] DEBUG: sending back response, message = "
                      << message << "\n\n";

            boost::asio::write(socket,
                               boost::asio::buffer(message),
                               ignored_error_code);
        }
    }
    catch (std::exception& e) {
        std::cout << "Caught Exception: " << e.what() << std::endl;
    }
}




////////////////////////////////////////////////////////////
// Example 3 - An asynchronous TCP daytime server
////////////////////////////////////////////////////////////

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>


using boost::asio::ip::tcp;

class TcpConnection : public boost::enable_shared_from_this<TcpConnection> {

private:

    tcp::socket      socket_;
    std::string      message_;

    TcpConnection(boost::asio::io_context& io_context) :
        socket_(io_context)
    {
        std::cout << "[TcpConnection] DEBUG: TcpConnection private constructor called, initialize socket_ with io_context\n";
    }

    void handle_write(const boost::system::error_code& /*error*/,
                      size_t /*bytes_transferred*/)
    {
        std::cout << "[TcpConnection] DEBUG: handle_write(...) called\n";
    }

public:

    typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;

    static TcpConnectionPtr create(boost::asio::io_context& io_context)
    {
        std::cout << "[TcpConnection] DEBUG: static function create(...) called, return a shared pointer of TcpConnection\n";
        return TcpConnectionPtr(new TcpConnection(io_context));
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
        std::cout << "[TcpConnection] DEBUG: start() called\n";
        
        message_ = make_daytime_string();

        std::cout << "    [TcpConnection] DEBUG: calling async_write(...), using TcpConnection::handle_write as callback\n";
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(message_),
                                 boost::bind(&TcpConnection::handle_write,
                                             shared_from_this(),
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));

        std::cout << "    [TcpConnection] DEBUG: async_write(...) returned\n";
    }

    virtual ~TcpConnection() {
        std::cout << "[TcpConnection] DEBUG: ~TcpConnection() destructor called\n\n\n";
    }
};


class TcpServer {
private:
    tcp::acceptor    acceptor_;

    void handle_accept(TcpConnection::TcpConnectionPtr new_connection,
                       const boost::system::error_code& errorCode)
    {
        std::cout << "[TcpServer] DEBUG: handle_accept(...) called\n";
        
        if (!errorCode)
        {
            std::cout << "    [TcpServer] DEBUG: handler_accept(...) invokes new_connection->start()\n";
            new_connection->start();
        }
        std::cout << "    [TcpServer] DEBUG: handler_accept(...) calls start_accept()\n";
        start_accept();

        std::cout << "    [TcpServer] DEBUG: start_accept() returned\n";
    }

    void start_accept()
    {
        std::cout << "[TcpServer] DEBUG: start_accept() called, create a new connection\n";
        
        TcpConnection::TcpConnectionPtr new_connection =
            TcpConnection::create(acceptor_.get_executor().context());

        std::cout << "    [TcpServer] DEBUG: start_accept() invoke acceptor_.async_accept(...), use TcpServer::handle_accept as callback\n";
        acceptor_.async_accept(new_connection->socket(),
                               boost::bind(&TcpServer::handle_accept,
                                           this,
                                           new_connection,
                                           boost::asio::placeholders::error));

        std::cout << "    [TcpServer] DEBUG: acceptor_.async_accept(...), returned\n";
    }

public:
    TcpServer(boost::asio::io_context& io_context):
        acceptor_(io_context, tcp::endpoint(tcp::v4(), 13))
    {
        std::cout << "[TcpServer] DEBUG: TcpServer constructor called, calling start_accept()\n";
        start_accept();
    }

    ~TcpServer()
    {
        std::cout << "[TcpServer] DEBUG: TcpServer destructor called\n\n\n";
    }
};

void run_asynchronous_tcp_daytime_server()
{
    try {

        boost::asio::io_context io_context;
        
        TcpServer server(io_context);
        
        io_context.run();

    } catch (std::exception& e) {
        std::cout << "[async server] caught exception: " << e.what() << std::endl;
    }
}








////////////////////////////////////////////////////////////
// menu
////////////////////////////////////////////////////////////

void menu()
{
    std::string yOrN = "n";

    while (yOrN != "y") {
        std::cout << "\n\n\n==========================================";
        std::cout << "\n0. Exit";
        std::cout << "\n1. Run synchronous TCP daytime client";
        std::cout << "\n2. Get current daytime";
        std::cout << "\n3. Run synchronous TCP daytime server";
        std::cout << "\n4. Run asynchronous TCP daytime server";

        std::cout << "\n\nSelect item: ";

        int selectItem = -1;
        std::string selectItemStr;
        std::getline(std::cin, selectItemStr);

        try {
            selectItem = std::stoi(selectItemStr);
        }
        catch (...) {
            std::cout << "Invalid entry!\n";
        }

        switch (selectItem) {
        case 0: {
            std::cout << "Are you sure you want to exit? (y/n): ";
            std::getline(std::cin, yOrN);
            if (yOrN == "y") {
                return;
            }
        } break;

        case 1: {
            run_synchronous_tcp_daytime_client();
        } break;

        case 2: {
            std::cout << "daytime string is: " << make_daytime_string() << std::endl;
        } break;

        case 3: {
            run_synchronous_tcp_daytime_server();
        } break;

        case 4: {
            run_asynchronous_tcp_daytime_server();
        } break;

        }
    }
}

int main()
{
    menu();
    
    return 0;
}

