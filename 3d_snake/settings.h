#pragma once

#include "types.h"
#include <string>

// 设置模块：关卡、速度、难度、目标分数。
int GetMenuX();
int GetMapSizeByLevel(int level);
std::string GetLevelName(const GameContext &context);

void SetLevel(GameContext &context, int level);
void SetSpeedByLevel(GameContext &context, int speedLevel);
void UpdateTargetScore(GameContext &context);
void SetDefaultSettings(GameContext &context);
