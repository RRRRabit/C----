#pragma once

#include "types.h"
#include <string>

// 设置模块：处理关卡、速度、难度和目标分数。
int GetMenuX();                                      // 计算菜单左上角 x 坐标，让菜单居中。
int GetMapSizeByLevel(int level);                    // 根据关卡编号返回地图大小。
std::string GetLevelName(const GameContext &context); // 返回当前关卡名称文本。

void SetLevel(GameContext &context, int level);              // 设置当前关卡和地图大小。
void SetSpeedByLevel(GameContext &context, int speedLevel);  // 设置当前速度等级对应的速度参数。
void UpdateTargetScore(GameContext &context);                // 根据地图大小和难度计算目标分数。
void SetDefaultSettings(GameContext &context);               // 设置游戏启动时的默认菜单参数。
