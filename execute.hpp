#ifndef EXECUTE
#define EXECUTE

#include "types.hpp"

void execute_create(CreateStatement);
void execute_insert(InsertStatement);
void execute_select(SelectStatement);

#endif
