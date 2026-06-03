#include "assets.h"
#include "constants.h"
#include "error_handling.h"

// 图片资源管理。

// 加载图片资源。
void LoadAssets(GameContext &context)
{
    // throw/catch 在 error_handling.cpp。
    TextureLoadResult result = LoadTextureWithExceptionHandling(MENU_BACKGROUND_FILE);

    context.assets.menuBackground = result.texture;
    context.assets.menuBackgroundLoaded = result.loaded;
    context.assets.menuBackgroundLoadFailed = result.hadError;
}

// 释放图片资源。
void UnloadAssets(GameContext &context)
{
    if (context.assets.menuBackgroundLoaded)
    {
        UnloadTexture(context.assets.menuBackground);
        context.assets.menuBackgroundLoaded = false;
        context.assets.menuBackgroundLoadFailed = false;
    }
}

// 绘制全屏背景图。
void DrawBackgroundImage(const GameContext &context)
{
    // 背景图加载失败时，用纯色背景兜底，避免程序崩溃或黑屏。
    if (!context.assets.menuBackgroundLoaded)
    {
        ClearBackground((Color){13, 18, 30, 255});
        return;
    }

    // source：从原图取整张。
    Rectangle source = {
        0.0f,
        0.0f,
        (float)context.assets.menuBackground.width,
        (float)context.assets.menuBackground.height};

    // target：拉伸到整个窗口。
    Rectangle target = {
        0.0f,
        0.0f,
        (float)SCREEN_WIDTH,
        (float)SCREEN_HEIGHT};

    DrawTexturePro(
        context.assets.menuBackground,
        source,
        target,
        (Vector2){0.0f, 0.0f},
        0.0f,
        WHITE);
}
