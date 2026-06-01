#include "score_file.h"
#include "constants.h"
#include <fstream>

int LoadHighScore()
{
    int savedScore = 0;

    std::ifstream inputFile(HIGH_SCORE_FILE);

    if (inputFile.is_open())
    {
        inputFile >> savedScore;
        inputFile.close();
    }

    return savedScore;
}

void SaveHighScore(const GameContext &context)
{
    std::ofstream outputFile(HIGH_SCORE_FILE);

    if (outputFile.is_open())
    {
        outputFile << context.highScore;
        outputFile.close();
    }
}

void UpdateHighScore(GameContext &context)
{
    if (context.game.score > context.highScore)
    {
        context.highScore = context.game.score;
        SaveHighScore(context);
    }
}
