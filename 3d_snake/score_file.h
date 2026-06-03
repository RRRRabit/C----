#pragma once

#include "types.h"

// 最高分文件模块。
int LoadHighScore();
void SaveHighScore(const GameContext &context);
void UpdateHighScore(GameContext &context);
