#include "settings.h"
#include "constants.h"

// settings.cpp 负责“菜单选项 -> 实际游戏参数”。
// 例如玩家选 Level 3，这里会把地图大小设置为 20 x 20。

// 函数作用：计算主菜单左上角的 x 坐标，让菜单水平居中。
int GetMenuX()
{
    // 菜单水平居中。
    return (SCREEN_WIDTH - MENU_WIDTH) / 2; // 屏幕宽度减菜单宽度，再除以 2 就是居中 x。
}

// 函数作用：根据关卡编号返回地图边长。
int GetMapSizeByLevel(int level)
{
    // 关卡只决定地图大小，不决定速度和目标分数。
    if (level == 1)
    {
        return 10; // Level 1：10 x 10，小地图。
    }

    if (level == 2)
    {
        return 15; // Level 2：15 x 15。
    }

    if (level == 3)
    {
        return 20; // Level 3：20 x 20。
    }

    if (level == 4)
    {
        return 25; // Level 4：25 x 25，大地图。
    }

    return DEFAULT_GRID_SIZE; // 非法关卡使用默认地图大小兜底。
}

// 函数作用：返回当前关卡的显示文字。
std::string GetLevelName(const GameContext &context)
{
    return "Level " + std::to_string(context.settings.currentLevel); // 把数字关卡拼成显示文本。
}

// 函数作用：设置当前关卡，并同步地图大小和目标分。
void SetLevel(GameContext &context, int level)
{
    // 如果传入错误关卡，就回到 Level 1。
    if (level < 1)
    {
        level = 1; // 小于 1 的非法值改回 Level 1。
    }

    if (level > LEVEL_COUNT)
    {
        level = 1; // 超过关卡数量的非法值也改回 Level 1。
    }

    context.settings.currentLevel = level; // 保存当前关卡编号。
    context.settings.gridSize = GetMapSizeByLevel(level); // 根据关卡编号更新地图大小。

    UpdateTargetScore(context); // 地图大小变了，目标分也要跟着变。
}

// 函数作用：根据速度等级设置蛇的移动速度参数。
void SetSpeedByLevel(GameContext &context, int speedLevel)
{
    // 先把速度等级限制在合法范围内。
    if (speedLevel < MIN_SPEED_LEVEL)
    {
        speedLevel = MIN_SPEED_LEVEL; // 速度不能低于最小档。
    }

    if (speedLevel > MAX_SPEED_LEVEL)
    {
        speedLevel = MAX_SPEED_LEVEL; // 速度不能高于最大档。
    }

    context.settings.speedLevel = speedLevel; // 保存菜单上显示的速度档位。

    // 不同速度等级对应不同移动间隔。
    // 间隔越小，蛇移动越快。
    if (speedLevel == 1)
    {
        context.settings.speed = {0.28f, 0.14f, 0.006f}; // 慢速：起步慢，最低速度也较慢。
    }
    else if (speedLevel == 2)
    {
        context.settings.speed = {0.22f, 0.11f, 0.008f}; // 较慢。
    }
    else if (speedLevel == 3)
    {
        context.settings.speed = {0.18f, 0.08f, 0.010f}; // 默认速度。
    }
    else if (speedLevel == 4)
    {
        context.settings.speed = {0.14f, 0.07f, 0.012f}; // 较快。
    }
    else
    {
        context.settings.speed = {0.10f, 0.05f, 0.014f}; // 快速：移动间隔最短。
    }
}

// 函数作用：根据地图大小和难度等级计算目标分数。
void UpdateTargetScore(GameContext &context)
{
    // 目标分数和地图大小、难度等级有关。
    context.settings.targetScore =
        context.settings.gridSize * // 地图越大，目标分越高。
        context.settings.difficultyLevel * // 难度越高，目标分越高。
        2; // 简单倍率，避免目标分计算太复杂。
}

// 函数作用：设置游戏启动时的默认菜单选项。
void SetDefaultSettings(GameContext &context)
{
    // 游戏启动时的默认菜单设置。
    context.settings.currentLevel = 1; // 默认选 Level 1。
    context.settings.difficultyLevel = 3; // 默认中等难度。
    context.settings.crazyMode = false; // 默认普通模式。

    SetLevel(context, 1); // 同步地图大小和目标分。
    SetSpeedByLevel(context, 3); // 默认速度等级 3。
}
