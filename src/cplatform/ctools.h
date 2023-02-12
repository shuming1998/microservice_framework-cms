#ifndef CTOOLS_H
#define CTOOLS_H

#ifdef _WIN32
#ifdef CCOME_EXPORTS
#define CCOME_API __declspec(dllexport)
#else
#define CCOME_API __declspec(dllimport)
#endif
#else
#define CCOME_API
#endif // _WIN32

#include <string>
#include <string.h>
#include <sstream>
#include <iostream>

#define LOG(level, msg) std::cout << '[' <<  level << ']' << __FILE__ << ':' << __LINE__ << '\n' << msg << '\n';
#define LOG_INFO(msg) LOG("INFO", msg);
#define LOG_ERROR(msg) LOG("ERROR", msg);
#define LOG_DEBUG(msg) LOG("DEBUG", msg);

CCOME_API std::string getDirData(std::string path);

#endif // !CTOOLS_H

