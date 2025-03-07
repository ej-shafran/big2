#ifndef SHARED_H_
#define SHARED_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint64_t seed;
  int32_t playerCount;
  const char* programName;
} Args;

bool parseArgs(int argc, const char** argv, Args* args);

#endif  // SHARED_H_
