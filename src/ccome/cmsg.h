#ifndef CMSG_H
#define CMSG_H

#ifdef _WIN32
#ifdef CCOME_EXPORTS
#define CCOME_API __declspec(dllexport)
#else
#define CCOME_API __declspec(dllimport)
#endif
#else
#define CCOME_API
#endif // _WIN32

constexpr int MSG_MAX_SIZE = 1024000;  // 消息最大字节数

enum MsgType {
  MSG_NONE = 0,
  MSG_GET_DIR,            // 客户端请求目录
  MSG_UPLOAD_INFO,        // 客户端请求上传文件
  MSG_DOWNLOAD_INFO,      // 客户端请求下载文件
  MSG_DOWNLOAD_COMPLETE,  // 客户端下载文件成功

  MSG_DIR_LIST,           // 服务端返回目录列表
  MSG_UPLOAD_ACCEPT,      // 服务端准备好接收文件
  MSG_UPLOAD_COMPLETE,    // 服务端接收文件完成
  MSG_DOWNLOAD_ACCEPT,    // 服务端确认文件可以开始下载


  MSG_MAX_TYPE,           // 边界值用于验证类型正确
};

// 消息头
struct CCOME_API CMsgHd {
  MsgType type_;
  int size_ = 0;
};

// 消息内容
// 约定每个消息必须包含内容，否则使用 OK
struct CCOME_API CMsg : public CMsgHd {
  char *data_ = 0;        // 存储消息内容
  int recved_ = 0;        // 已接收的消息字节数
};

#endif
