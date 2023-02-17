#ifndef CLOG_DAO_H
#define CLOG_DAO_H

namespace cmsg {
  class CAddLogReq;
}

class CLogDAO {
public:
  ~CLogDAO() {}
  static CLogDAO *get() {
    static CLogDAO log;
    return &log;
  }

  bool init();
  bool install();
  bool addLog(const cmsg::CAddLogReq *req);

private:
  CLogDAO() {}

};

#endif // !CLOG_DAO_H

