#include <iostream>

#include "execute.hpp"
#include "parse.hpp"

int main() {
  Lexer lexer{};
  Parser parser{};
  std::string input;

  std::cout << "> ";
  while (std::getline(std::cin, input)) {
    if (input == "exit" || input == "q" || input == "quit") {
      break;
    }
    lexer.lex(input);
    Statement* parsed_stmt = parser.parse(lexer);
    parsed_stmt->execute();
    std::cout << "> ";
  }

  return 0;
}
