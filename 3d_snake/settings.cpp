#include "settings.h"
#include "constants.h"

int GetMenuX()
{
    return (SCREEN_WIDTH - MENU_WIDTH) / 2;
}

int GetMapSizeByLevel(int level)
{
    if (level == 1)
    {
        return 10;
    }

    if (level == 2)
    {
        return 15;
    }

    if (level == 3)
    {
        return 20;
    }

    if (level == 4)
    {
        return 25;
    }

    return DEFAULT_GRID_SIZE;
}

std::string GetLevelName(const GameContext &context)
{
    return "Level " + std::to_string(context.settings.currentLevel);
}

void SetLevel(GameContext &context, int level)
{
    if (level < 1)
    {
        level = 1;
    }

    if (level > LEVEL_COUNT)
    {
        level = 1;
    }

    context.settings.currentLevel = level;
    context.settings.gridSize = GetMapSizeByLevel(level);

    UpdateTargetScore(context);
}

void SetSpeedByLevel(GameContext &context, int speedLevel)
{
    if (speedLevel < MIN_SPEED_LEVEL)
    {
        speedLevel = MIN_SPEED_LEVEL;
    }

    if (speedLevel > MAX_SPEED_LEVEL)
    {
        speedLevel = MAX_SPEED_LEVEL;
    }

    context.settings.speedLevel = speedLevel;

    if (speedLevel == 1)
    {
        context.settings.speed = {0.28f, 0.14f, 0.006f};
    }
    else if (speedLevel == 2)
    {
        context.settings.speed = {0.22f, 0.11f, 0.008f};
    }
    else if (speedLevel == 3)
    {
        context.settings.speed = {0.18f, 0.08f, 0.010f};
    }
    else if (speedLevel == 4)
    {
        context.settings.speed = {0.14f, 0.07f, 0.012f};
    }
    else
    {
        context.settings.speed = {0.10f, 0.05f, 0.014f};
    }
}

void UpdateTargetScore(GameContext &context)
{
    context.settings.targetScore =
        context.settings.gridSize *
        context.settings.difficultyLevel *
        2;
}

void SetDefaultSettings(GameContext &context)
{
    context.settings.currentLevel = 1;
    context.settings.difficultyLevel = 3;
    context.settings.crazyMode = false;

    SetLevel(context, 1);
    SetSpeedByLevel(context, 3);
}
