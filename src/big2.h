#ifndef BIG2_H_
#define BIG2_H_

#include <stdbool.h>
#include <stdint.h>

#define PACKED __attribute__((__packed__))

#define CARD_AMOUNT 52
#define MAX_PLAYERS 4
#define MIN_PLAYERS 2
#define MAX_HAND_SIZE 5

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

typedef struct {
  Card cards[MAX_HAND_SIZE];
  uint8_t cardCount;
} CardHand;

typedef enum {
  NO_HAND,
  HIGH_CARD,
  PAIR,
  THREE_OF_A_KIND,
  STRAIGHT,
  FLUSH,
  FULL_HOUSE,
  FOUR_OF_A_KIND,
  STRAIGHT_FLUSH,
} HandKind;

GameContext generateGame(uint8_t playerCount);

void printCard(Card card);

void printHandKind(HandKind kind);

HandKind handKind(CardHand hand);

#endif  // BIG2_H_
