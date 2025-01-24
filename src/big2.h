#ifndef BIG2_H_
#define BIG2_H_

#include <stdint.h>

#define PACKED __attribute__((__packed__))

#define CARD_AMOUNT 52
#define MAX_PLAYERS 4

typedef enum PACKED {
  RANK_3,
  RANK_4,
  RANK_5,
  RANK_6,
  RANK_7,
  RANK_8,
  RANK_9,
  RANK_10,
  RANK_J,
  RANK_Q,
  RANK_K,
  RANK_A,
  RANK_2,
  RANK_AMOUNT
} CardRank;

typedef enum PACKED { DIAMONDS, CLUBS, HEARTS, SPADES, SUIT_AMOUNT } CardSuit;

typedef struct {
  CardRank rank;
  CardSuit suit;
} Card;

typedef Card MaxSelectedCards[CARD_AMOUNT / 2];

typedef struct {
  MaxSelectedCards cards;
  uint8_t count;
} CardArray;

typedef struct {
  CardArray hands[MAX_PLAYERS];
  uint8_t playerCount;
} PlayerCards;

PlayerCards dealDeck(uint8_t playerCount);

void printCard(Card card);

#endif  // BIG2_H_
