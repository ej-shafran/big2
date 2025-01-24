#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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


int main(void) {
  uint64_t seed = generateSeed();
  pcg32Srandom(seed, SEQ);
  printf("Seed: %lx\n\n", seed);

  PlayerCards deck = dealDeck(4);

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
