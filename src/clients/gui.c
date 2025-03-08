#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include "../big2.h"
#include "../utils/pcg_basic.h"
#include "shared.h"

#define CLAY_IMPLEMENTATION
#include "../clay.h"
#include "../clay_renderer_raylib.c"

#define SEQ 54u

#if __APPLE__
#define KEY_LEFT_DEBUG_MODIFER KEY_LEFT_SUPER
#define KEY_RIGHT_DEBUG_MODIFER KEY_RIGHT_SUPER
#else
#define KEY_LEFT_DEBUG_MODIFER KEY_LEFT_CONTROL
#define KEY_RIGHT_DEBUG_MODIFER KEY_RIGHT_CONTROL
#endif  // __APPLE__

Arena contextArena = {0};
GameContext gameContext = {0};

// Textures
Texture2D SUIT_TO_ICON[SUIT_AMOUNT] = {};

// Colors
const Clay_Color BACKGROUND_COLOR = {0, 0, 0, 255};
const Clay_Color CONTAINER_BACKGROUND_COLOR = {122, 122, 122, 255};
const Clay_Color CARD_BACKGROUND_COLOR = {245, 245, 245, 255};
const Clay_Color UI_TEXT_COLOR = {255, 255, 255, 255};
const Clay_Color CARD_TEXT_COLOR = {15, 15, 15, 255};
const Clay_Color BUTTON_TEXT_COLOR = {255, 255, 255, 255};
const Clay_Color PRIMARY_COLOR = {102, 163, 255, 255};
const Clay_Color SECONDARY_COLOR = {255, 102, 102, 255};
const Clay_Color DARK_COLOR = {0, 0, 26, 255};

// Fonts & Text
const int FONT_ID_BODY_16 = 0;
const Clay_TextElementConfig UI_TEXT_CONFIG = {.fontId = FONT_ID_BODY_16,
                                               .fontSize = 40,
                                               .textColor = UI_TEXT_COLOR};
const Clay_TextElementConfig CARD_TEXT_CONFIG = {.fontId = FONT_ID_BODY_16,
                                                 .fontSize = 32,
                                                 .textColor = CARD_TEXT_COLOR};
const Clay_TextElementConfig BUTTON_TEXT_CONFIG = {
    .fontId = FONT_ID_BODY_16,
    .fontSize = 40,
    .textColor = BUTTON_TEXT_COLOR,
    .wrapMode = CLAY_TEXT_WRAP_WORDS};

// Layout
//   Sizing
const Clay_Sizing EXPAND_SIZING = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()};
const int CARD_WIDTH = 90;
const int CARD_HEIGHT = CARD_WIDTH * 14 / 9;
const Clay_Sizing CARD_SIZING = {
    .width = CLAY_SIZING_FIXED(CARD_WIDTH),
    .height = CLAY_SIZING_FIXED(CARD_HEIGHT),
};
const int SELECTED_CARD_WIDTH = 99;
const int SELECTED_CARD_HEIGHT = SELECTED_CARD_WIDTH * 14 / 9;
const Clay_Sizing SELECTED_CARD_SIZING = {
    .width = CLAY_SIZING_FIXED(SELECTED_CARD_WIDTH),
    .height = CLAY_SIZING_FIXED(SELECTED_CARD_HEIGHT)};
//   Gap
const int32_t CONTAINER_GAP = 32;
const int32_t CARD_GAP = 10;
const int32_t BUTTON_GAP = 10;
//   Alignment
const Clay_ChildAlignment CHILD_ALIGNMENT_CENTER = {.x = CLAY_ALIGN_X_CENTER,
                                                    .y = CLAY_ALIGN_Y_CENTER};
//   Padding
const Clay_Padding CONTAINER_PADDING = CLAY_PADDING_ALL(10);

// Shapes
const Clay_CornerRadius CONTAINER_CORNER_RADIUS = CLAY_CORNER_RADIUS(16);
const Clay_CornerRadius CARD_CORNER_RADIUS = CLAY_CORNER_RADIUS(8);
const Clay_CornerRadius BUTTON_CORNER_RADIUS = CLAY_CORNER_RADIUS(16);

