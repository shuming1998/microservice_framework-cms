#include "cmsghead.pb.h"
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>
#include <iostream>
#include <fstream>
#include <string>

int main() {
  std::cout << "test reflect protobuf\n";

  // 一
  // 通过 message 对象和字段名称设置和获取内容
  cmsg::CMsgHead msg;
  
  // 对象的描述对象，描述结构体对象的类型
  auto descriptor = msg.GetDescriptor();

  // 反射对象
  auto reflecter = msg.GetReflection();

  // 属性对象
  std::string fName = "msg_size";
  auto sizeField = descriptor->FindFieldByName(fName);
  if (!sizeField) {
    std::cerr << "FindFieldByName " << fName << " failed!\n";
    return -1;
  }

  // 设置属性
  reflecter->SetInt32(&msg, sizeField, 1024);

  // 获取属性的值
  std::cout << fName << "= " << reflecter->GetInt32(msg, sizeField) << '\n';



  // 二
  // 运行时解析 proto 文件
  // 准备文件系统
  google::protobuf::compiler::DiskSourceTree sourceTree;
  // 设置一个字符串，用于路径替换
  sourceTree.MapPath("protobufRoot", "./");

  // 创建动态编译器    (文件系统对象，错误收集器)
  google::protobuf::compiler::Importer importer(&sourceTree, NULL);
  std::string protoFileName = "cmsghead.proto";
  std::string path = "protobufRoot/";
  path += protoFileName;

  // 动态编译 proto 文件，如果已经编译过，直接返回缓冲对象
  auto fileDescriptor = importer.Import(path);
  if (!fileDescriptor) {
    std::cerr << "importer.Inport " << path << " failed!\n";
    return -1;
  }

  // 获取 Message 类型的描述
  auto messDesc = importer.pool()->FindMessageTypeByName("cmsg.CMsgHead");

  // 用消息工厂,创建消息对象
  google::protobuf::DynamicMessageFactory factory;
  // 创建一个类型原型
  auto messageProto = factory.GetPrototype(messDesc);
  // 创建消息
  auto messageTest = messageProto->New();
  // 接下来对消息的操作就同【一】
  {
    // 对象的描述对象，描述结构体对象的类型
    auto descriptor = messageTest->GetDescriptor();

    // 反射对象
    auto reflecter = messageTest->GetReflection();

    // 属性对象
    std::string fName = "msg_str";
    auto strField = descriptor->FindFieldByName(fName);
    if (!strField) {
      std::cerr << "FindFieldByName " << fName << " failed!\n";
      return -1;
    }

    // 设置属性
    reflecter->SetString(messageTest, strField, "test dy proto str");

    // 获取属性的值
    std::cout << "test dy proto" << fName << " = " << reflecter->GetString(*messageTest, strField) << '\n';
  }
  return 0;
}

