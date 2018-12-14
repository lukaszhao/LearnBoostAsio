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

            std::cout << "[sync server] DEBUG: sending back response...\n\n";
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

        }
    }
}

int main()
{
    menu();
    
    return 0;
}

