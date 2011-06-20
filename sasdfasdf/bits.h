#ifndef BITS_H_
#include "iv_util.h"

namespace iv{
  class obitstream{
  protected:
    vector <unsigned char> buf;
    unsigned char curbit;
    unsigned char bytecache;

  public:
    obitstream(): curbit(0), bytecache('\0'){
    }

    void freeze(){
      if(curbit & 0x80)
        return ;
      if(curbit)
        buf.push_back(bytecache << (8-curbit));
      curbit |= 0x80;
    }

    void unfreeze(){
      if(curbit & 0x80){
        curbit &= 0x7F;
        if(curbit)
          buf.pop_back();
      }
    }

    unsigned char * getbuf(){
      freeze();
      return &buf[0];
    }

    friend obitstream & operator << (obitstream & obs, bool b_val);

    template <class UT>
    void put(UT u_val, unsigned char n=-1){
      unsigned char cnt = sizeof(u_val)*8;

      unfreeze();
      cnt = (cnt>n) ? n : cnt;
      while(cnt--){
        *this << (u_val & 0x1);
        u_val >>= 1;
      }

      return result;
    }
  };
}

#endif
