#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "big2.h"
#include "utils/pcg_basic.h"

#define SEQ 54u

uint64_t generateSeed(void) {
  pcg32_random_t rng = PCG32_INITIALIZER;
  int rounds = 5;
  pcg32Srandom_r(&rng, time(NULL) ^ (intptr_t)&printf, (intptr_t)&rounds);
  return pcg32Random_r(&rng);
}

const char* shiftArg(int* argc, const char*** argv) {
  const char* arg = **argv;
  *argc -= 1;
  *argv += 1;
  return arg;
}

void printPlayerCards(CardArray player,
                      uint8_t selectedCardIndexes[CARD_AMOUNT / MIN_PLAYERS],
                      uint8_t selectedCardCount) {
  for (int cardIndex = 0; cardIndex < player.cardCount; cardIndex++) {
    printf("   ");
    bool isSelected = false;
    for (int i = 0; i < selectedCardCount; i++) {
      if (selectedCardIndexes[i] == cardIndex) {
        isSelected = true;
        break;
      }
    }
    if (isSelected) {
      printf("\033[41;30m");
    }
    printf("%02d", cardIndex + 1);
    printf("\033[0m");
    if (cardIndex != player.cardCount - 1)
      printf(" ");
  }
  printf("\n");
  for (int cardIndex = 0; cardIndex < player.cardCount; cardIndex++) {
    printf("  ");
    printCard(player.cards[cardIndex]);
    if (cardIndex != player.cardCount - 1)
      printf(" ");
  }
  printf("\n");
}

int main(int argc, const char** argv) {
  uint8_t playerCount = 4;
  uint64_t deterministicSeed = 0;

  shiftArg(&argc, &argv);

  while (argc > 0) {
    const char* arg = shiftArg(&argc, &argv);
    if (strcmp(arg, "--seed") == 0) {
      if (argc == 0) {
        fprintf(stderr, "missing argument for --seed\n");
        return EXIT_FAILURE;
      }

      const char* seedString = shiftArg(&argc, &argv);
      char* endPointer;
      errno = 0;
      deterministicSeed = strtoul(seedString, &endPointer, 16);
      if (errno != 0) {
        perror(NULL);
        return EXIT_FAILURE;
      }

      if (endPointer == seedString) {
        fprintf(stderr, "invalid seed string\n");
        return EXIT_FAILURE;
      }
    } else if (strcmp(arg, "--player-count") == 0) {
      if (argc == 0) {
        fprintf(stderr, "missing argument for --player-count\n");
        return EXIT_FAILURE;
      }

      const char* playerCountString = shiftArg(&argc, &argv);
      char* endPointer;
      errno = 0;
      playerCount = strtoul(playerCountString, &endPointer, 10);
      if (errno != 0) {
        perror(NULL);
        return EXIT_FAILURE;
      }

      if (endPointer == playerCountString || playerCount > MAX_PLAYERS ||
          playerCount < MIN_PLAYERS) {
        fprintf(stderr, "invalid player count string\n");
        return EXIT_FAILURE;
      }
    } else {
      fprintf(stderr, "unrecognized argument %s\n", arg);
      return EXIT_FAILURE;
    }
  }

  uint64_t seed = deterministicSeed == 0 ? generateSeed() : deterministicSeed;
  pcg32Srandom(seed, SEQ);
  printf("Seed: %lx\n\n", seed);

  GameContext gameContext = generateGame(playerCount);

  uint8_t selectedCardIndexes[CARD_AMOUNT / 2] = {0};
  uint8_t selectedCardCount = 0;

  printf("It is player %d's turn\n", gameContext.currentPlayerIndex);
  CardArray player = gameContext.players[gameContext.currentPlayerIndex];
  printPlayerCards(player, selectedCardIndexes, selectedCardCount);

  char* line;
  do {
    printf("Enter what to do: ");
    size_t lineLength;
    ssize_t numberRead = getline(&line, &lineLength, stdin);
    if (numberRead == -1) {
      fprintf(stderr, "no input provided\n");
      return EXIT_FAILURE;
    }

    if (strcmp(line, "done\n") == 0)
      break;

    char* endPointer;
    errno = 0;
    uint64_t playedCardNumber = strtoul(line, &endPointer, 10);
    if (errno != 0) {
      perror(NULL);
      return EXIT_FAILURE;
    }

    if (endPointer == line || playedCardNumber > player.cardCount) {
      fprintf(stderr, "invalid card index string\n");
      return EXIT_FAILURE;
    }

    ssize_t alreadySelectedIndex = -1;
    for (int i = 0; i < selectedCardCount; i++) {
      if (selectedCardIndexes[i] == playedCardNumber - 1) {
        alreadySelectedIndex = i;
        break;
      }
    }
    if (alreadySelectedIndex != -1) {
      selectedCardIndexes[alreadySelectedIndex] =
          selectedCardIndexes[--selectedCardCount];
    } else {
      selectedCardIndexes[selectedCardCount++] = playedCardNumber - 1;
    }

    printPlayerCards(player, selectedCardIndexes, selectedCardCount);
  } while (true);

  free(line);
  return 0;
}
