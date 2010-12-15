/* File ip_filter.h */

#ifndef IP_FILTER_H
#define IP_FILTER_H

//#include "cvsort.h"
#include "dbg.h"
#include "iostream"

using std::cout;
using std::cin;
using std::endl;

/* ip : p3.p2.p1.p0
 * inInt : p3p2p1p0(logic)  p0p1p2p3(Mem)
 * In the memory, inInt will store as p0p1p2p3 on little endian of 32bits/64bits machines.
 */


typedef union ip_union {
  unsigned ipInt;
  struct {
    unsigned p0:8;
    unsigned p1:8;
    unsigned p2:8;
    unsigned p3:8;
  } rip;
  unsigned char part[4];
  void show(void) {
    cout << rip.p3 << rip.p2 << rip.p1 << rip.p0;
  }
} ip_type;


inline unsigned shift_p3_p2(unsigned i)
{
  ip_type tmp;
  tmp.ipInt = i;
  tmp.rip.p3 ^= tmp.rip.p2;
  tmp.rip.p2 ^= tmp.rip.p3;
  tmp.rip.p3 ^= tmp.rip.p2;

  return tmp.ipInt;
}

class ip_filter{
 public:
  unsigned from;
  unsigned to;
  unsigned part;

  /* Is ok if the ip in [from, to) */
  int ok(unsigned ipInt) const
  {
    ip_type tmp;
    tmp.ipInt = ipInt;

    if (tmp.part[part] < from)
      return -1;
    else if (tmp.part[part] < to)
      return 0;
    else
      return 1;
    }
  int ok(ip_type ip) const {
    return ok(ip.ipInt);
  }

  ip_filter(unsigned part, unsigned from, unsigned to)
    {
      if (part < 4) {
	this->part = part;
	if (from <= to) {
	  this->from = from;
	  this->to = to;
	} else {
	  this->from = to;
	  this->to = from;
	}
      } else
	errexit("ip_filter() arg1 should be 0~3, arg2 & arg3 should be 0~255.\n");
    }

  bool set(unsigned part, unsigned from, unsigned to)
  {
    if (part < 4) {
      this->part = part;
      if (from <= to) {
	this->from = from;
	this->to = to;
      } else {
	this->from = to;
	this->to = from;
      }

      return true;
    }

    return false;
  }
};

bool iseq_masked(unsigned i, unsigned j, unsigned mask)
{
  return (i & mask) == (j & mask);
}

bool noless_masked(unsigned i, unsigned j, unsigned mask)
{
  return (i & mask) >= (j & mask);
}

#endif
