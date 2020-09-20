#include "parse.hpp"

CreateStatement parse_create(const std::string &create_stmt) {
  CreateStatement response;
  std::string token = "";
  unsigned int idx = 0;

  while (idx < create_stmt.size()) {
    if (create_stmt[idx] == '(') {
      std::string colname = "", datatype = "";
      bool space_seen = false;
      idx++;
      while (idx < create_stmt.size()) {
        if (create_stmt[idx] == ',' || create_stmt[idx] == ')') {
          idx++;
          response.add_mapping(colname, datatype);
          while (idx < create_stmt.size() and std::isblank(create_stmt[idx])) {
            idx++;
          }
          space_seen = false;
          datatype = colname = "";
        } else {
          while (idx < create_stmt.size() and std::isblank(create_stmt[idx])) {
            idx++;
            space_seen = true;
          }

          if (space_seen) {
            datatype += create_stmt[idx++];
          } else {
            colname += create_stmt[idx++];
          }
        }
      }
    } else {
      if (std::isblank(create_stmt[idx])) {
        if (token != "table") {
          response.set_table_name(token);
        }
        idx++;
        token = "";
      } else {
        token += create_stmt[idx++];
      }
    }
  }

  return response;
}

SelectStatement parse_select(const std::string &select_stmt) {
  SelectStatement response;
  std::string token = "";
  unsigned int idx = 0;

  while (idx != select_stmt.size()) {
    if (select_stmt[idx] == ',' || std::isblank(select_stmt[idx])) {
      if (!token.empty() && token != "from") {
        response.add_attribute(token);
      }
      idx++;
      token = "";
    } else {
      token += select_stmt[idx++];
    }
  }
  response.set_table_name(token);

  return response;
}
