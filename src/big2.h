#ifndef BIG2_H_
#define BIG2_H_

#include <stdbool.h>
#include <stdint.h>
#include "utils/arena.h"
#include "utils/array.h"

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

typedef enum PACKED {
  NO_HAND,
  HIGH_CARD,
  PAIR,
  THREE_OF_A_KIND,
  STRAIGHT,
  FLUSH,
  FULL_HOUSE,
  FOUR_OF_A_KIND,
  STRAIGHT_FLUSH,
  HAND_KIND_AMOUNT,
} HandKind;

typedef struct {
  CardRank rank;
  CardSuit suit;
} Card;
bool Card_Eq(Card a, Card b);
bool Card_EqRank(Card a, Card b);
bool Card_GtRank(Card a, Card b);
bool Card_EqSuit(Card a, Card b);
bool Card_GtSuit(Card a, Card b);
ARRAY__DECLARE(Card, CardArray)

typedef struct {
  CardArray hand;
} Player;
ARRAY__DECLARE(Player, PlayerArray)

bool CardIndex_Eq(int32_t a, int32_t b);
bool CardIndex_Gt(int32_t a, int32_t b);
ARRAY__DECLARE(int32_t, CardIndexArray)

typedef struct {
  PlayerArray players;
  CardIndexArray selectedCardIndexes;
  const char* seedString;
  int32_t seedStringLength;
  int32_t currentPlayerIndex;
  int32_t playedHandSize;
  HandKind selectedHandKind;
} GameContext;

GameContext generateGame(uint64_t seed, int32_t playerCount, Arena* arena);
HandKind handKind(CardArray* hand, CardIndexArray* selectedIndexes);

#endif  // BIG2_H_
