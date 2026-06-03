#include "settings.h"
#include "constants.h"

// 菜单选项 -> 实际游戏参数。

// 计算菜单居中 x 坐标。
int GetMenuX()
{
    return (SCREEN_WIDTH - MENU_WIDTH) / 2;
}

// 根据关卡返回地图边长。
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

// 当前关卡显示文字。
std::string GetLevelName(const GameContext &context)
{
    return "Level " + std::to_string(context.settings.currentLevel);
}

// 设置关卡，并同步地图大小和目标分。
void SetLevel(GameContext &context, int level)
{
    // 如果传入错误关卡，就回到 Level 1。
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

// 根据速度等级设置移动速度。
void SetSpeedByLevel(GameContext &context, int speedLevel)
{
    // 先把速度等级限制在合法范围内。
    if (speedLevel < MIN_SPEED_LEVEL)
    {
        speedLevel = MIN_SPEED_LEVEL;
    }

    if (speedLevel > MAX_SPEED_LEVEL)
    {
        speedLevel = MAX_SPEED_LEVEL;
    }

    context.settings.speedLevel = speedLevel;

    // 不同速度等级对应不同移动间隔。
    // 间隔越小，蛇移动越快。
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

// 根据地图大小和难度计算目标分数。
void UpdateTargetScore(GameContext &context)
{
    // 目标分数和地图大小、难度等级有关。
    context.settings.targetScore =
        context.settings.gridSize *
        context.settings.difficultyLevel *
        2;
}

// 设置默认菜单选项。
void SetDefaultSettings(GameContext &context)
{
    context.settings.currentLevel = 1;
    context.settings.difficultyLevel = 3;
    context.settings.crazyMode = false;

    SetLevel(context, 1);
    SetSpeedByLevel(context, 3);
}
