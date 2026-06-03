#pragma once

#include "types.h"

// Loading 模块：进入游戏和返回菜单时的黑屏过渡。
void StartLoading(GameContext &context);
void UpdateLoading(GameContext &context);
bool IsLoading(const GameContext &context);
void DrawLoadingScreen(const GameContext &context);