// Text contents
const Clay_String RANK_TO_STRING[RANK_AMOUNT] = {
    [RANK_3] = CLAY_STRING("3"), [RANK_4] = CLAY_STRING("4"),
    [RANK_5] = CLAY_STRING("5"), [RANK_6] = CLAY_STRING("6"),
    [RANK_7] = CLAY_STRING("7"), [RANK_8] = CLAY_STRING("8"),
    [RANK_9] = CLAY_STRING("9"), [RANK_10] = CLAY_STRING("10"),
    [RANK_J] = CLAY_STRING("J"), [RANK_Q] = CLAY_STRING("Q"),
    [RANK_K] = CLAY_STRING("K"), [RANK_A] = CLAY_STRING("A"),
    [RANK_2] = CLAY_STRING("2"),
};
const Clay_String HAND_KIND_TO_STRING[HAND_KIND_AMOUNT] = {
    [NO_HAND] = CLAY_STRING(""),
    [HIGH_CARD] = CLAY_STRING("High Card"),
    [PAIR] = CLAY_STRING("Pair"),
    [THREE_OF_A_KIND] = CLAY_STRING("Three of a Kind"),
    [STRAIGHT] = CLAY_STRING("Straight"),
    [FLUSH] = CLAY_STRING("Flush"),
    [FULL_HOUSE] = CLAY_STRING("Full House"),
    [FOUR_OF_A_KIND] = CLAY_STRING("Four of a Kind"),
    [STRAIGHT_FLUSH] = CLAY_STRING("Straight Flush"),
};

void handleCardHover(Clay_ElementId elementId,
                     Clay_PointerData pointerData,
                     intptr_t userData) {
  int32_t index = userData;

  if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    Player* currentPlayer =
        PlayerArray_Get(&gameContext.players, gameContext.currentPlayerIndex);
    int32_t indexInSelectedCards = CardIndexArray_IndexOf(
        &gameContext.selectedCardIndexes, index, CardIndex_Eq);
    Card card = CardArray_GetValue(&currentPlayer->hand, index);
    if (indexInSelectedCards == -1) {
      if (gameContext.selectedCardIndexes.length < MAX_HAND_SIZE) {
        // Add to array
        CardIndexArray_Add(&gameContext.selectedCardIndexes, index);
        CardArray_Add(&gameContext.selectedCards, card);
        gameContext.selectedHandKind = handKind(&gameContext.selectedCards);
      }
    } else {
      // Remove from array
      CardIndexArray_RemoveSwapback(&gameContext.selectedCardIndexes,
                                    indexInSelectedCards);
      CardArray_RemoveSwapback(
          &gameContext.selectedCards,
          CardArray_IndexOf(&gameContext.selectedCards, card, Card_Eq));
      gameContext.selectedHandKind = handKind(&gameContext.selectedCards);
    }
  }
}

void renderCard(Card card, int32_t index) {
  int32_t isSelected = CardIndexArray_IndexOf(&gameContext.selectedCardIndexes,
                                              index, CardIndex_Eq) != -1;
  CLAY({
      .layout = {.sizing = isSelected ? SELECTED_CARD_SIZING : CARD_SIZING,
                 .childAlignment = CHILD_ALIGNMENT_CENTER,
                 .layoutDirection = CLAY_TOP_TO_BOTTOM},
      .cornerRadius = CARD_CORNER_RADIUS,
      .backgroundColor = CARD_BACKGROUND_COLOR,
  }) {
    Clay_OnHover(handleCardHover, index);
    CLAY_TEXT(RANK_TO_STRING[card.rank], CLAY_TEXT_CONFIG(CARD_TEXT_CONFIG));
    CLAY({.layout = {.sizing = {.width = CLAY_SIZING_FIXED(20),
                                .height = CLAY_SIZING_FIXED(30)}},
          .image = {&SUIT_TO_ICON[card.suit]}}) {}
  }
}

void renderSeed(void) {
  Clay_String seedString = {.chars = gameContext.seedString,
                            .length = gameContext.seedStringLength};
  CLAY({.layout = {.sizing = EXPAND_SIZING,
                   .childAlignment = CHILD_ALIGNMENT_CENTER}}) {
    CLAY_TEXT(CLAY_STRING("Seed: "), CLAY_TEXT_CONFIG(UI_TEXT_CONFIG));
    CLAY_TEXT(seedString, CLAY_TEXT_CONFIG(UI_TEXT_CONFIG));
  }
}

const Clay_Sizing sizingButton = {.width = CLAY_SIZING_GROW()};

Clay_ElementDeclaration buttonConfig(Clay_Color color) {
  return (Clay_ElementDeclaration){
      .layout =
          {
              .sizing = sizingButton,
              .childAlignment = CHILD_ALIGNMENT_CENTER,
              .padding = CLAY_PADDING_ALL(5),
          },
      .cornerRadius = CONTAINER_CORNER_RADIUS,
      .backgroundColor = color};
}

