#pragma once

#include "types.h"

// 最高分文件模块：读取、保存、更新 highscore.txt。
int LoadHighScore();                                // 从 highscore.txt 读取最高分。
void SaveHighScore(const GameContext &context);     // 把最高分写入 highscore.txt。
void UpdateHighScore(GameContext &context);         // 如果当前分数更高，就更新并保存。
