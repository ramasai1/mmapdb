#include "lexer.hpp"

void Lexer::lex(const std::string &input) {
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

const bool Lexer::has_next_token() const noexcept {
  return this->next_token_pos < (int) this->tokens.size();
}

const std::pair<std::string, Token> Lexer::get_prev_token() const noexcept {
  return this->tokens[this->prev_token_pos];
}

const std::pair<std::string, Token> Lexer::get_curr_token() noexcept {
  std::pair<std::string, Token> token = this->tokens[this->next_token_pos];
  this->prev_token_pos = this->curr_token_pos;
  this->curr_token_pos = this->next_token_pos;
  this->next_token_pos++;
  return std::move(token);
}
