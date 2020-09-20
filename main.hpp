#ifndef MAIN
#define MAIN

#include "execute.hpp"
#include "parse.hpp"

enum class StatementType { CREATE, INSERT, SELECT, INVALID };

std::map<std::string, StatementType> map_stmt_str_to_type();
std::pair<std::string, int> parse_stmt_name_and_limitidx(const std::string &);
StatementType get_stmt_type_from_str(const std::string &,
                                     std::map<std::string, StatementType> &);

#endif