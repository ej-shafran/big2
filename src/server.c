#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "big2.h"
#include "utils/pcg_basic.h"

#define SEQ 54u

uint64_t generateSeed(void) {
  pcg32_random_t rng = PCG32_INITIALIZER;
  int rounds = 5;
  pcg32Srandom_r(&rng, time(NULL) ^ (intptr_t)&printf, (intptr_t)&rounds);
  return pcg32Random_r(&rng);
}

const char* shiftArg(int* argc, const char*** argv) {
  const char* arg = **argv;
  *argc -= 1;
  *argv += 1;
  return arg;
}

int main(int argc, const char** argv) {
  uint8_t playerCount = 4;
  uint64_t deterministicSeed = 0;

  shiftArg(&argc, &argv);

  while (argc > 0) {
    const char* arg = shiftArg(&argc, &argv);
    if (strcmp(arg, "--seed") == 0) {
      if (argc == 0) {
        fprintf(stderr, "missing argument for --seed\n");
        return EXIT_FAILURE;
      }

      const char* seedString = shiftArg(&argc, &argv);
      char* endPointer;
      errno = 0;
      deterministicSeed = strtoul(seedString, &endPointer, 16);
      if (errno != 0) {
        perror(NULL);
        return EXIT_FAILURE;
      }

      if (endPointer == seedString) {
        fprintf(stderr, "invalid seed string\n");
        return EXIT_FAILURE;
      }
    } else if (strcmp(arg, "--player-count") == 0) {
      if (argc == 0) {
        fprintf(stderr, "missing argument for --player-count\n");
        return EXIT_FAILURE;
      }

      const char* playerCountString = shiftArg(&argc, &argv);
      char* endPointer;
      errno = 0;
      playerCount = strtoul(playerCountString, &endPointer, 10);
      if (errno != 0) {
        perror(NULL);
        return EXIT_FAILURE;
      }

      if (endPointer == playerCountString || playerCount > MAX_PLAYERS ||
          playerCount < MIN_PLAYERS) {
        fprintf(stderr, "invalid player count string\n");
        return EXIT_FAILURE;
      }
    } else {
      fprintf(stderr, "unrecognized argument %s\n", arg);
      return EXIT_FAILURE;
    }
  }

  uint64_t seed = deterministicSeed == 0 ? generateSeed() : deterministicSeed;
  pcg32Srandom(seed, SEQ);
  printf("Seed: %lx\n\n", seed);

  PlayerCards deck = dealDeck(playerCount);

  for (int i = 0; i < deck.playerCount; i++) {
    CardArray hand = deck.hands[i];
    printf("Player %d:\n", i + 1);
    for (int j = 0; j < hand.count; j++) {
      printf("  ");
      printCard(hand.cards[j]);
      if (j != hand.count - 1)
        printf(" ");
    }
    printf("\n");
  }

  return 0;
}
