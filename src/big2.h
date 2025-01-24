#ifndef BIG2_H_
#define BIG2_H_

#include <stdint.h>

#define PACKED __attribute__((__packed__))

#define CARD_AMOUNT 52
#define MAX_PLAYERS 4
#define MIN_PLAYERS 2

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

typedef Card MaxSelectedCards[CARD_AMOUNT / MIN_PLAYERS];

typedef struct {
  MaxSelectedCards cards;
  uint8_t cardCount;
} CardArray;

typedef struct {
  CardArray players[MAX_PLAYERS];
  uint8_t playerCount;
  uint8_t currentPlayerIndex;
} GameContext;

GameContext generateGame(uint8_t playerCount);

void printCard(Card card);

#endif  // BIG2_H_
