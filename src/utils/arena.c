#include "arena.h"
#include <assert.h>

Arena CreateArenaWithCapacityAndMemory(size_t capacity, void* memory) {
  return (Arena){
      .capacity = capacity,
      .nextAllocation = 0,
      .memory = memory,
  };
}

void* Arena_Allocate(size_t bytes, Arena* arena) {
  uintptr_t nextAllocationOffset =
      arena->nextAllocation + (64 - (arena->nextAllocation % 64));
  assert(nextAllocationOffset + bytes <= arena->capacity && "Arena too small");
  arena->nextAllocation = nextAllocationOffset + bytes;
  return (void*)((uintptr_t)arena->memory + (uintptr_t)nextAllocationOffset);
}

void Arena_Free(Arena* arena) {
  arena->nextAllocation = 0;
}
