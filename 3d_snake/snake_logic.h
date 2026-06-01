#pragma once

#include "types.h"

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
