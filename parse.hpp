#ifndef PARSE
#define PARSE

#include "lexer.hpp"
#include "types.hpp"

class Parser {
 public:
  ~Parser() = default;

  Statement* parse(Lexer&);
  CreateStatement* parse_create(Lexer&);
  InsertStatement* parse_insert(Lexer&);
  SelectStatement* parse_select(Lexer&);
  DropStatement* parse_drop(Lexer&);
  std::vector<std::pair<std::string, Token>> parse_until(Lexer&, const Token&,
                                                         const Token&);
};

#endif
