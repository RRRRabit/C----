#pragma once

#include "types.h"

int LoadHighScore();
void SaveHighScore(const GameContext &context);
void UpdateHighScore(GameContext &context);
