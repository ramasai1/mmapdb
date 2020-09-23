#ifndef TYPES
#define TYPES

#include <map>
#include <string>
#include <vector>

enum class Token {
  CREATE,
  INSERT,
  SELECT,
  FROM,
  INTO,
  TABLE,
  ALL_ATTRIBUTES,
  IDENTIFIER,
  VALUE,
  RPAREN,
  LPAREN,
  VALUES,
  TYPE_ID,
  TYPE_STRING,
  COMMA
};

class Statement {
 protected:
  std::string table_name;

 public:
  Statement(std::string table_name) : table_name(table_name){};
  virtual ~Statement() = default;
  virtual void execute() = 0;
};

class CreateStatement : public Statement {
  std::map<std::string, std::string> colname_to_datatype;

 public:
  CreateStatement()
      : Statement(""),
        colname_to_datatype(std::map<std::string, std::string>()){};

  CreateStatement(std::string table_name,
                  std::map<std::string, std::string> colname_to_datatype)
      : Statement(table_name), colname_to_datatype(colname_to_datatype){};

  ~CreateStatement() override{};

  void execute() override;

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

class InsertStatement : public Statement {
  std::vector<std::vector<std::string>> tuples;

 public:
  InsertStatement(std::string table_name,
                  std::vector<std::vector<std::string>> tuples)
      : Statement(table_name), tuples(tuples){};

  InsertStatement()
      : Statement(""), tuples(std::vector<std::vector<std::string>>()){};

  ~InsertStatement() override{};

  void execute() override;

  std::string get_table_name() const { return table_name; }

  void set_table_name(const std::string &table_name) {
    this->table_name = table_name;
  }

  void add_tuple(const std::vector<std::string> &tuple) {
    tuples.push_back(tuple);
  }

  std::vector<std::vector<std::string>> &get_tuples() { return tuples; }
};

class SelectStatement : public Statement {
  std::vector<std::string> attributes;
  bool select_all;

 public:
  SelectStatement(std::string table_name, std::vector<std::string> attributes)
      : Statement(table_name), attributes(attributes), select_all(false){};

  SelectStatement()
      : Statement(""),
        attributes(std::vector<std::string>()),
        select_all(false){};

  ~SelectStatement() override{};

  void execute() override;

  std::string get_table_name() const { return table_name; }
  void set_table_name(const std::string &table_name) {
    this->table_name = table_name;
  }

  void add_attribute(const std::string &attribute) {
    this->attributes.push_back(attribute);
  }

  std::vector<std::string> &get_attributes() { return attributes; }

  void set_select_all(const bool &select_all) { this->select_all = select_all; }

  const bool &get_select_all() const { return select_all; }
};

#endif
