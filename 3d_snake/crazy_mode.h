#pragma once

#include "types.h"

// 疯狂模式模块：红光预警、加速、绿光恢复、蓝色方块。
void UpdateCrazyMode(GameContext &context);                     // 每帧更新疯狂模式总入口。
void UpdateBoostEvent(GameContext &context, float deltaTime);   // 更新红光、加速、绿光三个计时器。
void UpdateBlueBlocks(GameContext &context, float deltaTime);   // 更新蓝色方块从预警到实体的过程。
void TryStartCrazyEvent(GameContext &context);                  // 事件倒计时结束后尝试触发新事件。
void TriggerRandomCrazyEvent(GameContext &context);             // 随机选择加速事件或蓝方块事件。
void StartBoostEvent(GameContext &context);                     // 开始红光预警，之后进入加速。
void StartBlueBlockEvent(GameContext &context);                 // 在空格子生成一个蓝色方块。
bool IsBoostEventBusy(const GameContext &context);              // 判断加速事件流程是否还没结束。
