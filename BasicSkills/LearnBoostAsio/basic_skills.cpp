
#pragma once

#include "stdafx.h"

#include <iostream>

#include <boost/asio.hpp>

#include <boost/bind.hpp>

#include <boost/thread/thread.hpp>

#include <boost/array.hpp>

#include "basic_skills.h"





//////////////////////////////////////////////////////
//                 Basic Skills                     //
//////////////////////////////////////////////////////


// Timer Example 1 : a synchronous timer

void timer_example_1()
{
    boost::asio::io_context io;

    boost::asio::steady_timer t(io, boost::asio::chrono::seconds(5));
    t.wait();

    std::cout << "Hello, world!" << std::endl;
}




// Timer Example 2 : using a timer asynchronously

void print2(const boost::system::error_code& /*e*/)
{
    std::cout << "Hellow, world!" << std::endl;
}

void timer_example_2()
{
    boost::asio::io_context io;

    boost::asio::steady_timer t(io, boost::asio::chrono::seconds(5));

    // async wait, pass in callback
    t.async_wait(&print2);

    // must call io_conext::run()
    // guarantees callback will be called from the thread that calls io_context::run()
    // run() will return when all work finished (in this case, when timer expires)
    io.run();

    std::cout << "io.run() has returned, end of timer_example_2()" << std::endl;
}




// Timer Example 3 : binding arguments to a handler

// repeating timer
// callback print3 can access the timer object

void print3(const boost::system::error_code& /*e*/,
            boost::asio::steady_timer* t,
            int* count)
{
    if (*count < 5) {

        std::cout << *count << std::endl;
        ++(*count);

        // move the expiry time for th timer along by one second from the previous expiry time
        t->expires_at(t->expiry() + boost::asio::chrono::seconds(1));

        // then start async_wait, use boost::bind() to associate the extra parameters with callback
        t->async_wait(boost::bind(print3,
                                  boost::asio::placeholders::error,
                                  t,
                                  count));
    }
}

void timer_example_3()
{
    boost::asio::io_context io;

    int count = 0;

    boost::asio::steady_timer t(io, boost::asio::chrono::seconds(1));

    t.async_wait(boost::bind(print3,
                             boost::asio::placeholders::error,
                             &t,
                             &count));

    io.run();

    std::cout << "io.run() has returned, end of timer_example_3()" << std::endl;
}




// Timer Example 4 : using a memeber funciton as handler

class Printer {

private:
    boost::asio::steady_timer    timer_;
    int                          count_;

public:
    // constructor takes a ref to oc_context obj, and ues it to initialize member variable timer_
    Printer(boost::asio::io_context& io) :
        timer_(io, boost::asio::chrono::seconds(1)),
        count_(0)
    {
        // start async_wait, use boost::bind() to bind member function as timer_ 's callback
        timer_.async_wait(boost::bind(&Printer::print, this));
    }

    ~Printer()
    {
        std::cout << "[destructor ~Printer()] Final count is " << count_ << std::endl;
    }

    void print()
    {
        if (count_ < 5)
        {
            std::cout << count_ << std::endl;
            ++count_;

            timer_.expires_at(timer_.expiry() + boost::asio::chrono::seconds(1));
            timer_.async_wait(boost::bind(&Printer::print, this));
        }
    }
};

void timer_example_4()
{
    boost::asio::io_context io;
    Printer p(io);
    io.run();
    std::cout << "io.run() has returned, end of timer_example_4()" << std::endl;
}





// Timer Example 5 : synchronizing handlers in multithreaded programs

// use io_context::strand class to synchronize callback in a multithreaded program

// class Printer5 will run tow timers in parallel

class Printer5
{

private:
    boost::asio::io_context::strand     strand_;
    boost::asio::steady_timer           timer1_;
    boost::asio::steady_timer           timer2_;
    int                                 count_;

public:
    Printer5(boost::asio::io_context& io) :
        strand_(io),                                    // initialize strand_
        timer1_(io, boost::asio::chrono::seconds(1)),   // initialize timer1_
        timer2_(io, boost::asio::chrono::seconds(1)),   // initialize timer2_
        count_(0)
    {
        std::cout << "[constructor Printer5()] constructor called" << std::endl;

        // an io_context::strand is an executor that guarantees:
        //     for those handlers that are despatched through the same io_context::strand,
        //     one handler's execution completes before another handler starts,
        //     regardless how many threads are calling io_context::run()

        // initializing asyn operations:
        //     each callback is cound to the same io_context::strand obj,
        //     bind_executor(...) returns a new handler which automatically dispathes its contained handler
        //     through io_context::strand obj;
        //     by binding handlers to the same io_context::strand obj, we ensure that these handlers
        //     cannot execute concurrently (only one at a time)
        timer1_.async_wait(boost::asio::bind_executor(strand_,
                                                      boost::bind(&Printer5::print1,
                                                                  this)));

        timer2_.async_wait(boost::asio::bind_executor(strand_,
                                                      boost::bind(&Printer5::print2,
                                                                  this)));

        std::cout << "[constructor Printer5()] initialized timer1_ and timer2_ with strand_" << std::endl;
    }

    ~Printer5()
    {
        std::cout << "[destructor ~Printer5()] Final count is " << count_ << std::endl;
    }

    // handlers print1() and print2() access shared resources: count_ and std::cout;
    // in multithreaded program, accessing shared resources should be synchronized (using strand)
    void print1()
    {
        if (count_ < 10)
        {
            std::cout << "Timer 1: " << count_ << std::endl;
            ++count_;

            timer1_.expires_at(timer1_.expiry() + boost::asio::chrono::seconds(1));

            timer1_.async_wait(boost::asio::bind_executor(strand_,
                                                          boost::bind(&Printer5::print1,
                                                                      this)));
        }
    }

    void print2()
    {
        if (count_ < 10)
        {
            std::cout << "Timer 2: " << count_ << std::endl;
            ++count_;

            timer2_.expires_at(timer2_.expiry() + boost::asio::chrono::seconds(1));

            timer2_.async_wait(boost::asio::bind_executor(strand_,
                                                          boost::bind(&Printer5::print2,
                                                                      this)));
        }
    }
};

void timer_example_5()
{
    // io_context::run() will be called from two threads: main thread and another thread t
    boost::asio::io_context io;

    Printer5 p(io);

    // call io.run() in thread t
    boost::thread t(boost::bind(&boost::asio::io_context::run, &io));
    std::cout << "[main thread] thread t starts to run, thread t calls io.run()" << std::endl;

    // call io.run() in main thread
    std::cout << "[main thread] main thread also calls io.run()..." << std::endl;
    io.run();
    std::cout << "[main thread] main thread io.run() returned" << std::endl;

    t.join();
    std::cout << "[main thread] t.join() returned" << std::endl;
    // note: these main thread std::cout may race with Printer5 's std::cout printing
}







void learn_basic_skills()
{
    timer_example_1();
    timer_example_2();
    timer_example_3();
    timer_example_4();
    timer_example_5();
}