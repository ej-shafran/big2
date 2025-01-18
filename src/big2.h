#ifndef BIG2_H_
#define BIG2_H_

#define PACKED __attribute__((__packed__))

#define CARD_AMOUNT 52

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

void printCard(Card card);

void printDeck(void);

#endif  // BIG2_H_
