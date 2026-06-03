#pragma once

#include "types.h"

// 贪吃蛇核心规则模块。
class CollisionChecker
{
public:
    bool IsWallHit(const GameContext &context, Cell head) const;
    bool IsSelfHit(const GameContext &context, Cell head, bool willGrow) const;
    bool CanPassWall(const GameContext &context) const;
    bool IsGameOver(GameContext &context, Cell head, bool willGrow) const;
};

class SnakeMover
{
public:
    Cell GetNextHead(const GameContext &context) const;
    void WrapHeadIfNeeded(GameContext &context, Cell &head) const;
    void UpdateLength(GameContext &context) const;
    void MoveOneStep(GameContext &context) const;
};

class FoodSpawner
{
public:
    void Spawn(GameContext &context) const;

private:
    bool IsFoodCellSafe(const GameContext &context, Cell cell) const;
};

void InitGame(GameContext &context);
void StartSelectedGame(GameContext &context);
void UpdateGame(GameContext &context);
void UpdateEffects(GameContext &context);

bool IsSameCell(Cell a, Cell b);
bool IsCellOnSnake(const GameContext &context, Cell cell);
bool IsCellOnCrazyBlock(const GameContext &context, Cell cell);
bool IsCellOnSolidCrazyBlock(const GameContext &context, Cell cell);

bool CheckWallCollision(const GameContext &context, Cell head);
bool CheckSelfCollision(const GameContext &context, Cell head, bool willGrow);
bool CanPassWall(const GameContext &context);

Cell GetNextHead(const GameContext &context);
void WrapHeadIfNeeded(GameContext &context, Cell &head);
bool CheckGameOver(GameContext &context, Cell head, bool willGrow);
void EatFoodIfNeeded(GameContext &context, Cell head);
void UpdateSnakeLength(GameContext &context);
void MoveSnake(GameContext &context);
void SpawnFood(GameContext &context);
float GetCurrentMoveInterval(const GameContext &context);
