#include "error_handling.h"

#include <fstream>
#include <stdexcept>

// error_handling.cpp 集中放简单异常处理。
// 其它文件只调用这里的函数，不直接写 try/catch。
// 这样新手阅读时，可以很清楚地知道异常处理都在哪里。

// 函数作用：把最高分文件重写成一个安全的默认分数。
void RewriteHighScoreFileToDefault(const char *fileName)
{
    std::ofstream outputFile(fileName);

    if (!outputFile.is_open())
    {
        // 如果连重写都失败，就抛出异常。
        // 外层 catch 会接住它，游戏仍然使用默认最高分 0。
        throw std::runtime_error("Can not rewrite high score file.");
    }

    outputFile << 0;
    outputFile.close();
}

// 函数作用：用 throw/catch 安全加载图片资源。
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
        // 图片加载失败不应该让游戏崩溃。
        // 这里只记录失败状态，之后绘制时会用纯色背景兜底。
        result.texture = {};
        result.loaded = false;
        result.hadError = true;
    }

    return result;
}

// 函数作用：用 throw/catch 安全读取最高分文件。
int LoadHighScoreWithExceptionHandling(const char *fileName)
{
    int savedScore = 0;

    try
    {
        std::ifstream inputFile(fileName);

        if (!inputFile.is_open())
        {
            // 文件不存在时不算严重错误。
            // 第一次运行游戏很可能没有 highscore.txt，直接使用 0。
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
        // 如果文件内容不是数字，或者分数是负数，就把它当成坏文件。
        // 游戏继续运行，最高分回到 0，同时尝试把文件重写成 0。
        savedScore = 0;

        try
        {
            RewriteHighScoreFileToDefault(fileName);
        }
        catch (...)
        {
            // 如果重写失败，也不让游戏崩溃。
            // 最高分仍然使用内存里的 0。
            savedScore = 0;
        }
    }

    return savedScore;
}
