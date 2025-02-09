#ifndef BIG2_H_
#define BIG2_H_

#include <stdbool.h>
#include <stdint.h>

#define PACKED __attribute__((__packed__))

#define CARD_AMOUNT 52
#define MAX_PLAYERS 4
#define MIN_PLAYERS 2
#define MAX_HAND_SIZE 5
#define MAX_SELECTED_CARDS (CARD_AMOUNT / MIN_PLAYERS)

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

typedef Card DeckOfCards[CARD_AMOUNT];

typedef struct {
  Card items[MAX_SELECTED_CARDS];
  uint8_t count;
} CardArray;

typedef struct {
  CardArray hand;
} Player;

typedef struct {
  Player items[MAX_PLAYERS];
  uint8_t count;
} PlayerArray;

typedef struct {
  PlayerArray players;
  uint8_t currentPlayerIndex;
} GameContext;

typedef struct {
  Card items[MAX_HAND_SIZE];
  uint8_t count;
} PlayedCardHand;

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

HandKind handKind(PlayedCardHand hand);

#endif  // BIG2_H_
