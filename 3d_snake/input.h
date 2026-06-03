#pragma once

#include "types.h"

// 输入模块。
void HandleInput(GameContext &context);
void HandleLevelSelectInput(GameContext &context);
void TryChangeDirection(GameContext &context, Cell newDirection);
