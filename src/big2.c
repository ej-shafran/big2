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
}

bool CardIndex_Gt(int32_t a, int32_t b) {
  return a > b;
}

const Card THREE_OF_DIAMONDS = {.rank = RANK_3, .suit = DIAMONDS};

GameContext generateGame(uint64_t seed, int32_t playerCount, Arena* arena) {
  char* seedString = Arena_Allocate(20 * sizeof(char), arena);
  int32_t seedStringLength = sprintf(seedString, "%" PRIx64, seed);

  GameContext gameContext = {
      .players = PlayerArray_ArenaAllocate(playerCount, arena),
      .selectedCardIndexes = CardIndexArray_ArenaAllocate(MAX_HAND_SIZE, arena),
      .lastPlayedHand = CardArray_ArenaAllocate(MAX_HAND_SIZE, arena),
      .selectedHandKind = NO_HAND,
      .seedString = seedString,
      .seedStringLength = seedStringLength,
      .currentPlayerIndex = 0,
      .winnerIndex = -1,
      .skippedCount = 0,
  };

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

      if (Card_Eq(card, THREE_OF_DIAMONDS))
        gameContext.currentPlayerIndex = playerIndex;

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

bool isStraight(CardArray* hand, CardIndexArray* selectedIndexes) {
  // Use a bit-set to mark which ranks exist in the selected cards
  int16_t rankBitSet = 0;
  // Keep track of the minimum rank encountered
  CardRank minRank = RANK_AMOUNT;
  for (int32_t i = 0; i < selectedIndexes->length; i++) {
    int32_t index = CardIndexArray_GetValue(selectedIndexes, i);
    CardRank rank = CardArray_GetValue(hand, index).rank;
    if (rank < minRank)
      minRank = rank;
    rankBitSet |= 1 << rank;
  }
  // If the selected cards form a straight, the bit-set will be a sequence of 5
  // 1s starting at the minimum rank's bit
  int16_t targetBitMask = 0x1f /* 0b11111 */ << minRank;
  return (rankBitSet & targetBitMask) == targetBitMask;
}

bool isFullHouse(CardArray* hand, CardIndexArray* selectedIndexes) {
  // NOTE: since this in only run on hands that aren't four-of-a-kind,
  // we just check that there are only two distinct ranks in the hand

  int32_t firstIndex = CardIndexArray_GetValue(selectedIndexes, 0);
  CardRank first = CardArray_GetValue(hand, firstIndex).rank;
  CardRank second = RANK_AMOUNT;

  for (int32_t i = 1; i < selectedIndexes->length; i++) {
    int32_t index = CardIndexArray_GetValue(selectedIndexes, i);
    CardRank rank = CardArray_GetValue(hand, index).rank;
    if (rank != first) {
      if (second == RANK_AMOUNT) {
        second = rank;
      } else if (rank != second) {
        return false;
      }
    }
  }

  return true;
}

bool isFourOfAKind(CardArray* hand, CardIndexArray* selectedIndexes) {
  // A four-of-a-kind is made of four cards of the "main" rank,
  // and one card of a "filler" rank
  int32_t firstIndex = CardIndexArray_GetValue(selectedIndexes, 0);
  CardRank main = CardArray_GetValue(hand, firstIndex).rank;
  CardRank filler = RANK_AMOUNT;
  // Keep track of whether or not we've seen multiple instances of `main`;
  // if we haven't, it may be the filler card
  bool seenMultipleMain = false;

  for (int32_t i = 1; i < selectedIndexes->length; i++) {
    int32_t index = CardIndexArray_GetValue(selectedIndexes, i);
    CardRank rank = CardArray_GetValue(hand, index).rank;
    if (rank == main) {
      // What we have in `main` is for sure the main rank
      seenMultipleMain = true;
    } else if (filler == RANK_AMOUNT) {
      // We don't know what the filler rank is yet,
      // so set this rank as the filler rank
      filler = rank;
    } else if (!seenMultipleMain && rank == filler) {
      // `main` is actually the `filler`, and vice-versa
      filler = main;
      main = rank;
      seenMultipleMain = true;
    } else {
      // We've seen multiple non-main cards
      return false;
    }
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

      if (isFourOfAKind(hand, selectedIndexes))
        return FOUR_OF_A_KIND;

      if (isFullHouse(hand, selectedIndexes))
        return FULL_HOUSE;

      return NO_HAND;
    }
  }

  assert(0);
  return NO_HAND;
}

bool isSelectedHandPlayable(GameContext* gameContext) {
  if (gameContext->selectedHandKind == NO_HAND)
    return false;
  if (gameContext->lastPlayedHandKind == NO_HAND)
    return true;

  if (gameContext->selectedCardIndexes.length !=
      gameContext->lastPlayedHand.length)
    return false;

  if (gameContext->selectedHandKind != gameContext->lastPlayedHandKind)
    return gameContext->selectedHandKind > gameContext->lastPlayedHandKind;

  int32_t highestSelectedIndex =
      CardIndexArray_GetValue(&gameContext->selectedCardIndexes,
                              gameContext->selectedCardIndexes.length - 1);
  Player* currentPlayer =
      PlayerArray_Get(&gameContext->players, gameContext->currentPlayerIndex);
  CardArray* hand = &currentPlayer->hand;
  Card highestSelectedCard = CardArray_GetValue(hand, highestSelectedIndex);
  Card highestLastPlayedCard = CardArray_GetValue(
      &gameContext->lastPlayedHand, gameContext->lastPlayedHand.length - 1);

  switch (gameContext->selectedHandKind) {
    case HAND_KIND_AMOUNT:
    case NO_HAND:
      assert(0);
      return false;
    case HIGH_CARD:
    case PAIR:
    case THREE_OF_A_KIND: {
      int32_t selectedValue =
          (highestSelectedCard.rank * SUIT_AMOUNT) + highestSelectedCard.suit;
      int32_t lastPlayedValue = (highestLastPlayedCard.rank * SUIT_AMOUNT) +
                                highestLastPlayedCard.suit;
      return selectedValue > lastPlayedValue;
    } break;
    default:
      break;
  }

  return Card_Gt(highestSelectedCard, highestLastPlayedCard);
}
