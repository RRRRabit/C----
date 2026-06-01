#pragma once

#include "types.h"

// 资源模块：加载、释放和绘制背景图片。
void LoadAssets(GameContext &context);                  // 程序启动时加载图片资源。
void UnloadAssets(GameContext &context);                // 程序退出前释放图片资源。
void DrawBackgroundImage(const GameContext &context);    // 绘制菜单和游戏界面的背景图。
