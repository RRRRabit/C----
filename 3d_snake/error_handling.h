#pragma once

#include "raylib.h"

// 图片加载结果。
// loaded 表示是否加载成功。
// hadError 表示加载过程中是否出现错误。
struct TextureLoadResult
{
    Texture2D texture;
    bool loaded;
    bool hadError;
};

// 函数作用：用 throw/catch 安全加载图片资源。
TextureLoadResult LoadTextureWithExceptionHandling(const char *fileName);

// 函数作用：用 throw/catch 安全读取最高分文件。
int LoadHighScoreWithExceptionHandling(const char *fileName);
