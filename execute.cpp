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

static bool file_exists(std::string filename) {
  struct stat buffer;

  return stat(filename.c_str(), &buffer) == 0;
}

void execute_create(CreateStatement create_stmt) {
  struct stat metadata = {0};
  int metadata_fd, table_fd, stat_status;
  std::map<std::string, std::string> &schema = create_stmt.get_mappings();
  int textsize = 0;
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
  stat_status = fstat(metadata_fd, &metadata);
  check_errors(stat_status < 0, "Unable to stat metadata file.");

  for (auto it = schema.begin(); it != schema.end(); it++) {
    textsize += (it->first.size() + 1 + it->second.size() + 1);
  }
  ftruncate(metadata_fd, textsize);
  char *mapped = (char *)mmap(0, textsize, PROT_READ | PROT_WRITE, MAP_SHARED,
                              metadata_fd, 0);
  check_errors(mapped == MAP_FAILED,
               "mmap() while writing to metadata failed.");

  int i = 0;
  for (auto it = schema.begin(); it != schema.end(); it++) {
    std::string colname = it->first, datatype = it->second;

    for (unsigned int j = 0; j < colname.size(); j++, i++) {
      mapped[i] = colname[j];
    }
    mapped[i++] = ':';
    for (unsigned int j = 0; j < datatype.size(); j++, i++) {
      mapped[i] = datatype[j];
    }
    mapped[i++] = '\n';
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

void execute_select(SelectStatement select_stmt) {}
