#ifndef CTOOLS_H
#define CTOOLS_H
#include <string>
#ifdef _WIN32
#ifdef CCOME_EXPORTS
#define CCOME_API __declspec(dllexport)
#else
#define CCOME_API __declspec(dllimport)
#endif
#else
#define CCOME_API
#endif // _WIN32

CCOME_API std::string getDirData(std::string path);


#endif // !CTOOLS_H

