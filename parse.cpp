#include "parse.hpp"

#include <assert.h>

#include <stdexcept>

Statement* Parser::parse(Lexer& lexer) {
  std::pair<std::string, Token> token = lexer.get_curr_token();

  switch (token.second) {
    case Token::CREATE:
      return parse_create(lexer);
      break;
    case Token::INSERT:
      return parse_insert(lexer);
      break;
    case Token::SELECT:
      return parse_select(lexer);
      break;
    default:
      return nullptr;
      break;
  }
}

CreateStatement* Parser::parse_create(Lexer& lexer) {
  CreateStatement* stmt = new CreateStatement{};
  std::pair<std::string, Token> token;

  // grab table name.
  while (lexer.has_next_token() &&
         (token = lexer.get_curr_token()).second != Token::IDENTIFIER)
    ;
  if (lexer.get_prev_token().second != Token::TABLE) {
    throw std::invalid_argument(
        "Malformed \"create\" query, expected \"table\" before table name.");
  }
  stmt->set_table_name(token.first);
  // skip over "values" keyword and "("
  token = lexer.get_curr_token();
  if (token.second != Token::LPAREN) {
    throw std::invalid_argument(
        "Malformed \"create\" query, expected ( after \"values\".");
  }

  // parse values in the statement.
  while (lexer.has_next_token()) {
    std::vector<std::pair<std::string, Token>> value =
        parse_until(lexer, Token::COMMA, Token::RPAREN);
    if (value.size() != 2) {
      throw std::invalid_argument(
          "Malformed\"create\" query, too many keywords specified.");
    }
    stmt->add_mapping(value[0].first, value[1].first);
  }

  return stmt;
}

InsertStatement* Parser::parse_insert(Lexer& lexer) { return nullptr; }

SelectStatement* Parser::parse_select(Lexer& lexer) { return nullptr; }

std::vector<std::pair<std::string, Token>> Parser::parse_until(
    Lexer& lexer, const Token& t1, const Token& t2) {
  std::vector<std::pair<std::string, Token>> tokens;

  while (lexer.has_next_token()) {
    std::pair<std::string, Token> token = lexer.get_curr_token();
    if (token.second == t1 || token.second == t2) {
      return std::move(tokens);
    }
    tokens.push_back(std::move(token));
  }

  throw std::invalid_argument("Malformed query.");
}
