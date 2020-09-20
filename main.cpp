#include "main.hpp"

#include <iostream>

int main() {
  std::string line;
  std::map<std::string, StatementType> stmt_to_type = map_stmt_str_to_type();

  std::cout << "> ";
  while (std::getline(std::cin, line)) {
    if (line == "exit") {
      break;
    }
    std::pair<std::string, int> stmt_name_and_limitidx =
        parse_stmt_name_and_limitidx(line);
    std::string stmt_after_clause = line.substr(stmt_name_and_limitidx.second);
    switch (
        get_stmt_type_from_str(stmt_name_and_limitidx.first, stmt_to_type)) {
      case StatementType::CREATE:
        execute_create(parse_create(stmt_after_clause));
        break;
      case StatementType::INSERT:
        break;
      case StatementType::SELECT:
        execute_select(parse_select(stmt_after_clause));
        break;
      case StatementType::INVALID:
        std::cout << "unrecognized command" << std::endl;
        break;
    }
    std::cout << "> ";
  }

  return 0;
}

std::pair<std::string, int> parse_stmt_name_and_limitidx(
    const std::string &stmt) {
  int idx = 0;
  std::string keyword = "";

  while (!std::isblank(stmt[idx])) {
    keyword += stmt[idx++];
  }

  return std::move(make_pair(keyword, idx + 1));
}

std::map<std::string, StatementType> map_stmt_str_to_type() {
  std::map<std::string, StatementType> stmt_to_type = {
      {"create", StatementType::CREATE},
      {"insert", StatementType::INSERT},
      {"select", StatementType::SELECT}};

  return std::move(stmt_to_type);
}

StatementType get_stmt_type_from_str(
    const std::string &clause,
    std::map<std::string, StatementType> &clause_to_type) {
  if (clause_to_type.find(clause) == clause_to_type.end()) {
    return StatementType::INVALID;
  }

  return clause_to_type[clause];
}
