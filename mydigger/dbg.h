#ifndef DBG_H
#define DBG_H

#include <iostream>

#define todo(){	std::cout << "Sth need to do here." << std::endl; }while(0)

inline void tellerr (char *STR)
{
  std::cout << STR << std::endl;
}

inline void errexit (char *STR)
{
  std::cout << STR << std::endl;
  exit (-1);
}
#endif