void clearSelectedCards(void) {
  CardIndexArray_Clear(&gameContext.selectedCardIndexes);
  CardArray_Clear(&gameContext.selectedCards);
  gameContext.selectedHandKind = NO_HAND;
}

void handleDeselectAllButtonHover(Clay_ElementId elementId,
                                  Clay_PointerData pointerData,
                                  intptr_t userData) {
  if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    clearSelectedCards();
  }
}

void handleSkipButtonHover(Clay_ElementId elementId,
                           Clay_PointerData pointerData,
                           intptr_t userData) {
  if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    clearSelectedCards();
    gameContext.currentPlayerIndex =
        (gameContext.currentPlayerIndex + 1) % gameContext.players.length;
  }
}

void renderActionButtons(void) {
  CLAY({.layout = {.sizing = EXPAND_SIZING,
                   .childGap = BUTTON_GAP,
                   .padding = CONTAINER_PADDING}}) {
    CLAY(buttonConfig(SECONDARY_COLOR)) {
      Clay_OnHover(handleSkipButtonHover, 0);
      CLAY_TEXT(CLAY_STRING("Skip"), CLAY_TEXT_CONFIG(BUTTON_TEXT_CONFIG));
    }
    CLAY(buttonConfig(PRIMARY_COLOR)) {
      Clay_OnHover(handleDeselectAllButtonHover, 0);
      CLAY_TEXT(CLAY_STRING("Deselect All"),
                CLAY_TEXT_CONFIG(BUTTON_TEXT_CONFIG));
    }
    CLAY(buttonConfig(DARK_COLOR)) {
      CLAY_TEXT(CLAY_STRING("Play"), CLAY_TEXT_CONFIG(BUTTON_TEXT_CONFIG));
    }
  }
}

bool triggeredDebugMode = false;

void toggleDebugMode(void) {
  bool isModifierDown =
      IsKeyDown(KEY_LEFT_DEBUG_MODIFER) || IsKeyDown(KEY_RIGHT_DEBUG_MODIFER);
  bool isShiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
  bool isIDown = IsKeyDown(KEY_I);
  bool pressingDebugKeys = isModifierDown && isShiftDown && isIDown;

  if (!triggeredDebugMode && pressingDebugKeys) {
    bool isDebugModeEnabled = Clay_IsDebugModeEnabled();
    TraceLog(LOG_INFO, "%s debug mode",
             isDebugModeEnabled ? "disabling" : "enabling");
    Clay_SetDebugModeEnabled(!isDebugModeEnabled);
    triggeredDebugMode = true;
  }

  if (triggeredDebugMode && !pressingDebugKeys) {
    triggeredDebugMode = false;
  }
}

void handleClayErrors(Clay_ErrorData errorData) {
  // See the Clay_ErrorData struct for more information
  TraceLog(LOG_ERROR, "%s", errorData.errorText.chars);
}

