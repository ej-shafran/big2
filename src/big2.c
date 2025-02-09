#include "big2.h"
#include <assert.h>
#include <stdint.h>
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

void printHandKind(HandKind kind) {
  switch (kind) {
    case NO_HAND:
      printf("no hand");
      break;
    case HIGH_CARD:
      printf("high card");
      break;
    case PAIR:
      printf("pair");
      break;
    case THREE_OF_A_KIND:
      printf("three of a kind");
      break;
    case STRAIGHT:
      printf("straight");
      break;
    case FLUSH:
      printf("flush");
      break;
    case FULL_HOUSE:
      printf("full house");
      break;
    case FOUR_OF_A_KIND:
      printf("four of a kind");
      break;
    case STRAIGHT_FLUSH:
      printf("straight flush");
      break;
  }
}

GameContext generateGame(uint8_t playerCount) {
  GameContext gameContext = {
      .players =
          {
              .items = {0},
              .count = playerCount,
          },
      .currentPlayerIndex = 0,
  };
  for (int i = 0; i < playerCount; i++) {
    Player player = {
        .hand = (CardArray){.count = 0, .items = {{0}}},
    };
    gameContext.players.items[i] = player;
  }

  // Create the standard deck of cards
  Card deck[CARD_AMOUNT];
  for (int rank = 0; rank < RANK_AMOUNT; rank++) {
    for (int suit = 0; suit < SUIT_AMOUNT; suit++) {
      Card card = {.rank = rank, .suit = suit};
      int i = (RANK_AMOUNT * suit) + rank;
      deck[i] = card;
    }
  }

  // Deal random hands to players
  int cardsPerPlayer = CARD_AMOUNT / playerCount;
  int count = CARD_AMOUNT;
  for (int player = 0; player < playerCount; player++) {
    for (int i = 0; i < cardsPerPlayer; i++) {
      int cardIndex = pcg32Boundedrand(count);

      Card card = deck[cardIndex];
      deck[cardIndex] = deck[count - 1];
      count -= 1;

      gameContext.players.items[player]
          .hand.items[gameContext.players.items[player].hand.count++] = card;
    }
  }

  return gameContext;
}

bool areAllCardsSame(PlayedCardHand hand) {
  Card first = hand.items[0];
  for (int i = 1; i < hand.count; i++) {
    if (hand.items[i].rank != first.rank)
      return false;
  }
  return true;
}

bool isFlush(PlayedCardHand hand) {
  CardSuit firstSuit = hand.items[0].suit;
  for (int i = 1; i < hand.count; i++) {
    if (hand.items[i].suit != firstSuit)
      return false;
  }
  return true;
}

bool isStraight(PlayedCardHand hand) {
  CardRank lastRank = hand.items[0].rank;
  for (int i = 1; i < hand.count; i++) {
    if (((hand.items[i].rank - lastRank) % RANK_AMOUNT) != 1)
      return false;
    lastRank += 1;
  }
  return true;
}

bool isFullHouse(PlayedCardHand hand) {
  CardRank first = hand.items[0].rank;
  CardRank last = hand.items[4].rank;

  return (first == hand.items[1].rank && last == hand.items[3].rank &&
          (first == hand.items[2].rank || last == hand.items[2].rank));
}

bool isFourOfAKind(PlayedCardHand hand) {
  CardRank first = hand.items[0].rank;
  CardRank last = hand.items[4].rank;
  CardRank rank = hand.items[1].rank == first ? first : last;
  for (int i = 1; i < hand.count - 1; i++) {
    if (hand.items[i].rank != rank)
      return false;
  }
  return true;
}

uint8_t quicksortHandPartition(PlayedCardHand* hand,
                               uint8_t low,
                               uint8_t high) {
  Card pivot = hand->items[high];
  uint8_t i = low;
  for (uint8_t j = low; j < high; j++) {
    if (hand->items[j].rank <= pivot.rank) {
      Card temp = hand->items[i];
      hand->items[i] = hand->items[j];
      hand->items[j] = temp;
      i += 1;
    }
  }
  Card temp = hand->items[i];
  hand->items[i] = hand->items[high];
  hand->items[high] = temp;
  return i;
}

void quicksortHand(PlayedCardHand* hand, uint8_t low, uint8_t high) {
  if (low >= high)
    return;

  uint8_t partitionIndex = quicksortHandPartition(hand, low, high);
  quicksortHand(hand, low, partitionIndex == 0 ? 0 : partitionIndex - 1);
  quicksortHand(hand, partitionIndex + 1, high);
}

HandKind handKind(PlayedCardHand hand) {
  quicksortHand(&hand, 0, hand.count - 1);

  switch (hand.count) {
    case 1:
      return HIGH_CARD;
    case 2:
      return areAllCardsSame(hand) ? PAIR : NO_HAND;
    case 3:
      return areAllCardsSame(hand) ? THREE_OF_A_KIND : NO_HAND;
    case 4:
      return NO_HAND;
    case 5: {
      if (isFlush(hand))
        return isStraight(hand) ? STRAIGHT_FLUSH : FLUSH;

      if (isStraight(hand))
        return STRAIGHT;

      if (isFullHouse(hand))
        return FULL_HOUSE;

      if (isFourOfAKind(hand))
        return FOUR_OF_A_KIND;

      return NO_HAND;
    }
  }

  assert(0);
  return NO_HAND;
}
