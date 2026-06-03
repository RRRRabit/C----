#pragma once

#include "types.h"

// 疯狂模式模块：红光、加速、绿光、蓝方块。
void UpdateCrazyMode(GameContext &context);
void UpdateBoostEvent(GameContext &context, float deltaTime);
void UpdateBlueBlocks(GameContext &context, float deltaTime);
void TryStartCrazyEvent(GameContext &context);
void TriggerRandomCrazyEvent(GameContext &context);
void StartBoostEvent(GameContext &context);
void StartBlueBlockEvent(GameContext &context);
bool IsBoostEventBusy(const GameContext &context);
