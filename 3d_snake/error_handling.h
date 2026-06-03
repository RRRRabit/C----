#pragma once

#include "raylib.h"

// 图片加载结果。
struct TextureLoadResult
{
    Texture2D texture;
    bool loaded;   // 是否加载成功。
    bool hadError; // 是否发生错误。
};

// 安全加载图片资源。
TextureLoadResult LoadTextureWithExceptionHandling(const char *fileName);

// 安全读取最高分。
int LoadHighScoreWithExceptionHandling(const char *fileName);
