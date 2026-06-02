#include "score_file.h"
#include "constants.h"
#include "error_handling.h"
#include <fstream>

// score_file.cpp 只负责最高分文件。
// 文件读写和游戏规则分开，代码更容易找。

// 函数作用：从文件读取历史最高分。
int LoadHighScore()
{
    // 具体 throw/catch 写在 error_handling.cpp 里。
    // 如果文件内容坏了，这里会拿到安全默认值 0。
    return LoadHighScoreWithExceptionHandling(HIGH_SCORE_FILE);
}

// 函数作用：把当前最高分保存到文件。
void SaveHighScore(const GameContext &context)
{
    // 每次保存都会覆盖原文件，只保留一个最高分数字。
    std::ofstream outputFile(HIGH_SCORE_FILE); // 打开文件用于写入，会覆盖旧内容。

    if (outputFile.is_open())
    {
        outputFile << context.highScore; // 只写入最高分数字。
        outputFile.close(); // 写完关闭文件。
    }
}

// 函数作用：当前分数超过最高分时更新并保存。
void UpdateHighScore(GameContext &context)
{
    // 只有当前分数超过最高分，才写文件。
    if (context.game.score > context.highScore)
    {
        context.highScore = context.game.score; // 内存里的最高分先更新。
        SaveHighScore(context); // 再保存到文件。
    }
}
