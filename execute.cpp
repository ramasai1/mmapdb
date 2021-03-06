#include "execute.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>

static void inline check_errors(const bool &, const std::string &);
static bool file_exists(const std::string &);
static int inline round_down_to_pagesize(const int &);
static std::vector<std::string> tokenize(const std::string &, char delim);
static std::map<std::string, int> get_row_layout_on_disk(const std::string &);

void CreateStatement::execute() {
  int metadata_fd, table_fd;
  std::vector<std::pair<std::string, std::string>> &schema = this->get_mappings();
  int textsize = 0, mapped_idx = 0;
  std::string metadata_filename =
      "data/" + this->get_table_name() + ".metadata";
  std::string db_filename = "data/" + this->get_table_name() + ".db";

  if (file_exists(metadata_filename)) {
    std::cout << "table " + this->get_table_name() + " already exists."
              << std::endl;
    return;
  }
  metadata_fd =
      open(metadata_filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
  check_errors(metadata_fd < 0, "Unable to open metadata file.");

  for (auto it = schema.begin(); it != schema.end(); it++) {
    textsize += (it->first.size() + 1);
    textsize += (it->first.size() + 1 + it->second.size() + 1);
  }
  ftruncate(metadata_fd, textsize);
  char *mapped = (char *)mmap(0, textsize, PROT_READ | PROT_WRITE, MAP_SHARED,
                              metadata_fd, 0);
  check_errors(mapped == MAP_FAILED,
               "mmap() while writing to metadata failed.");

  // dump list of attributes for `insert` to read and know the order.
  for (auto it = schema.begin(); it != schema.end(); ++it) {
    std::string colname = it->first;
    for (unsigned int colname_idx = 0; colname_idx < colname.size();
         colname_idx++, mapped_idx++) {
      mapped[mapped_idx] += colname[colname_idx];
    }
    if (std::distance(schema.begin(), it) != (int)(schema.size() - 1)) {
      mapped[mapped_idx++] = ',';
    }
  }
  mapped[mapped_idx++] = '\n';

  // write attributes and their data types (colon separated).
  for (auto it = schema.begin(); it != schema.end(); it++) {
    std::string colname = it->first, datatype = it->second;

    for (unsigned int colname_idx = 0; colname_idx < colname.size();
         colname_idx++, mapped_idx++) {
      mapped[mapped_idx] = colname[colname_idx];
    }
    mapped[mapped_idx++] = ':';
    for (unsigned int datatype_idx = 0; datatype_idx < datatype.size();
         datatype_idx++, mapped_idx++) {
      mapped[mapped_idx] = datatype[datatype_idx];
    }
    mapped[mapped_idx++] = '\n';
  }
  check_errors(msync(mapped, textsize, MS_SYNC) != 0,
               "msync() while writing metadata failed.");
  check_errors(munmap(mapped, textsize) != 0,
               "munmap() while closing metadata map failed.");
  check_errors(close(metadata_fd) != 0, "close() on metadata file failed");

  table_fd = open(db_filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
  check_errors(table_fd < 0, "Unable to open() .db file");
  close(table_fd);
}

void InsertStatement::execute() {
  std::string db_filename = "data/" + this->get_table_name() + ".db";

  if (!file_exists(db_filename)) {
    check_errors(
        true, "table with name: " + this->get_table_name() + " does not exist");
  }

  struct stat db_fileinfo = {0};
  int fd, textsize = 0, mapped_idx = 0;
  std::vector<std::vector<std::string>> &tuples = this->get_tuples();
  char *mapped;

  fd = open(db_filename.c_str(), O_RDWR);
  check_errors(fd < 0, "Unable to open DB file to insert values.");
  check_errors(fstat(fd, &db_fileinfo) < 0, "Unable to stat DB file for size");
  for (auto &tuple : tuples) {
    for (auto &attribute : tuple) {
      textsize += (attribute.size() + 1);
    }
  }
  check_errors(ftruncate(fd, db_fileinfo.st_size + textsize) < 0,
               "Unable to extend file size while inserting values");
  int offset = round_down_to_pagesize(db_fileinfo.st_size);
  mapped =
      (char *)mmap(0, textsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
  check_errors(mapped == MAP_FAILED,
               "Unable to mmap() db file while inserting values");
  mapped_idx = db_fileinfo.st_size;
  for (auto &tuple : tuples) {
    for (unsigned int tuple_idx = 0; tuple_idx < tuple.size(); tuple_idx++) {
      std::string attribute = tuple[tuple_idx];
      for (unsigned int attribute_idx = 0; attribute_idx < attribute.size();
           attribute_idx++, mapped_idx++) {
        mapped[mapped_idx] = attribute[attribute_idx];
      }
      if (tuple_idx != tuple.size() - 1) {
        mapped[mapped_idx++] = ',';
      }
    }
    mapped[mapped_idx++] = '\n';
  }

  check_errors(msync(mapped, textsize, MS_SYNC) < 0,
               "Unable to sync written file to disk");
  check_errors(munmap(mapped, textsize) < 0,
               "Unable to unmap area after inserting");
  check_errors(close(fd) < 0, "Unable to close db file after inserting");
}

void SelectStatement::execute() {
  std::string db_filename = "data/" + this->get_table_name() + ".db";
  std::string metadata_filename =
      "data/" + this->get_table_name() + ".metadata";

  check_errors(!file_exists(db_filename),
               "table: " + this->get_table_name() + " does not exist.");
  int fd;
  size_t sz = 0;
  struct stat db_fileinfo;
  char *mapped, *line;
  FILE *stream;
  std::map<std::string, std::string> attribute_to_type;
  std::string row_layout;
  bool collected_layout = false;

  // get order of row layout on disk.
  fd = open(metadata_filename.c_str(), O_RDONLY);
  check_errors(fd < 0, "Unable to open metadata file to get row layout.");
  stream = fdopen(fd, "r");
  check_errors(stream == NULL, "Unable to open stream to read metadata");

  while (getline(&line, &sz, stream) != -1) {
    if (!collected_layout) {
      row_layout = std::string(line);
    }
    if (this->get_where_operator() == Token::SENTINEL) {
      // not going to read metadata file if there is no where filter specified.
      break;
    } else if (collected_layout) {
      std::vector<std::string> tokens = tokenize(line, ':');
      attribute_to_type[tokens[0]] = tokens[1];
    }
    collected_layout = true;
  }
  check_errors(fclose(stream) == EOF,
               "Unable to close stream after reading row layout");

  // mmap() DB file.
  fd = open(db_filename.c_str(), O_RDONLY);
  check_errors(fd < 0, "Unable to open db file to read values.");
  check_errors(fstat(fd, &db_fileinfo) < 0, "Unable to stat db file to read");
  mapped = (char *)mmap(0, db_fileinfo.st_size, PROT_READ, MAP_SHARED, fd, 0);
  check_errors(mapped == MAP_FAILED,
               "mmap failed while reading DB to select values");

  // print out contents of each row.
  db_row_buffer buffer(mapped, db_fileinfo.st_size);
  std::istream db_row_stream(&buffer);
  std::string row;
  bool select_all_rows = this->get_select_all();
  std::map<std::string, int> attribute_to_col_idx =
      get_row_layout_on_disk(row_layout);
  std::vector<std::string> &desired_attributes = this->get_attributes();
  bool broken = false;
  std::vector<std::vector<std::string>> output_rows;
  std::pair<std::string, std::string> &where_filter = this->get_where_filter();

  while (std::getline(db_row_stream, row)) {
    std::vector<std::string> all_attributes = tokenize(row, ',');
    if (this->get_where_operator() == Token::SENTINEL) {
      output_rows.push_back(all_attributes);
    } else {
      // check if attribute on which we're filtering exists.
      if (attribute_to_col_idx.find(where_filter.first) ==
          attribute_to_col_idx.end()) {
        throw std::invalid_argument(
            "calling where on an invalid attribute on " +
            this->get_table_name());
      }
      // check the type to see if they match. string doesn't matter because
      // everything is a string.
      if (attribute_to_type[where_filter.first] == "int") {
        try {
          std::stoi(where_filter.second);
        } catch (std::invalid_argument &e) {
          throw std::invalid_argument("Wrong type comparison for " +
                                      where_filter.first);
        }
      }
      // check if the value actually matches.
      if (all_attributes[attribute_to_col_idx[where_filter.first]] ==
          where_filter.second) {
        output_rows.push_back(all_attributes);
      }
    }
  }
  for (auto &output_row : output_rows) {
    if (!select_all_rows) {
      for (auto &desired_attribute : desired_attributes) {
        if (attribute_to_col_idx.find(desired_attribute) ==
            attribute_to_col_idx.end()) {
          check_errors(true, "Attribute " + desired_attribute +
                                 " does not exist on table " +
                                 this->get_table_name());
          broken = true;
          break;
        }
        std::cout << output_row[attribute_to_col_idx[desired_attribute]]
                  << '\t';
      }
    } else {
      for (auto &attribute : output_row) {
        std::cout << attribute << '\t';
      }
    }
    if (broken) break;
    std::cout << std::endl;
  }

  check_errors(munmap(mapped, db_fileinfo.st_size) < 0,
               "Unable to close mapped file after selecting");
  check_errors(close(fd) < 0, "Unable to close file after reading");
}

void DropStatement::execute() {
  std::string metadata_file = "data/" + this->get_table_name() + ".metadata";
  std::string db_file = "data/" + this->get_table_name() + ".db";

  if (file_exists(db_file) && file_exists(metadata_file)) {
    system(std::string("rm " + metadata_file).c_str());
    system(std::string("rm " + db_file).c_str());
  } else {
    std::cout << "files for table " << this->get_table_name()
              << " does not fully exist; table can't be dropped. please "
                 "manually clean up files."
              << std::endl;
  }
}

static void inline check_errors(const bool &invalid, const std::string &msg) {
  if (invalid) {
    std::cout << msg << std::endl;
    exit(-1);
  }
}

static bool file_exists(const std::string &filename) {
  struct stat buffer;

  return stat(filename.c_str(), &buffer) == 0;
}

static int inline round_down_to_pagesize(const int &file_size) {
  int page_size = getpagesize();
  int remainder = file_size % page_size;

  if (remainder == 0) {
    return file_size;
  }

  return file_size - remainder;
}

static std::map<std::string, int> get_row_layout_on_disk(
    const std::string &layout) {
  int idx = 0;
  std::map<std::string, int> attribute_to_col_idx;

  for (auto &token : tokenize(layout, ',')) {
    attribute_to_col_idx[token] = idx++;
  }

  return attribute_to_col_idx;
}

static std::vector<std::string> tokenize(const std::string &str, char delim) {
  std::stringstream ss(str);
  std::string token;
  std::vector<std::string> tokens;

  while (std::getline(ss, token, delim)) {
    if (!token.empty() && token[token.size() - 1] == '\n') {
      token.erase(token.length() - 1);
    }
    tokens.push_back(token);
  }

  return tokens;
}
