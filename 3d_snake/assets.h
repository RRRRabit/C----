#pragma once

#include "types.h"

// 资源模块：加载、释放、绘制背景图。
void LoadAssets(GameContext &context);
void UnloadAssets(GameContext &context);
void DrawBackgroundImage(const GameContext &context);
