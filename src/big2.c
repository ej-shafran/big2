#include "big2.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "utils/array.h"
#include "utils/pcg_basic.h"

ARRAY__DEFINE(Player, PlayerArray)
ARRAY__DEFINE(Card, CardArray)
ARRAY__DEFINE(int32_t, CardIndexArray)

bool Card_Eq(Card a, Card b) {
  return Card_EqRank(a, b) && Card_EqSuit(a, b);
}

bool Card_Gt(Card a, Card b) {
  return ((a.suit * RANK_AMOUNT) + a.rank) > ((b.suit * RANK_AMOUNT) + b.rank);
}

bool Card_EqRank(Card a, Card b) {
  return a.rank == b.rank;
}

bool Card_GtRank(Card a, Card b) {
  return a.rank > b.rank;
}

bool Card_EqSuit(Card a, Card b) {
  return a.suit == b.suit;
}

bool Card_GtSuit(Card a, Card b) {
  return a.suit > b.suit;
}

bool CardIndex_Eq(int32_t a, int32_t b) {
  return a == b;
};

bool CardIndex_Gt(int32_t a, int32_t b) {
  return a > b;
};

GameContext generateGame(uint64_t seed, int32_t playerCount, Arena* arena) {
  char* seedString = Arena_Allocate(20 * sizeof(char), arena);
  int32_t seedStringLength = sprintf(seedString, "%" PRIx64, seed);

  GameContext gameContext = {
      .players = PlayerArray_ArenaAllocate(playerCount, arena),
      .selectedCardIndexes = CardIndexArray_ArenaAllocate(MAX_HAND_SIZE, arena),
      .selectedHandKind = NO_HAND,
      .seedString = seedString,
      .seedStringLength = seedStringLength,
      .currentPlayerIndex = 0,
      .playedHandSize = 0};

  int cardsPerPlayer = CARD_AMOUNT / playerCount;

  for (int32_t i = 0; i < playerCount; i++) {
    PlayerArray_Set(
        &gameContext.players, i,
        (Player){.hand = CardArray_ArenaAllocate(cardsPerPlayer, arena)});
  }

  Card deck[CARD_AMOUNT] = {0};
  for (CardRank rank = 0; rank < RANK_AMOUNT; rank++) {
    for (CardSuit suit = 0; suit < SUIT_AMOUNT; suit++) {
      deck[(RANK_AMOUNT * suit) + rank] = (Card){rank, suit};
    }
  }

  // Deal random hands to players
  int count = CARD_AMOUNT;
  for (int32_t playerIndex = 0; playerIndex < playerCount; playerIndex++) {
    Player* player = PlayerArray_Get(&gameContext.players, playerIndex);
    for (int i = 0; i < cardsPerPlayer; i++) {
      uint32_t cardIndex = pcg32Boundedrand(count);

      Card card = deck[cardIndex];
      deck[cardIndex] = deck[count - 1];
      count -= 1;

      CardArray_InsertSorted(&player->hand, card, Card_Gt);
    }
  }

  return gameContext;
}

bool areAllCardsSame(CardArray* hand, CardIndexArray* selectedIndexes) {
  Card first =
      CardArray_GetValue(hand, CardIndexArray_GetValue(selectedIndexes, 0));
  for (int i = 1; i < selectedIndexes->length; i++) {
    if (!Card_EqRank(CardArray_GetValue(
                         hand, CardIndexArray_GetValue(selectedIndexes, i)),
                     first))
      return false;
  }
  return true;
}

bool isFlush(CardArray* hand, CardIndexArray* selectedIndexes) {
  Card first =
      CardArray_GetValue(hand, CardIndexArray_GetValue(selectedIndexes, 0));
  for (int i = 1; i < selectedIndexes->length; i++) {
    if (!Card_EqSuit(CardArray_GetValue(
                         hand, CardIndexArray_GetValue(selectedIndexes, i)),
                     first))
      return false;
  }
  return true;
}

// TODO: doesn't work because we no longer sort by rank
bool isStraight(CardArray* hand, CardIndexArray* selectedIndexes) {
  CardRank latestRank =
      CardArray_GetValue(hand, CardIndexArray_GetValue(selectedIndexes, 0))
          .rank;
  for (int i = 1; i < selectedIndexes->length; i++) {
    if (((CardArray_GetValue(hand, CardIndexArray_GetValue(selectedIndexes, i))
              .rank -
          latestRank) %
         RANK_AMOUNT) != 1)
      return false;
    latestRank += 1;
  }
  return true;
}

// TODO: doesn't work because we no longer sort by rank
bool isFullHouse(CardArray* hand, CardIndexArray* selectedIndexes) {
  Card first =
      CardArray_GetValue(hand, CardIndexArray_GetValue(selectedIndexes, 0));
  Card second =
      CardArray_GetValue(hand, CardIndexArray_GetValue(selectedIndexes, 1));
  Card third =
      CardArray_GetValue(hand, CardIndexArray_GetValue(selectedIndexes, 2));
  Card fourth =
      CardArray_GetValue(hand, CardIndexArray_GetValue(selectedIndexes, 3));
  Card last =
      CardArray_GetValue(hand, CardIndexArray_GetValue(selectedIndexes, 4));

  return (Card_EqRank(second, first) && Card_EqRank(fourth, last) &&
          (Card_EqRank(third, first) || Card_EqRank(third, last)));
}

// TODO: doesn't work because we no longer sort by rank
bool isFourOfAKind(CardArray* hand, CardIndexArray* selectedIndexes) {
  Card first =
      CardArray_GetValue(hand, CardIndexArray_GetValue(selectedIndexes, 0));
  Card second =
      CardArray_GetValue(hand, CardIndexArray_GetValue(selectedIndexes, 1));
  Card last =
      CardArray_GetValue(hand, CardIndexArray_GetValue(selectedIndexes, 4));
  Card rank = Card_EqRank(first, second) ? first : last;
  for (int i = 1; i < selectedIndexes->length - 1; i++) {
    if (!Card_EqRank(rank, CardArray_GetValue(hand, CardIndexArray_GetValue(
                                                        selectedIndexes, i))))
      return false;
  }
  return true;
}

HandKind handKind(CardArray* hand, CardIndexArray* selectedIndexes) {
  switch (selectedIndexes->length) {
    case 0:
      return NO_HAND;
    case 1:
      return HIGH_CARD;
    case 2:
      return areAllCardsSame(hand, selectedIndexes) ? PAIR : NO_HAND;
    case 3:
      return areAllCardsSame(hand, selectedIndexes) ? THREE_OF_A_KIND : NO_HAND;
    case 4:
      return NO_HAND;
    case 5: {
      if (isFlush(hand, selectedIndexes))
        return isStraight(hand, selectedIndexes) ? STRAIGHT_FLUSH : FLUSH;

      if (isStraight(hand, selectedIndexes))
        return STRAIGHT;

      if (isFullHouse(hand, selectedIndexes))
        return FULL_HOUSE;

      if (isFourOfAKind(hand, selectedIndexes))
        return FOUR_OF_A_KIND;

      return NO_HAND;
    }
  }

  assert(0);
  return NO_HAND;
}
