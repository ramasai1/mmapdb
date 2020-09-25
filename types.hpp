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
  TYPE_INT,
  TYPE_STRING,
  COMMA,
  DROP,
  WHERE,
  EQUAL,
  SENTINEL
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
  Token where_operator;
  std::pair<std::string, std::string> where_filter;

 public:
  SelectStatement(std::string table_name, std::vector<std::string> attributes)
      : Statement(table_name),
        attributes(attributes),
        select_all(false),
        where_operator(Token::SENTINEL),
        where_filter(std::make_pair("", "")){};

  SelectStatement()
      : Statement(""),
        attributes(std::vector<std::string>()),
        select_all(false),
        where_operator(Token::SENTINEL),
        where_filter(std::make_pair("", "")){};

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

  void set_where_operator(const Token &where_operator) {
    this->where_operator = where_operator;
  }

  const Token get_where_operator() const noexcept {
    return this->where_operator;
  }

  void set_where_filter(const std::string &attribute,
                        const std::string &condition) {
    this->where_filter = std::make_pair(attribute, condition);
  }

  std::pair<std::string, std::string> &get_where_filter() noexcept {
    return this->where_filter;
  }
};

class DropStatement : public Statement {
 public:
  DropStatement(std::string table_name) : Statement(table_name){};

  DropStatement() : Statement(""){};

  ~DropStatement() override{};

  void execute() override;

  void set_table_name(const std::string &table_name) {
    this->table_name = table_name;
  }

  const std::string &get_table_name() const { return table_name; }
};

#endif
