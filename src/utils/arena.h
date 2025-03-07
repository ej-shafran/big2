#include <stddef.h>
#include <stdint.h>

#ifndef ARENA_H_
#define ARENA_H_

typedef struct {
  uintptr_t nextAllocation;
  size_t capacity;
  char* memory;
} Arena;

Arena CreateArenaWithCapacityAndMemory(size_t capacity, void* memory);

void* Arena_Allocate(size_t bytes, Arena* arena);
void Arena_Free(Arena* arena);

#endif  // ARENA_H_
