#include "cdata.h"
#include <fstream>
#include <iostream>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <iconv.h>
#endif


namespace cmysql {

#ifndef _WIN32
  static size_t convert(char *fromChar, char *toChar, char *in, size_t inLen, char *out, size_t outLen) {
    // 转化上下文
    iconv_t cd;
    cd = iconv_open(toChar, fromChar);
    if (cd == 0) {
      return -1;
    }
    memset(out, 0, outLen);
    char **pin = &in;
    char **pout = &out;
    // 返回转换字节数的数量，但转 gbk 时经常不正确   >= 0 就成功，失败返回复数
    size_t res = iconv(cd, pin, &inLen, pout, &outLen);

    if (cd != 0) {
      iconv_close(cd);
    }
    return res;
  }
#endif

CData::CData(const int *d) {
  this->type = CMYSQL_TYPE_LONG;
  this->data = (const char *)d;
  this->size = sizeof(int);
}


CData::CData(const char *data) {
  this->type = CMYSQL_TYPE_STRING;

  if (!data) {
    return;
  }
  
  this->data = data;
  this->size = strlen(data);
}

bool CData::loadFile(const char *fileName) {
  if (!fileName) {
    return false;
  }

  std::fstream in(fileName, std::ios::in | std::ios::binary);
  if (!in.is_open()) {
    std::cerr << "loadFile: " << fileName << "failed!\n";
    return false;
  }

  // 获取文件大小
  in.seekg(0, std::ios::end);
  size = in.tellg();
  in.seekg(0, std::ios::beg);
  if (size <= 0) {
    return false;
  }

  // 申请空间大小
  data = new char[size];
  int readed = 0;
  while (!in.eof()) {
    in.read((char *)data + readed, size - readed);
    if (in.gcount() > 0) {
      readed += in.gcount();
    } else {
      break;
    }
  }
  in.close();
  this->type = CMYSQL_TYPE_BLOB;
  return true;
}

bool CData::saveFile(const char *fileName) {
  if (!data || size <= 0) {
    return false;
  }

  std::fstream out(fileName, std::ios::out | std::ios::binary);
  if (!out.is_open()) {
    std::cerr << "saveFile [" << fileName << "] failed!\n";
    return false;
  }

  out.write(data, size);
  out.close();
  return true;
}

void CData::drop() {
  delete data;
  data = nullptr;
}

std::string CData::utf8ConvGbk(ConvType direction) {
  std::string res = "";
  // 1 utf8/gbk 先要转为 unicode(win:utf16)
  // 1.1 统计转换后的字节数
#ifdef _WIN32
  int format_1 = -1;
  int format_2 = -1;
  if (direction == UTF82GBK) {
    format_1 = CP_UTF8;
    format_2 = CP_ACP;
  }
  else {
    format_1 = CP_ACP;
    format_2 = CP_UTF8;
  }
  int len = MultiByteToWideChar(format_1,  // 转换的格式
                                0,        // 默认转换方式
                                data,     // 输入的字节
                                -1,       // 输入的字符串大小， -1代表寻找 \0
                                0,        // 输出
                                0);       // 输出的空间大小
  if (len <= 0) {
    return res;
  }
  std::wstring udata;
  udata.resize(len);
  MultiByteToWideChar(format_1, 0, data, -1, (wchar_t *)udata.data(), len);

  // 2 unicode 转 GBK
  len = WideCharToMultiByte(format_2, 0, (wchar_t *)udata.data(), -1, 0, 0,
    0,    // 失败默认替代字符
    0);   // 是否使用默认替代
  if (len <= 0) {
    return res;
  }
  res.resize(len);
  WideCharToMultiByte(format_2, 0, (wchar_t *)udata.data(), -1, (char *)res.data(), len, 0, 0);
#else
  res.resize(1024);
  int inLen = strlen(data);
  if (direction == UTF82GBK) {
    convert((char *)"utf-8", (char *)"gbk", (char *)data, inLen, (char *)res.data(), res.size());
  }
  else {
    convert((char *)"gbk", (char *)"utf-8", (char *)data, inLen, (char *)res.data(), res.size());
  }

  int outLen = strlen(res.data());
  res.resize(outLen);

#endif
  return res;
}




} // namespace cmysql