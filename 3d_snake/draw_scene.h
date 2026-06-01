#pragma once

#include "types.h"

// 3D 场景绘制模块：地图、墙、蛇、食物、疯狂模式方块。
void UpdateCameraPosition(GameContext &context);                            // 设置固定摄像机位置。
Vector3 CellToWorld(const GameContext &context, Cell cell);                 // 把地图格子转换成 3D 坐标。
Color GetSnakeColor(const GameContext &context, Color normalColor);         // 根据疯狂模式状态决定蛇颜色。

void DrawGame3D(const GameContext &context);                                 // 绘制整个 3D 游戏场景。
void DrawGridGround(const GameContext &context);                             // 绘制棋盘格地面。
void DrawWalls(const GameContext &context);                                  // 绘制地图边界墙。
void DrawSnake(const GameContext &context);                                  // 绘制整条蛇。
void DrawSnakeHead(const GameContext &context, Vector3 pos);                 // 绘制蛇头和眼睛。
void DrawSnakeBody(const GameContext &context, Vector3 pos, int index, int totalLength); // 绘制一节蛇身。
void DrawFood(const GameContext &context);                                   // 绘制当前食物。
void DrawEatEffect(const GameContext &context);                              // 绘制吃食物后的爆炸圈。
void DrawCrazyBlocks(const GameContext &context);                            // 绘制疯狂模式蓝色方块。
