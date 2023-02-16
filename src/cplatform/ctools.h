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

//#define LOG_DEBUG(msg) cms::cLog(cmsg::CLOG_DEBUG, msg, __FILE__, __LINE__);
//#define LOG_INFO(msg) cms::cLog(cmsg::CLOG_INFO, msg, __FILE__, __LINE__);
//#define LOG_ERROR(msg) cms::cLog(cmsg::CLOG_ERROR, msg, __FILE__, __LINE__);
//#define LOG_FATAL(msg) cms::cLog(cmsg::CLOG_FATAL, msg, __FILE__, __LINE__);

//#define LOG(level, msg) std::cout << '[' <<  level << ']' << __FILE__ << ':' << __LINE__ << '\n' << msg << '\n';
//#define LOG_INFO(msg) LOG("INFO", msg);
//#define LOG_ERROR(msg) LOG("ERROR", msg);
//#define LOG_DEBUG(msg) LOG("DEBUG", msg);

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





/* fmt
%a 星期几的简写
%A 星期几的全称
%b 月分的简写
%B 月份的全称
%c 标准的日期的时间串
%C 年份的后两位数字
%d 十进制表示的每月的第几天
%D 月/天/年
%e 在两字符域中，十进制表示的每月的第几天
%F 年-月-日
%g 年份的后两位数字，使用基于周的年
%G 年分，使用基于周的年
%h 简写的月份名
%H 24小时制的小时
%I 12小时制的小时
%j 十进制表示的每年的第几天
%m 十进制表示的月份
%M 十时制表示的分钟数
%n 新行符
%p 本地的AM或PM的等价显示
%r 12小时的时间
%R 显示小时和分钟：hh:mm
%S 十进制的秒数
%t 水平制表符
%T 显示时分秒：hh:mm:ss
%u 每周的第几天，星期一为第一天 （值从0到6，星期一为0）
%U 第年的第几周，把星期日做为第一天（值从0到53）
%V 每年的第几周，使用基于周的年
%w 十进制表示的星期几（值从0到6，星期天为0）
%W 每年的第几周，把星期一做为第一天（值从0到53）
%x 标准的日期串
%X 标准的时间串
%y 不带世纪的十进制年份（值从0到99）
%Y 带世纪部分的十制年份
%z，%Z 时区名称，如果不能得到时区名称则返回空字符。
%% 百分号下面的程序则显示当前的完整日期：
*/
CCOME_API std::string cgetTime(int timestamp, std::string fmt = "%F %T");



#endif // !CTOOLS_H

