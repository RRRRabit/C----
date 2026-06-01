#pragma once

#include "types.h"

// Loading 模块：进入游戏和返回菜单时的黑屏过渡。
void StartLoading(GameContext &context);                // 开始一次 Loading 过渡。
void UpdateLoading(GameContext &context);               // 每帧减少 Loading 倒计时。
bool IsLoading(const GameContext &context);             // 判断 Loading 是否还在显示。
void DrawLoadingScreen(const GameContext &context);     // 绘制黑屏和 LOADING 文字。
