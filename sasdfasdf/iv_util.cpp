#include "iv_util.h"

namespace iv{
  inline void staterr(void)
  {
    switch(errno)
    {
    case EBADF:
      perror("Bad file descriptor!");
      break;
    case EIO:
      perror("I/O error!");
      break;
    case EINVAL:
      perror("Invalid parameter: buffer was NULL!");
      break;
    case ENOMEM:
      perror("Not enough memory!");
      break;
    case EACCES:
      perror("Permission denied!");
      break;
    case EBUSY:
      perror("Device or resource busy!");
      break;
    case ENOENT:
      perror("File or path not found.");
      break;
    default:
      // An unrelated error occured 
      perror("Unexpected error!");
    }
  }

  int stat(const string & fname, struct stat *fstat)
  {
    int r;
    if(-1 == (r=::_stat(fname.c_str(), fstat)))
      staterr();
    return r;
  }

  int checkopen(const string & fname, int flag, int pmod)
  {
    errno_t i_err;
    int fh;
    i_err = _sopen_s(&fh, fname.c_str(), flag, _SH_DENYNO, pmod);
    if(i_err){
      switch(i_err){
      case EACCES:
        errexit("Given path is a directory, or file is read-only, but an \
                open-for-writing operation was attempted.");
        break;
      case EEXIST:
        errexit("_O_CREAT and _O_EXCL flags were specified, but filename \
                already exists.");
        break;
      case EINVAL:
        errexit("Invalid oflag or pmode argument. ");
        break;
      case EMFILE:
        errexit("No more file descriptors available (too many open files).");
        break;
      case ENOENT:
        errexit("File or path not found.");
        break;
      default:
        errexit("For more information about these and other return codes, see \
                _doserrno, errno, _sys_errlist, and _sys_nerr.");
      }
    }

    return fh;
  }

  int open_or_exit(const string & fname, int flag, int pmod)
  {
    errno_t i_err;
    int fh;
    i_err = _sopen_s(&fh, fname.c_str(), flag, _SH_DENYNO, pmod);
    if(i_err){
      switch(i_err){
      case EACCES:
        errexit("Given path is a directory, or file is read-only, but an \
                open-for-writing operation was attempted.");
        break;
      case EEXIST:
        errexit("_O_CREAT and _O_EXCL flags were specified, but filename \
                already exists.");
        break;
      case EINVAL:
        errexit("Invalid oflag or pmode argument. ");
        break;
      case EMFILE:
        errexit("No more file descriptors available (too many open files).");
        break;
      case ENOENT:
        errexit("File or path not found.");
        break;
      default:
        errexit("For more information about these and other return codes, see \
                _doserrno, errno, _sys_errlist, and _sys_nerr.");
      }
    }

    return fh;
  }

  void make_path(const string & filename, const char *drive, const char *dir, const char *ext)
  {
    char path_buffer[_MAX_PATH];
    string path;
    size_t pos1=0, pos2=0;

    _makepath_s(path_buffer, drive, dir, filename.c_str(), ext);

    path = path_buffer;
    pos2 = path.find_first_of("\\/", pos1);
    while(string::npos != pos2){
      string dir = path.substr(0, pos2);
      if(dir.size()){
        _mkdir(dir.c_str());
        pos1 = pos2+1;
        if(pos1 < path.size() )
          pos2 = path.find_first_of("\\/", pos1);
        else
          break;
      }
      else{
        pos1 = pos2+1;
        if(pos1 < path.size() )
          pos2 = path.find_first_of("\\/", pos1);
        else
          break;
      }
    }

    return ;
  }

  int write_file(const string & filename, const unsigned char *buff, unsigned len){
    int fh;
    errno_t i_err;
    int num = -1;

    i_err = _sopen_s(&fh, filename.c_str(), _O_BINARY | _O_WRONLY | _O_CREAT | _O_TRUNC,
      _SH_DENYNO, _S_IREAD | _S_IWRITE);
    if(!i_err){
      num = ::_write(fh, buff, len);
      _close(fh);
    }
    else{
      cerr << "It was failed to write the file " << filename << endl;
    }

    return num;
  }

  int writetail(int fd, const void *buff, unsigned int count){
    int cnt=0;

    if(_isatty(fd)){
      ::_lseeki64(fd, _filelengthi64(fd), 0);
      cnt = write(fd, buff, count);
    }
    else
      cerr << "fd `" << fd << "' has not been redirected to a file\n";
    
    return cnt;
  }

  long long writef2f(int fd_dest, int fd_src){
    long long cnt = -1;

    if(_isatty(fd_dest) && _isatty(fd_src)){
      unsigned long long pages;
      unsigned tail;
      const unsigned pagesize = 4096;// 4K per page
      const unsigned addrlen = 12; // log2(4K) 
      char buf[pagesize];

      cnt = ::_filelengthi64(fd_src);
      pages = cnt >> addrlen;
      tail = cnt % pagesize;
      for(unsigned long long i=0; i<pages; i++){
        read(fd_src, buf, pagesize);
        write(fd_dest, buf, pagesize);
      }
      if(tail){
        read(fd_src, buf, tail);
        write(fd_dest, buf, tail);
      }
    }

    return cnt;
  }

