#include <stdio.h>
#include "big2.h"

int main(void) {
  PlayerCards deck = dealDeck(0, 4);

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
