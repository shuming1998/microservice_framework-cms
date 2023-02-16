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

// 返回输出数据的大小
CCOME_API int base64Encode(const unsigned char *in, int len, char *outBase64);
CCOME_API int base64Decode(const char *in, int len, unsigned char *outData);

///生成md5 128bit(16字节) 
///@para in_data 输入数据
///@para in_data_size 输入数据字节数
///@para out_md 输出的MD5数据 （16字节）
CCOME_API unsigned char *CMD5(const unsigned char *inData, unsigned long inDataSize, unsigned char *outMd);

///生成md5Base64  (24字节) 再经过base64转化为字符串
///@para inData 输入数据
///@para inDataSize 输入数据字节数
///@return  输出的MD5 base64 数据 （24字节）
CCOME_API std::string CMD5Base64(const unsigned char *inData, unsigned long inDataSize);









#endif // !CTOOLS_H

