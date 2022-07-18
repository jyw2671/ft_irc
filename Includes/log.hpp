#pragma once
#ifndef LOG_HPP
#define LOG_HPP

#include <ctime>
#include <iostream>

namespace log
{
	namespace time
	{
		#define BUFFER_SIZE 32
		static time_t raw;
		static char buffer[BUFFER_SIZE];
	} // namesapce time
	void timestamp();
	std::ostream& print();
	std::ostream& endl(std::ostream& os);
} //namespace log

#endif