int gameLoop(void) {
  // Initialize Raylib
  Clay_Raylib_Initialize(1024, 768, "Big2", 0);

  // Initialize Clay
  size_t clayMemorySize = Clay_MinMemorySize();
  Clay_Arena clayArena = Clay_CreateArenaWithCapacityAndMemory(
      clayMemorySize, malloc(clayMemorySize));
  Clay_Initialize(clayArena,
                  (Clay_Dimensions){GetScreenWidth(), GetScreenHeight()},
                  (Clay_ErrorHandler){handleClayErrors});

  // Load textures
  SUIT_TO_ICON[DIAMONDS] = LoadTexture("resources/suits-diamonds.png");
  SUIT_TO_ICON[CLUBS] = LoadTexture("resources/suits-clubs.png");
  SUIT_TO_ICON[HEARTS] = LoadTexture("resources/suits-hearts.png");
  SUIT_TO_ICON[SPADES] = LoadTexture("resources/suits-spades.png");

  // Load fonts
  Font fonts[1];
  fonts[FONT_ID_BODY_16] =
      LoadFontEx("resources/Roboto-Regular.ttf", 48, 0, 400);

  // Connect fonts to Clay
  Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

  // Render loop
  while (!WindowShouldClose()) {
    // Update Clay with layout and mouse information
    Clay_SetLayoutDimensions(
        (Clay_Dimensions){GetScreenWidth(), GetScreenHeight()});
    Vector2 mousePosition = GetMousePosition();
    Vector2 scrollDelta = GetMouseWheelMoveV();
    Clay_SetPointerState((Clay_Vector2){mousePosition.x, mousePosition.y},
                         IsMouseButtonDown(0));
    Clay_UpdateScrollContainers(
        true, (Clay_Vector2){scrollDelta.x, scrollDelta.y}, GetFrameTime());

    Player* currentPlayer =
        PlayerArray_Get(&gameContext.players, gameContext.currentPlayerIndex);

    toggleDebugMode();

    Clay_BeginLayout();

    CLAY({.layout = {.sizing = EXPAND_SIZING,
                     .padding = CLAY_PADDING_ALL(32),
                     .childGap = CONTAINER_GAP,
                     .layoutDirection = CLAY_TOP_TO_BOTTOM},
          .backgroundColor = BACKGROUND_COLOR}) {
      // Card container
      CLAY({.layout = {.sizing = EXPAND_SIZING,
                       .padding = CONTAINER_PADDING,
                       .childGap = CARD_GAP,
                       .layoutDirection = CLAY_TOP_TO_BOTTOM},
            .cornerRadius = CONTAINER_CORNER_RADIUS,
            .backgroundColor = CONTAINER_BACKGROUND_COLOR}) {
        const int32_t CARDS_PER_ROW = 7;

        for (int32_t row = 0; row < currentPlayer->hand.length;
             row += CARDS_PER_ROW) {
          CLAY({.layout = {
                    .sizing = EXPAND_SIZING,
                    .childAlignment = CHILD_ALIGNMENT_CENTER,
                    .childGap = 10,
                }}) {
            for (int32_t i = row;
                 i < row + CARDS_PER_ROW && i < currentPlayer->hand.length;
                 i++) {
              renderCard(CardArray_GetValue(&currentPlayer->hand, i), i);
            }
          }
        }
      }

      CLAY({.layout = {.sizing = EXPAND_SIZING, .childGap = CONTAINER_GAP}}) {
        // Selected hand container
        CLAY({.layout = {.sizing = EXPAND_SIZING,
                         .padding = CONTAINER_PADDING,
                         .layoutDirection = CLAY_TOP_TO_BOTTOM,
                         .childAlignment = CHILD_ALIGNMENT_CENTER},
              .cornerRadius = CONTAINER_CORNER_RADIUS,
              .backgroundColor = CONTAINER_BACKGROUND_COLOR}) {
          CLAY() {
            CLAY_TEXT(CLAY_STRING("Player "), CLAY_TEXT_CONFIG(UI_TEXT_CONFIG));
            char playerNumber[1] = {'0' + (gameContext.currentPlayerIndex + 1)};
            Clay_String playerNumberString = {.chars = playerNumber,
                                              .length = 1};
            CLAY_TEXT(playerNumberString, CLAY_TEXT_CONFIG(UI_TEXT_CONFIG));
          }
          if (gameContext.selectedHandKind != NO_HAND) {
            CLAY_TEXT(HAND_KIND_TO_STRING[gameContext.selectedHandKind],
                      CLAY_TEXT_CONFIG(UI_TEXT_CONFIG));
          }
        }

        // Other UI
        CLAY({.layout = {.sizing = {.height = CLAY_SIZING_GROW(),
                                    .width = CLAY_SIZING_PERCENT(.75)},
                         .padding = CONTAINER_PADDING,
                         .layoutDirection = CLAY_TOP_TO_BOTTOM},
              .cornerRadius = CONTAINER_CORNER_RADIUS,
              .backgroundColor = CONTAINER_BACKGROUND_COLOR}) {
          renderSeed();
          renderActionButtons();
        }
      }
    }

    Clay_RenderCommandArray renderCommands = Clay_EndLayout();

    BeginDrawing();
    ClearBackground(BLACK);
    Clay_Raylib_Render(renderCommands, fonts);
    EndDrawing();
  }

  return 0;
}

int main(int argc, const char** argv) {
  Args args = {0};
  if (!parseArgs(argc, argv, &args))
    return EXIT_FAILURE;

  pcg32Srandom(args.seed, SEQ);
  printf("Seed: %" PRIx64 "\n\n", args.seed);

  size_t contextMemorySize = 2048;
  contextArena = CreateArenaWithCapacityAndMemory(contextMemorySize,
                                                  malloc(contextMemorySize));

  gameContext = generateGame(args.seed, args.playerCount, &contextArena);
  printf("%d\n", gameContext.players.length);

  return gameLoop();
}
