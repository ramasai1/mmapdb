#ifndef TYPES
#define TYPES

#include <map>
#include <string>
#include <vector>

class CreateStatement {
  std::string table_name;
  std::map<std::string, std::string> colname_to_datatype;

 public:
  void set_table_name(const std::string &table_name) {
    this->table_name = table_name;
  }

  std::string get_table_name() const { return this->table_name; }

  void add_mapping(const std::string &colname, const std::string &datatype) {
    colname_to_datatype[colname] = datatype;
  }

  std::map<std::string, std::string> &get_mappings() {
    return colname_to_datatype;
  }
};

class InsertStatement {};

class SelectStatement {
  std::string table_name;
  std::vector<std::string> attributes;

 public:
  SelectStatement(std::string table_name, std::vector<std::string> attributes)
      : table_name(table_name), attributes(attributes){};

  SelectStatement() : table_name(""), attributes(std::vector<std::string>()){};

  void set_table_name(const std::string &table_name) {
    this->table_name = table_name;
  }

  void add_attribute(const std::string &attribute) {
    this->attributes.push_back(attribute);
  }
};

#endif
