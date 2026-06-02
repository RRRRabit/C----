#include "assets.h"
#include "constants.h"
#include "error_handling.h"

// assets.cpp 负责图片资源。
// 现在项目只有一张背景图，所以代码很短。

// 函数作用：加载游戏需要的图片资源。
void LoadAssets(GameContext &context)
{
    // 图片加载的 throw/catch 写在 error_handling.cpp 里。
    // 这里直接拿到加载结果，让 assets.cpp 保持简单。
    TextureLoadResult result = LoadTextureWithExceptionHandling(MENU_BACKGROUND_FILE);

    context.assets.menuBackground = result.texture;
    context.assets.menuBackgroundLoaded = result.loaded;
    context.assets.menuBackgroundLoadFailed = result.hadError;
}

// 函数作用：释放已经加载成功的图片资源。
void UnloadAssets(GameContext &context)
{
    // 只释放成功加载过的贴图。
    if (context.assets.menuBackgroundLoaded)
    {
        UnloadTexture(context.assets.menuBackground); // 释放显存里的贴图。
        context.assets.menuBackgroundLoaded = false; // 标记为已释放，避免重复释放。
        context.assets.menuBackgroundLoadFailed = false;
    }
}

// 函数作用：把背景图绘制到整个窗口上。
void DrawBackgroundImage(const GameContext &context)
{
    // 背景图加载失败时，用纯色背景兜底，避免程序崩溃或黑屏。
    if (!context.assets.menuBackgroundLoaded)
    {
        ClearBackground((Color){13, 18, 30, 255}); // 图片失败时使用深色背景兜底。
        return;
    }

    // source 表示从图片中取整张图。
    Rectangle source = {
        0.0f,
        0.0f,
        (float)context.assets.menuBackground.width, // 原图宽度。
        (float)context.assets.menuBackground.height}; // 原图高度。

    // target 表示把图片拉伸到整个窗口。
    Rectangle target = {
        0.0f,
        0.0f,
        (float)SCREEN_WIDTH, // 目标绘制宽度：整个窗口。
        (float)SCREEN_HEIGHT}; // 目标绘制高度：整个窗口。

    DrawTexturePro(
        context.assets.menuBackground,
        source,
        target,
        (Vector2){0.0f, 0.0f}, // 不额外偏移。
        0.0f, // 不旋转背景图。
        WHITE); // WHITE 表示保持图片原色。
}
