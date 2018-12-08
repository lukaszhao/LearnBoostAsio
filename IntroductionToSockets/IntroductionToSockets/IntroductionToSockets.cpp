// IntroductionToSockets.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <string>

#include <boost/array.hpp>
#include <boost/asio.hpp>





////////////////////////////////////////////////////////////
// Example 1 - A synchronous TCP daytime client
////////////////////////////////////////////////////////////

void run_synchronous_tcp_daytime_client()
{
    using boost::asio::ip::tcp;

    std::string serverName;

    std::cout << "server name = ";
    std::cin >> serverName;

    const std::string serviceName = "daytime";

    try {
        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);

        tcp::resolver::results_type endpoints =
            resolver.resolve(serverName, serviceName);

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

        std::cout << "\n\nSelect item: ";

        int selectItem;

        std::cin >> selectItem;

        switch (selectItem) {
        case 0: {
            std::cout << "Are you sure you want to exit? (y/n): ";
            std::cin >> yOrN;
            if (yOrN == "y") {
                return;
            }
        } break;

        case 1: {
            run_synchronous_tcp_daytime_client();
        } break;

        case 2: {

        } break;

        }
    }
}

int main()
{
    menu();
    
    return 0;
}

