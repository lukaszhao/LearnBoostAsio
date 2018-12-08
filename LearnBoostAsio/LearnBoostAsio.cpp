// LearnBoostAsio.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>

#include <boost/asio.hpp>


#include <boost/bind.hpp>




// Timer 1 : a synchronous timer

void timer_1()
{
    boost::asio::io_context io;

    boost::asio::steady_timer t(io, boost::asio::chrono::seconds(5));
    t.wait();

    std::cout << "Hello, world!" << std::endl;
}




// Timer 2 : using a timer asynchronously

void print(const boost::system::error_code& /*e*/)
{
    std::cout << "Hellow, world!" << std::endl;
}

void timer_2()
{
    boost::asio::io_context io;

    boost::asio::steady_timer t(io, boost::asio::chrono::seconds(5));
    
    // async wait, pass in callback
    t.async_wait(&print);

    // must call io_conext::run()
    // guarantees callback will be called from the thread that calls io_context::run()
    // run() will return when all work finished (in this case, when timer expires
    io.run();

    std::cout << "io.run() has returned, end of timer_2()" << std::endl;
}




// Timer 3 : binding arguments to a handler

// repeating timer
// callback print2 can access the timer object

void print2(const boost::system::error_code& /*e*/,
            boost::asio::steady_timer* t,
            int* count)
{
    if (*count < 5) {
        
        std::cout << *count << std::endl;
        ++(*count);

        // move the expiry time for th timer along by one second from the previous expiry time
        t->expires_at(t->expiry() + boost::asio::chrono::seconds(1));

        // then start async_wait, use boost::bind() to associate the extra parameters with callback
        t->async_wait(boost::bind(print2,
                      boost::asio::placeholders::error,
                      t,
                      count));
    }
}

void timer_3()
{
    boost::asio::io_context io;

    int count = 0;

    boost::asio::steady_timer t(io, boost::asio::chrono::seconds(1));

    t.async_wait(boost::bind(print2,
                             boost::asio::placeholders::error,
                             &t,
                             &count));

    io.run();

    std::cout << "io.run() has returned, end of timer_3()" << std::endl;
}





void learn_boost_asio()
{
    // timer_1();
    // timer_2();
    timer_3();
}

int main()
{
    learn_boost_asio();
    
    return 0;
}

