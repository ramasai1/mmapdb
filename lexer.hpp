#ifndef LEXER
#define LEXER

#include <iterator>
#include <regex>
#include <string>
#include <vector>

#include "types.hpp"

class Lexer {
  std::vector<std::pair<std::string, Token>> REGEX_TO_TOKENS = {
      {"select", Token::SELECT},
      {"create", Token::CREATE},
      {"insert", Token::INSERT},
      {"values", Token::VALUES},
      {"from", Token::FROM},
      {"into", Token::INTO},
      {"table", Token::TABLE},
      {"id", Token::TYPE_ID},
      {"string", Token::TYPE_STRING},
      {"\\*", Token::ALL_ATTRIBUTES},
      {"[A-Za-z0-9]+", Token::IDENTIFIER},
      {"\\(", Token::LPAREN},
      {"\\)", Token::RPAREN},
      {",", Token::COMMA}};
  int prev_token_pos, curr_token_pos, next_token_pos;
  std::vector<std::pair<std::string, Token>> tokens;

  const std::string get_valid_regex() const {
    std::string valid = "";

    for (const auto& regex_to_token : REGEX_TO_TOKENS) {
      valid += "(" + regex_to_token.first + ")|";
    }
    valid.pop_back();

    return valid;
  }

  const Token& get_token_at_idx(const int& idx) const {
    return REGEX_TO_TOKENS[idx].second;
  }

 public:
  Lexer()
      : prev_token_pos(-1),
        curr_token_pos(0),
        next_token_pos(0),
        tokens(std::vector<std::pair<std::string, Token>>()){};
  ~Lexer() = default;

  void lex(const std::string& input) {
    std::regex re(Lexer::get_valid_regex(), std::regex::extended);
    auto input_begin = std::sregex_iterator(input.begin(), input.end(), re);
    auto input_end = std::sregex_iterator();

    for (auto it = input_begin; it != input_end; ++it) {
      size_t idx = 0;

      for (; idx < it->size(); idx++) {
        if (!it->str(idx + 1).empty()) {
          break;
        }
      }
      this->tokens.push_back(
          make_pair(it->str(), Lexer::get_token_at_idx((int)idx)));
    }
  }

  const bool has_next_token() const {
    return next_token_pos < this->tokens.size();
  }

  const std::pair<std::string, Token> get_prev_token() const {
    return this->tokens[prev_token_pos];
  }

  const std::pair<std::string, Token> get_curr_token() {
    std::pair<std::string, Token> token = this->tokens[this->next_token_pos];
    this->prev_token_pos = this->curr_token_pos;
    this->curr_token_pos = this->next_token_pos;
    this->next_token_pos++;
    return std::move(token);
  }
};

#endif