  long long filecat(int fd_dest, int fd_src){
    long long cnt = -1;

    if(_isatty(fd_dest) && _isatty(fd_src)){
      unsigned long long pages;
      unsigned tail;
      const unsigned pagesize = 4096;// 4K per page
      const unsigned addlen = 12; // log2(4K) 
      char buf[pagesize];

      cnt = _filelengthi64(fd_src);
      pages = cnt >> 12;
      tail = cnt % pagesize;
      ::_lseeki64(fd_dest, _filelengthi64(fd_dest), 0);
      for(unsigned long long i=0; i<pages; i++){
        ::_read(fd_src, buf, pagesize);
        ::_write(fd_dest, buf, pagesize);
      }
      if(tail){
        ::_read(fd_src, buf, tail);
        ::_write(fd_dest, buf, tail);
      }
    }

    return cnt;
  }

  inline int write(int fd, const void *buff, unsigned count){
    long r;
    if(-1 == (r=::_write(fd, buff, count))){
      switch(errno)
      {
      case EBADF:
        perror("Bad file descriptor!");
        break;
      case ENOSPC:
        perror("No space left on device!");
        break;
      case EINVAL:
        perror("Invalid parameter: buffer was NULL!");
        break;
      default:
        // An unrelated error occured 
        perror("Unexpected error!");
      }
    }
    return r;
  }

  inline int read(int fd, void *buff, unsigned count){
    int r;
    if(-1 == (r=::_read(fd, buff, count))){
      switch(errno)
      {
      case EBADF:
        perror("Bad file descriptor!");
        break;
      case EIO:
        perror("I/O error!");
        break;
      case EINVAL:
        perror("Invalid parameter: buffer was NULL!");
        break;
      case ENOMEM:
        perror("Not enough memory!");
        break;
      case EACCES:
        perror("Permission denied!");
        break;
      case EBUSY:
        perror("Device or resource busy!");
        break;
      default:
        // An unrelated error occured 
        perror("Unexpected error!");
      }
    }

    return r;
  }

  int close(int fd){
    int r;
    if(-1 == (r = ::_close(fd))){
      staterr();
    }
    return r;
  }

  mbstate_t in_cvt_state;
  mbstate_t out_cvt_state;

  wstring s2ws(const std::string& s, const std::locale & sys_loc)
  {
    const char* src_str = s.c_str();
    const size_t BUFFER_SIZE = s.size() + 1;

    wchar_t* intern_buffer = new wchar_t[BUFFER_SIZE];
    wmemset(intern_buffer, 0, BUFFER_SIZE);

    const char* extern_from = src_str;
    const char* extern_from_end = extern_from + s.size();
    const char* extern_from_next = 0;
    wchar_t* intern_to = intern_buffer;
    wchar_t* intern_to_end = intern_to + BUFFER_SIZE;
    wchar_t* intern_to_next = 0;

    typedef std::codecvt<wchar_t, char, mbstate_t> CodecvtFacet;

    CodecvtFacet::result cvt_rst =
      std::use_facet<CodecvtFacet>(sys_loc).in(
      in_cvt_state,
      extern_from, extern_from_end, extern_from_next,
      intern_to, intern_to_end, intern_to_next);
    if (cvt_rst != CodecvtFacet::ok) {
      switch(cvt_rst) {
      case CodecvtFacet::partial:
        std::cerr << "partial";
        break;
      case CodecvtFacet::error:
        std::cerr << "error";
        break;
      case CodecvtFacet::noconv:
        std::cerr << "noconv";
        break;
      default:
        std::cerr << "unknown";
      }
      std::cerr << ", please check in_cvt_state."
        << std::endl;
    }
    std::wstring result = intern_buffer;

    delete []intern_buffer;

    return result;
  }

  string ws2s(const std::wstring& ws, const std::locale & sys_loc)
  {
    const wchar_t* src_wstr = ws.c_str();
    const size_t MAX_UNICODE_BYTES = 4;
    const size_t BUFFER_SIZE =
      ws.size() * MAX_UNICODE_BYTES + 1;

    char* extern_buffer = new char[BUFFER_SIZE];
    memset(extern_buffer, 0, BUFFER_SIZE);

    const wchar_t* intern_from = src_wstr;
    const wchar_t* intern_from_end = intern_from + ws.size();
    const wchar_t* intern_from_next = 0;
    char* extern_to = extern_buffer;
    char* extern_to_end = extern_to + BUFFER_SIZE;
    char* extern_to_next = 0;

    typedef std::codecvt<wchar_t, char, mbstate_t> CodecvtFacet;

    CodecvtFacet::result cvt_rst =
      std::use_facet<CodecvtFacet>(sys_loc).out(
      out_cvt_state,
      intern_from, intern_from_end, intern_from_next,
      extern_to, extern_to_end, extern_to_next);
    if (cvt_rst != CodecvtFacet::ok) {
      switch(cvt_rst) {
      case CodecvtFacet::partial:
        std::cerr << "partial";
        break;
      case CodecvtFacet::error:
        std::cerr << "error";
        break;
      case CodecvtFacet::noconv:
        std::cerr << "noconv";
        break;
      default:
        std::cerr << "unknown";
      }
      std::cerr << ", please check out_cvt_state."
        << std::endl;
    }
    std::string result = extern_buffer;

    delete []extern_buffer;

    return result;
  }
}
