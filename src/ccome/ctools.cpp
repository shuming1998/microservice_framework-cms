#include "ctools.h"
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