#include "cssl.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <iostream>

bool CSSL::accept() {
  if (!ssl_) {
    std::cerr << "ssl_ is nullptr!\n";
    return false;
  }

  // 建立 ssl 连接验证，密钥协商
  int res = SSL_accept(ssl_);
  if (res <= 0) {
    std::cerr << "CSSL::accept() failed!\n";
    ERR_print_errors_fp(stderr);
    return false;
  }

  std::cout << "CSSL::accept() success!\n";
  printCipher();
  return true;
}

bool CSSL::connect() {
  // socket 的 connect 已经完成
  if (!ssl_) {
    std::cerr << "ssl_ is nullptr!\n";
    return false;
  }

  int res = SSL_connect(ssl_);
  if (res <= 0) {
    std::cerr << "CSSL::connect() failed!\n";
    ERR_print_errors_fp(stderr);
    return false;
  }

  std::cout << "CSSL::connect() success!\n";
  printCipher();
  printCert();
  return true;
}

int CSSL::write(const void *data, int dataSize) {
  if (!ssl_) {
    return 0;
  }
  return SSL_write(ssl_, data, dataSize);
}

int CSSL::read(void *buf, int bufSize) {
  if (!ssl_) {
    return 0;
  }
  return SSL_read(ssl_, buf, bufSize);
}

void CSSL::close() {
  if (ssl_) {
    SSL_shutdown(ssl_);
    SSL_free(ssl_);
    ssl_ = nullptr;
  }
}

void CSSL::printCipher() {
  if (!ssl_) {
    return;
  }

  std::cout << "Use Cipher: [" << SSL_get_cipher(ssl_) << "]\n";
}

void CSSL::printCert() {
  if (!ssl_) {
    return;
  }

  // 获取到证书
  auto cert = SSL_get_peer_certificate(ssl_);
  if (!cert) {
    std::cout << "no certificate!\n";
    return;
  }

  char buf[1024] = { 0 };
  auto sName = X509_get_subject_name(cert);
  auto str = X509_NAME_oneline(sName, buf, sizeof(buf));

  if (str) {
    std::cout << "subject: [" << str << "]\n";
  }

  auto issuer = X509_get_issuer_name(cert);
  str = X509_NAME_oneline(sName, buf, sizeof(buf));

  if (str) {
    std::cout << "issuer: [" << str << "]\n";
  }

  // 释放证书
  X509_free(cert);
}