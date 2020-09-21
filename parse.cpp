#include "parse.hpp"

// `create table t (id int, name string)`
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

// `select id(, ...) from table`
SelectStatement parse_select(const std::string &select_stmt) {
  SelectStatement response;
  std::string token = "";
  unsigned int idx = 0;

  while (idx != select_stmt.size()) {
    if (select_stmt[idx] == ',' || std::isblank(select_stmt[idx])) {
      if (!token.empty() && token != "from") {
        response.add_attribute(token);
        if (token == "*") {
          response.set_select_all(true);
        }
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

// `insert into t (1, samuel), (2, harry)`
InsertStatement parse_insert(const std::string &insert_stmt) {
  InsertStatement response;
  std::string token;
  unsigned int idx = 0;
  bool inside_attr = false, table_name_set = false;

  while (idx < insert_stmt.size()) {
    if (inside_attr) {
      std::vector<std::string> tuple;
      while (idx < insert_stmt.size() && insert_stmt[idx] != ')') {
        if (std::isblank(insert_stmt[idx])) {
          idx++;
        } else if (insert_stmt[idx] == ',') {
          tuple.push_back(token);
          token = "";
          idx++;
        } else {
          token += insert_stmt[idx++];
        }
      }
      tuple.push_back(token);
      response.add_tuple(tuple);
      inside_attr = false;
      token = "";
      idx++;
    } else {
      if (std::isblank(insert_stmt[idx])) {
        idx++;
        if (token != "into") {
          response.set_table_name(token);
          table_name_set = true;
        }
        token = "";
      } else if (insert_stmt[idx] == ',' || table_name_set) {
        if (insert_stmt[idx] == ',') {
          idx++;
        }
        while (std::isblank(insert_stmt[idx])) {
          idx++;
        }
        if (insert_stmt[idx] == '(') {
          idx++;
          token = "";
          inside_attr = true;
        }
      } else {
        token += insert_stmt[idx++];
      }
    }
  }

  return response;
}
