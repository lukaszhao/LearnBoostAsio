#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/array.hpp>

// declare all functions and classes used in basic_skills.cpp
// refer to basic_skills.cpp to learn details


void timer_example_1();

void print2(const boost::system::error_code& /*e*/);

void timer_example_2();

void print3(const boost::system::error_code& /*e*/,
            boost::asio::steady_timer* t,
            int* count);

void timer_example_3();

class Printer;

void timer_example_4();

class Printer5;

void timer_example_5();

void learn_basic_skills();