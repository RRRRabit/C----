#include "assets.h"
#include "constants.h"

void LoadAssets(GameContext &context)
{
    context.assets.menuBackground = LoadTexture(MENU_BACKGROUND_FILE);
    context.assets.menuBackgroundLoaded = context.assets.menuBackground.id != 0;
}

void UnloadAssets(GameContext &context)
{
    if (context.assets.menuBackgroundLoaded)
    {
        UnloadTexture(context.assets.menuBackground);
        context.assets.menuBackgroundLoaded = false;
    }
}

void DrawBackgroundImage(const GameContext &context)
{
    if (!context.assets.menuBackgroundLoaded)
    {
        ClearBackground((Color){13, 18, 30, 255});
        return;
    }

    Rectangle source = {
        0.0f,
        0.0f,
        (float)context.assets.menuBackground.width,
        (float)context.assets.menuBackground.height};

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
