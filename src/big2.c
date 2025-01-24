#include "big2.h"
#include <assert.h>
#include <stdio.h>
#include "utils/pcg_basic.h"

void printCard(Card card) {
  switch (card.rank) {
    case RANK_3:
      printf(" 3");
      break;
    case RANK_4:
      printf(" 4");
      break;
    case RANK_5:
      printf(" 5");
      break;
    case RANK_6:
      printf(" 6");
      break;
    case RANK_7:
      printf(" 7");
      break;
    case RANK_8:
      printf(" 8");
      break;
    case RANK_9:
      printf(" 9");
      break;
    case RANK_10:
      printf("10");
      break;
    case RANK_J:
      printf(" J");
      break;
    case RANK_Q:
      printf(" Q");
      break;
    case RANK_K:
      printf(" K");
      break;
    case RANK_A:
      printf(" A");
      break;
    case RANK_2:
      printf(" 2");
      break;
    case RANK_AMOUNT:
      assert(0);
  }

  switch (card.suit) {
    case DIAMONDS:
      printf("♦");
      break;
    case CLUBS:
      printf("♣");
      break;
    case HEARTS:
      printf("♥");
      break;
    case SPADES:
      printf("♠");
      break;
    case SUIT_AMOUNT:
      assert(0);
  }
}

PlayerCards dealDeck(uint8_t playerCount) {
  Card deck[CARD_AMOUNT];
  for (int rank = 0; rank < RANK_AMOUNT; rank++) {
    for (int suit = 0; suit < SUIT_AMOUNT; suit++) {
      Card card = {.rank = rank, .suit = suit};
      int i = (RANK_AMOUNT * suit) + rank;
      deck[i] = card;
    }
  }

  PlayerCards ret = {.playerCount = playerCount, .hands = {0}};
  for (int i = 0; i < playerCount; i++) {
    ret.hands[i] = (CardArray){.count = 0, .cards = {0}};
  }

  int cardsPerPlayer = CARD_AMOUNT / playerCount;
  int count = CARD_AMOUNT;
  for (int player = 0; player < playerCount; player++) {
    for (int i = 0; i < cardsPerPlayer; i++) {
      int cardIndex = pcg32Boundedrand(count);

      Card card = deck[cardIndex];
      deck[cardIndex] = deck[count - 1];
      count -= 1;

      ret.hands[player].cards[ret.hands[player].count] = card;
      ret.hands[player].count++;
    }
  }

  return ret;
}
