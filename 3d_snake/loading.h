#pragma once

#include "types.h"

void StartLoading(GameContext &context);
void UpdateLoading(GameContext &context);
bool IsLoading(const GameContext &context);
void DrawLoadingScreen(const GameContext &context);
