#include "score_file.h"
#include "constants.h"
#include "error_handling.h"
#include <fstream>

// 最高分文件读写。

// 读取最高分。
int LoadHighScore()
{
    // throw/catch 在 error_handling.cpp。
    return LoadHighScoreWithExceptionHandling(HIGH_SCORE_FILE);
}

// 保存最高分。
void SaveHighScore(const GameContext &context)
{
    std::ofstream outputFile(HIGH_SCORE_FILE);

    if (outputFile.is_open())
    {
        outputFile << context.highScore;
        outputFile.close();
    }
}

// 分数超过最高分时更新文件。
void UpdateHighScore(GameContext &context)
{
    if (context.game.score > context.highScore)
    {
        context.highScore = context.game.score;
        SaveHighScore(context);
    }
}
