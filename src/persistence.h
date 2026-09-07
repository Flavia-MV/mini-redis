#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "hashtable.h"

void aof_append(const char *command_line);
void aof_load(HashTable *ht);


#endif // !PERSISTENCE_H