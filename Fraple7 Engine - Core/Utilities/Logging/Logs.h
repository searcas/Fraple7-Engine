#pragma once
#include <string>
#include <iostream>


class Logs
{
public:
	enum LogLevel
	{
		INFO = 0,
		WARNING,
		API_ERROR,
		GENERIC_ERROR,
		LOGICAL_ERROR,
		FATAL_ERROR, 
	};
private:
	Logs() = delete;
	~Logs() = delete;
public:
	static void Log(std::string fileLocation, std::string function, uint32_t Line, std::string Error, std::string Message, LogLevel lvl = API_ERROR)
	{
		switch (lvl)
		{
		case Logs::INFO:
			std::cout << "INFO: " << std::endl;
			break;
		case Logs::WARNING:
			std::cout << "WARNING!!!: "<< std::endl;
			break;
		case Logs::API_ERROR:
			std::cout << "ERROR!!!: " << std::endl;
			break;
		case Logs::GENERIC_ERROR:
			std::cout << "GENERIC ERROR!!!: " << std::endl;
			break;
		case Logs::LOGICAL_ERROR:
			std::cout << "LOGICAL ERROR!!!: " << std::endl;
			break;
		case Logs::FATAL_ERROR:
			std::cout << "FATAL ERROR!!!: " << std::endl;
			break;
		default:
			break;
		}
		std::cout << "FILE: " << fileLocation << std::endl;
		std::cout << "Function: " << function << std::endl;
		std::cout << "Line: " << Line << std::endl;
		std::cout << "Error Message: " << Error << std::endl;
		std::cout << "Message: " << Message << std::endl;
	}

};

