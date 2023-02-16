#include "ctools.h"
#include <openssl/md5.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/sha.h>
#ifdef _WIN32
#include <io.h>
#else
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#endif // !_WIN32


CCOME_API std::string getDirData(std::string path) {
  std::string data = "";
#ifdef _WIN32
  // 存储文件信息
  _finddata_t file;
  std::string dirPath = path + "/*.*";
  // 目录上下文
  intptr_t dir = _findfirst(dirPath.c_str(), &file);
  if (dir < 0) {
    return data;
  }
  do {
    if (file.attrib & _A_SUBDIR) {
      continue;
    }
    char buf[1024] = { 0 };
    sprintf(buf, "%s,%u;", file.name, file.size);
    data += buf;
  } while (_findnext(dir, &file) == 0);
#else
  const char *dir = path.c_str();
  DIR *dp = 0;
  struct dirent *entry = 0;
  struct stat statBuf;
  dp = opendir(dir);
  if (!dp) {
    return data;
  }
  chdir(dir);
  char buf[1024] = { 0 };
  while ((entry = readdir(dp)) != NULL) {
    lstat(entry->d_name, &statBuf);
    if (S_ISDIR(statBuf.st_mode)) {
      continue;
    }
    sprintf(buf, "%s,%ld;", entry->d_name, statBuf.st_size);
    data += buf;
  }
#endif // _WIN32
  // 去掉结尾的 ';'
  if (!data.empty()) {
    data = data.substr(0, data.size() - 1);
  }
  return data;
}


CCOME_API int base64Encode(const unsigned char *in, int len, char *outBase64) {
  if (!in || len <= 0 || !outBase64) {
    return 0;
  }
  // 编码内存源，用于存储结果
  auto memBio = BIO_new(BIO_s_mem());
  if (!memBio) {
    return 0;
  }

  // base64 过滤器
  auto b64Bio = BIO_new(BIO_f_base64());
  if (!b64Bio) {
    BIO_free(memBio);
    return 0;
  }

  // 设置 64 字节不加换行符
  BIO_set_flags(b64Bio, BIO_FLAGS_BASE64_NO_NL);

  // 形成 BIO 链表 : b64Bio--memBio
  BIO_push(b64Bio, memBio);

  // 往链表头部写入，base64过滤器处理后转入下一个节点(链尾)
  // 对链表写入会调用编码
  int res = BIO_write(b64Bio, in, len);
  if (res <= 0) {
    // 清理链表
    BIO_free_all(b64Bio);
    return 0;
  }

  // 刷新缓存，写入链表的 mem
  BIO_flush(b64Bio);

  int outSize = 0;
  BUF_MEM *pData = nullptr;
  // 从链表输出源内存 memBio 中读取
  BIO_get_mem_ptr(b64Bio, &pData);
  if (pData) {
    memcpy(outBase64, pData->data, pData->length);
    outSize = pData->length;
  }
  BIO_free_all(b64Bio);
  return outSize;
}

CCOME_API int base64Decode(const char *in, int len, unsigned char *outData) {
  if (!in || len <= 0 || !outData) {
    return 0;
  }
  // 解码内存源，用于密文数据输入
  auto memBio = BIO_new_mem_buf(in, len);
  if (!memBio) {
    return 0;
  }

  // base64 过滤器
  auto b64Bio = BIO_new(BIO_f_base64());
  if (!b64Bio) {
    BIO_free(memBio);
    return 0;
  }

  // 设置 64 字节不加换行符
  BIO_set_flags(b64Bio, BIO_FLAGS_BASE64_NO_NL);

  // 形成 BIO 链表 : b64Bio--memBio
  BIO_push(b64Bio, memBio);

  // 读取解码后的数据
  size_t size = 0;
  BIO_read_ex(b64Bio, outData, len, &size);

  BIO_free_all(b64Bio);
  return size;
}