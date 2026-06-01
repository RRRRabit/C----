#pragma once

#include "types.h"

// 贪吃蛇核心规则模块：移动、碰撞、吃食物、生成食物。
void InitGame(GameContext &context);                // 初始化或重开一局游戏。
void StartSelectedGame(GameContext &context);       // 使用菜单设置开始游戏。
void UpdateGame(GameContext &context);              // 每帧更新普通游戏逻辑。
void UpdateEffects(GameContext &context);           // 每帧更新临时视觉效果计时器。

bool IsSameCell(Cell a, Cell b);                                    // 判断两个格子是否相同。
bool IsCellOnSnake(const GameContext &context, Cell cell);          // 判断格子是否在蛇身上。
bool IsCellOnCrazyBlock(const GameContext &context, Cell cell);     // 判断格子是否有蓝色方块。
bool IsCellOnSolidCrazyBlock(const GameContext &context, Cell cell); // 判断格子是否有实体蓝方块。

bool CheckWallCollision(const GameContext &context, Cell head);             // 判断蛇头是否出地图。
bool CheckSelfCollision(const GameContext &context, Cell head, bool willGrow); // 判断蛇头是否撞到身体。
bool CanPassWall(const GameContext &context);                              // 判断当前是否允许穿墙。

Cell GetNextHead(const GameContext &context);                       // 根据当前方向计算下一格蛇头。
void WrapHeadIfNeeded(GameContext &context, Cell &head);            // 疯狂模式加速时处理穿墙。
bool CheckGameOver(GameContext &context, Cell head, bool willGrow); // 判断这次移动是否失败。
void EatFoodIfNeeded(GameContext &context, Cell head);              // 如果吃到食物，处理加分和增长。
void UpdateSnakeLength(GameContext &context);                       // 根据 growLeft 决定是否删除尾巴。
void MoveSnake(GameContext &context);                               // 让蛇移动一格。
void SpawnFood(GameContext &context);                               // 在空格子随机生成食物。
float GetCurrentMoveInterval(const GameContext &context);           // 计算当前移动间隔。
