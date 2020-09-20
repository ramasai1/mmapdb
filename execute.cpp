#include "execute.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

static void check_errors(const bool &invalid, const std::string &msg) {
  if (invalid) {
    std::cout << msg << std::endl;
    exit(-1);
  }
}

static bool file_exists(const std::string &filename) {
  struct stat buffer;

  return stat(filename.c_str(), &buffer) == 0;
}

void execute_create(CreateStatement create_stmt) {
  int metadata_fd, table_fd;
  std::map<std::string, std::string> &schema = create_stmt.get_mappings();
  int textsize = 0, mapped_idx = 0;
  std::string metadata_filename =
      "data/" + create_stmt.get_table_name() + ".metadata";
  std::string db_filename = "data/" + create_stmt.get_table_name() + ".db";

  if (file_exists(metadata_filename)) {
    std::cout << "table " + create_stmt.get_table_name() + " already exists."
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

void execute_insert(InsertStatement insert_stmt) {
  std::string db_filename = "data/" + insert_stmt.get_table_name() + ".db";

  if (!file_exists(db_filename)) {
    check_errors(true, "table with name: " + insert_stmt.get_table_name() +
                           " does not exist");
  }

  struct stat db_fileinfo = {0};
  int fd, textsize = 0, mapped_idx = 0;
  std::vector<std::vector<std::string>> &tuples = insert_stmt.get_tuples();
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
  mapped = (char *)mmap(0, textsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  check_errors(mapped == MAP_FAILED,
               "Unable to mmap() db file while inserting values");
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

void execute_select(SelectStatement select_stmt) {}
