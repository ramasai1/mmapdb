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
    case Token::DROP:
      return parse_drop(lexer);
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

InsertStatement* Parser::parse_insert(Lexer& lexer) {
  InsertStatement* stmt = new InsertStatement{};
  std::pair<std::string, Token> token;

  while (lexer.has_next_token() &&
         (token = lexer.get_curr_token()).second != Token::IDENTIFIER)
    ;
  if (lexer.get_prev_token().second != Token::INTO) {
    throw std::invalid_argument(
        "Malformed \"insert\" query, expected into before table name.");
  }
  stmt->set_table_name(token.first);
  token = lexer.get_curr_token();
  if (token.second != Token::VALUES) {
    throw std::invalid_argument(
        "Malformed query, expecting values after table name");
  }
  while (lexer.has_next_token() &&
         (token = lexer.get_curr_token()).second != Token::LPAREN)
    ;
  while (lexer.has_next_token()) {
    std::vector<std::pair<std::string, Token>> tokens =
        parse_until(lexer, Token::RPAREN, Token::RPAREN);
    std::vector<std::string> tuple;
    for (unsigned int token_idx = 0; token_idx < tokens.size(); token_idx++) {
      std::pair<std::string, Token>& token = tokens[token_idx];
      if (token_idx % 2 != 0 && token.second != Token::COMMA) {
        throw std::invalid_argument(
            "Malformed query, need commas between values to insert");
      }
      if (token.second != Token::COMMA) {
        tuple.push_back(token.first);
      }
    }
    stmt->add_tuple(std::move(tuple));
    if (lexer.has_next_token()) {
      token = lexer.get_curr_token();
      if (token.second != Token::COMMA) {
        throw std::invalid_argument(
            "Malformed query, need commas separating sets of values");
      }
      token = lexer.get_curr_token();
      if (token.second != Token::LPAREN) {
        throw std::invalid_argument(
            "Malformed query, need parens surrounding values that need to be "
            "entered");
      }
    }
  }

  return stmt;
}

SelectStatement* Parser::parse_select(Lexer& lexer) {
  SelectStatement* stmt = new SelectStatement{};
  std::pair<std::string, Token> token;

  std::vector<std::pair<std::string, Token>> tokens =
      parse_until(lexer, Token::FROM, Token::FROM);
  std::vector<std::string> attributes;
  for (const auto& token : tokens) {
    if (token.second == Token::ALL_ATTRIBUTES) {
      stmt->set_select_all(true);
    }
    if (token.second != Token::COMMA && !stmt->get_select_all()) {
      attributes.push_back(token.first);
    }
  }
  if (!attributes.empty() && stmt->get_select_all()) {
    throw std::invalid_argument(
        "Malformed query: cannot select all and also specify other attributes");
  }
  for (const auto& attribute : attributes) {
    stmt->add_attribute(std::move(attribute));
  }
  token = lexer.get_curr_token();
  if (token.second != Token::IDENTIFIER) {
    throw std::invalid_argument(
        "expected table name at the end of the statement");
  }
  stmt->set_table_name(token.first);
  // Parse the `where...` part if it exists.
  if (lexer.has_next_token()) {
    token = lexer.get_curr_token();
    if (token.second != Token::WHERE) {
      throw std::invalid_argument(
          "Malformed query, expecting where or nothing after table name.");
    }
    token = lexer.get_curr_token();
    if (token.second != Token::IDENTIFIER) {
      throw std::invalid_argument(
          "Malformed query, using restricted keywords in the where query");
    }
    std::string attribute = token.first;
    token = lexer.get_curr_token();
    stmt->set_where_operator(token.second);
    if (!lexer.has_next_token()) {
      throw std::invalid_argument(
          "Malformed query, where condition is not properly specified.");
    }
    token = lexer.get_curr_token();
    stmt->set_where_filter(attribute, token.first);
  }

  return stmt;
}

DropStatement* Parser::parse_drop(Lexer& lexer) {
  DropStatement* stmt = new DropStatement{};
  std::pair<std::string, Token> token;

  token = lexer.get_curr_token();
  if (token.second != Token::TABLE) {
    throw std::invalid_argument(
        "malformed query, expected table after drop keyword");
  }
  token = lexer.get_curr_token();
  if (token.second != Token::IDENTIFIER) {
    throw std::invalid_argument("table name cannot be a reserved keyword");
  }
  stmt->set_table_name(token.first);

  return stmt;
}

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
