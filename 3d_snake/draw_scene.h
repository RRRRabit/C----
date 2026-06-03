#pragma once

#include "types.h"

// 3D 场景绘制模块。
void UpdateCameraPosition(GameContext &context);
Vector3 CellToWorld(const GameContext &context, Cell cell);
Color GetSnakeColor(const GameContext &context, Color normalColor);

void DrawGame3D(const GameContext &context);
void DrawGridGround(const GameContext &context);
void DrawWalls(const GameContext &context);
void DrawSnake(const GameContext &context);
void DrawSnakeHead(const GameContext &context, Vector3 pos);
void DrawSnakeBody(const GameContext &context, Vector3 pos, int index, int totalLength);
void DrawFood(const GameContext &context);
void DrawEatEffect(const GameContext &context);
void DrawCrazyBlocks(const GameContext &context);
