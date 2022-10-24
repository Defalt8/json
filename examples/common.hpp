#pragma once
#ifndef COMMON_HPP
#define COMMON_HPP

#include <utility>
#include <iostream>
#include <fstream>
#include <sstream>
#include <json.hpp>
#include <json_serial.hpp>

#ifndef RESOURCE_PATH
#define RESOURCE_PATH "./"
#endif

static inline void print_line_break() { std::cout << "-------------------------------" << std::endl; }

static inline void 
print()
{
	std::cout << std::endl;
} 

template <typename T, typename... Args>
static inline void 
print(T && v, Args &&... args) 
{
	std::cout << v;
	print(std::forward<Args>(args)...);
}

template <typename Array>
static inline void 
print_array(Array const & arr) 
{
	for(auto const & e : arr) 
		std::cout << e << ' '; 
	std::cout << std::endl;
}

#endif // COMMON_HPP
