#ifndef EXECUTE
#define EXECUTE

#include <streambuf>

#include "types.hpp"

class db_row_buffer : public std::streambuf {
 public:
  db_row_buffer(char* start, size_t size) {
    this->setg(start, start, start + size);
  }
};

#endif
