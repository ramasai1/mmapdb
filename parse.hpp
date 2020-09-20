#ifndef PARSE
#define PARSE

#include "types.hpp"

CreateStatement parse_create(const std::string &);
InsertStatement parse_insert(const std::string &);
SelectStatement parse_select(const std::string &);

#endif