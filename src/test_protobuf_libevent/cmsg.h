#ifndef CMSG_H
#define CMSG_H 
#include "cmsgtype.pb.h"
#include <string.h>

//ͷ����Ϣ������ֽ���
#define MAX_MSG_SIZE 8192
class CMsg {
public:
  int size_ = 0;            // ���ݴ�С
  int recvSize_ = 0;        // �ѽ��յ����ݴ�С
  char *data_ = nullptr;    // ��� protobuf �����л�����
  cmsg::CMsgType type_ = cmsg::NONE_DO_NOT_USE;  // ��Ϣ����

  bool alloc(int s) {
    if (s <= 0 || s > MAX_MSG_SIZE)
      return false;

    if (data_) {
      delete data_;
    }
    data_ = new char[s];
    if (!data_) {
      return false;
    }
      
    this->size_ = s;
    this->recvSize_ = 0;

    return true;
  }

  //�ж������Ƿ�������
  bool recved() {
    if (size_ <= 0) {
      return false;
    }
    return recvSize_ == size_;
  }

  void clear() {
    delete data_;
    memset(this, 0, sizeof(CMsg));
  }


};

#endif // !CMSG_H
