#ifndef IV_UTIL_H_
#define IV_UTIL_H_

#include <wchar.h>
#include <locale>
//#include <codecvt>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <utility>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <errno.h>
#include <share.h>

using std::pair;
using std::make_pair;
using std::string;
using std::vector;
using std::map;
using std::set;
using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::istream;
using std::ios_base;
using std::stringstream;
using std::wstring;
using std::multimap;

namespace iv{
  typedef struct _stat stat_t;

  inline void errexit(const string msg)
  {
    cerr << msg << endl;
    exit (-1);
    return;
  }
  inline void todo(const string msg)
  {
    cerr << msg << endl;
    return ;
  }
  int stat(const string & fname, stat_t *fstat);
  int checkopen(const string & fname, int flag, int pmod=_S_IREAD);
  int open_or_exit(const string & fname, int flag, int pmod=_S_IREAD);
  int close(int fd);
  void make_path(const string & filename, const char *drive=NULL, const char *dir=NULL, const char *ext=NULL);
  int write_file(const string & filename, const unsigned char *buff, unsigned len);
  int writetail(int fd, const void *buff, unsigned count);
  long long writef2f(int fd_dest, int fd_src);
  long long filecat(int fd_dest, int fd_src);
  inline int write(int fd, const void *buff, unsigned count);
  inline int read(int fd, void *buff, unsigned count);
  inline long long lseek(int fd, long long offset, int seek_set=SEEK_SET){
    return _lseeki64(fd, offset, seek_set);
  }
  inline long long tell(int fd) { return _telli64(fd); }
  inline int eof(int fd) { return _eof(fd); }

  //wchar wstring
  std::wstring s2ws(const std::string& s, const std::locale & sys_loc=std::locale(""));
  std::string ws2s(const std::wstring& s, const std::locale & sys_loc=std::locale(""));
}

#endif
