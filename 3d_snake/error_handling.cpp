#include "error_handling.h"

#include <fstream>
#include <stdexcept>

// 集中放简单异常处理。

// 重写默认最高分。
void RewriteHighScoreFileToDefault(const char *fileName)
{
    std::ofstream outputFile(fileName);

    if (!outputFile.is_open())
    {
        // 外层 catch 会接住，游戏继续用 0。
        throw std::runtime_error("Can not rewrite high score file.");
    }

    outputFile << 0;
    outputFile.close();
}

// 安全加载图片资源。
TextureLoadResult LoadTextureWithExceptionHandling(const char *fileName)
{
    TextureLoadResult result = {};

    try
    {
        result.texture = LoadTexture(fileName);

        if (result.texture.id == 0)
        {
            throw std::runtime_error("Texture load failed.");
        }

        result.loaded = true;
        result.hadError = false;
    }
    catch (...)
    {
        // 图片失败只记录状态，之后用纯色背景兜底。
        result.texture = {};
        result.loaded = false;
        result.hadError = true;
    }

    return result;
}

// 安全读取最高分。
int LoadHighScoreWithExceptionHandling(const char *fileName)
{
    int savedScore = 0;

    try
    {
        std::ifstream inputFile(fileName);

        if (!inputFile.is_open())
        {
            // 第一次运行可能还没有文件。
            return 0;
        }

        if (!(inputFile >> savedScore))
        {
            throw std::runtime_error("High score file content is not a number.");
        }

        if (savedScore < 0)
        {
            throw std::runtime_error("High score can not be negative.");
        }

        inputFile.close();
    }
    catch (...)
    {
        // 坏文件：最高分回到 0，并尝试修复文件。
        savedScore = 0;

        try
        {
            RewriteHighScoreFileToDefault(fileName);
        }
        catch (...)
        {
            savedScore = 0;
        }
    }

    return savedScore;
}
