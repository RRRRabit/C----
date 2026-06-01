#pragma once

#include "types.h"

// 输入模块：把键盘和鼠标操作转换成游戏动作。
void HandleInput(GameContext &context);                         // 每帧处理当前状态下的输入。
void HandleLevelSelectInput(GameContext &context);              // 处理菜单按钮点击。
void TryChangeDirection(GameContext &context, Cell newDirection); // 尝试改变蛇的方向。
