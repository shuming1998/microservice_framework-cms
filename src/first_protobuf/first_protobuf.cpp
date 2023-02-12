#include "cmsghead.pb.h"
#include <iostream>
#include <fstream>
#include <string>

int main() {
  std::cout << "test protobuf\n";
  cmsg::CMsgHead msg1;
  msg1.set_msg_size(1024);
  msg1.set_msg_str("test0001");
  std::cout << "msg1 size = " << msg1.msg_size() << "\n";
  std::cout << "msg1 str = " << msg1.msg_str() << "\n";

  // 序列化到 string
  std::string str1;
  msg1.SerializeToString(&str1);
  std::cout << "str1 size = " << str1.size() << '\n';
  std::cout << str1 << '\n';

  // 序列化到文件
  std::ofstream ofs;
  ofs.open("test.txt", std::ios::binary);
  msg1.SerializePartialToOstream(&ofs);
  ofs.close();

  // 从文件反序列化
  std::ifstream ifs;
  ifs.open("text.txt", std::ios::binary);
  cmsg::CMsgHead msg2;
  std::cout << msg2.ParseFromIstream(&ifs) << '\n';
  std::cout << "msg2 str = " << msg2.msg_str() << '\n'; 

  // 从 string 中反序列化
  msg2.set_msg_str("change msg2 str");
  std::string str2;
  msg2.SerializeToString(&str2);
  std::cout << "str2 size = " << str2.size() << '\n';

  cmsg::CMsgHead msg3;
  msg3.ParseFromArray(str2.data(), str2.size());
  std::cout << "msg3 str=" << msg3.msg_str() << '\n';

  return 0;
}

