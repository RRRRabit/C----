#pragma once

#include "types.h"

void DrawUI(const GameContext &context);
void DrawLevelSelectUI(const GameContext &context);
void DrawCenterMessage(const char *title, const char *subtitle, Color titleColor);
void DrawButton(Rectangle button, const char *text, bool selected);
bool IsButtonClicked(Rectangle button);

Rectangle GetLevelButtonRect(int index);
Rectangle GetSpeedMinusButtonRect();
Rectangle GetSpeedPlusButtonRect();
Rectangle GetDifficultyMinusButtonRect();
Rectangle GetDifficultyPlusButtonRect();
Rectangle GetCrazyModeButtonRect();
Rectangle GetStartButtonRect();
