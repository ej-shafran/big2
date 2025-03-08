#include "shared.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../big2.h"
#include "../utils/pcg_basic.h"

#define SHIFT_ARGS(argc, argv) \
  (assert(argc > 0 && "cannot shift empty args"), (argc)--, *((argv)++))

uint64_t generateSeed(void) {
  pcg32_random_t rng = PCG32_INITIALIZER;
  int rounds = 5;
  pcg32Srandom_r(&rng, time(NULL) ^ (intptr_t)&printf, (intptr_t)&rounds);
  return pcg32Random_r(&rng);
}

bool parseArgs(int argc, const char** argv, Args* args) {
  args->playerCount = 4;
  args->programName = SHIFT_ARGS(argc, argv);

  uint64_t deterministicSeed = 0;

  while (argc > 0) {
    const char* arg = SHIFT_ARGS(argc, argv);

    if (strcmp(arg, "--help") == 0) {
      fprintf(
          stderr,
          "Usage: %s [options]\n"
          "\n"
          "Play Big2.\n"
          "\n"
          "Options:\n"
          "  --seed <seed>           Set the seed to play with (default: "
          "random).\n"
          "  --player-count <count>  Set how many players to play with "
          "(default: 4).\n"
          "  --help                  Show this help and message and exit.\n",
          args->programName);
      return false;
    } else if (strcmp(arg, "--seed") == 0) {
      if (argc == 0) {
        fprintf(stderr, "missing argument for --seed\n");
        return false;
      }

      const char* seedString = SHIFT_ARGS(argc, argv);
      char* endPointer;
      errno = 0;
      deterministicSeed = strtoul(seedString, &endPointer, 16);
      if (errno != 0) {
        fprintf(stderr, "invalid seed - not a hexadecimal number\n");
        return false;
      }
      if (endPointer == seedString) {
        fprintf(stderr, "invalid seed - empty string\n");
        return false;
      }
    } else if (strcmp(arg, "--player-count") == 0) {
      if (argc == 0) {
        fprintf(stderr, "missing argument for --player-count\n");
        return false;
      }
      const char* playerCountString = SHIFT_ARGS(argc, argv);
      char* endPointer;
      errno = 0;
      args->playerCount = strtoul(playerCountString, &endPointer, 10);
      if (errno != 0) {
        fprintf(stderr, "invalid player count - not a number\n");
        return false;
      }

      if (endPointer == playerCountString || args->playerCount > MAX_PLAYERS ||
          args->playerCount < MIN_PLAYERS) {
        fprintf(stderr, "invalid player count - empty string\n");
        return false;
      }
    } else {
      fprintf(stderr, "unrecognized argument %s\n", arg);
      return false;
    }
  }
  args->seed = deterministicSeed == 0 ? generateSeed() : deterministicSeed;

  return true;
}
