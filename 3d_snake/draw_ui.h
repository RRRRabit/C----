#pragma once

#include "types.h"

// 2D UI 绘制模块：菜单、按钮、分数、状态提示。
void DrawUI(const GameContext &context);                                             // 根据当前状态绘制菜单或游戏 UI。
void DrawLevelSelectUI(const GameContext &context);                                  // 绘制主菜单。
void DrawCenterMessage(const char *title, const char *subtitle, Color titleColor);   // 绘制暂停/失败/胜利弹窗。
void DrawButton(Rectangle button, const char *text, bool selected);                  // 绘制一个通用按钮。
bool IsButtonClicked(Rectangle button);                                              // 判断按钮是否被鼠标点击。

Rectangle GetLevelButtonRect(int index);             // 获取第 index 个关卡按钮的位置。
Rectangle GetSpeedMinusButtonRect();                 // 获取速度减号按钮的位置。
Rectangle GetSpeedPlusButtonRect();                  // 获取速度加号按钮的位置。
Rectangle GetDifficultyMinusButtonRect();            // 获取难度减号按钮的位置。
Rectangle GetDifficultyPlusButtonRect();             // 获取难度加号按钮的位置。
Rectangle GetCrazyModeButtonRect();                  // 获取 Crazy Mode 按钮的位置。
Rectangle GetStartButtonRect();                      // 获取 START 按钮的位置。
