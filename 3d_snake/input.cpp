#include "input.h"
#include "constants.h"
#include "draw_ui.h"
#include "loading.h"
#include "settings.h"
#include "snake_logic.h"

// input.cpp 只负责“玩家输入转成游戏动作”。
// 菜单界面主要看鼠标，游戏界面主要看键盘。

// 函数作用：根据当前游戏状态处理键盘和鼠标输入。
void HandleInput(GameContext &context)
{
    // Loading 期间不响应输入，避免玩家在黑屏时误操作。
    if (IsLoading(context))
    {
        return;
    }

    // 菜单状态下，只处理菜单按钮。
    if (context.game.status == LevelSelect)
    {
        HandleLevelSelectInput(context);
        return;
    }

    // M：从游戏返回菜单。
    if (IsKeyPressed(KEY_M))
    {
        context.game.status = LevelSelect; // 切回菜单状态。
        context.game.started = false; // 下次进游戏时重新等待方向键开始。
        context.game.snake.clear(); // 菜单不需要保留旧蛇身。
        context.game.food = {-1, -1}; // 菜单不显示旧食物。

        StartLoading(context); // 返回菜单时显示黑屏过渡。
        return;
    }

    // 方向键和 WASD 都可以控制蛇。
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))
    {
        TryChangeDirection(context, {0, -1}); // z 减少表示向上。
    }

    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN))
    {
        TryChangeDirection(context, {0, 1}); // z 增加表示向下。
    }

    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT))
    {
        TryChangeDirection(context, {-1, 0}); // x 减少表示向左。
    }

    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT))
    {
        TryChangeDirection(context, {1, 0}); // x 增加表示向右。
    }

    // 空格：暂停和继续。
    if (IsKeyPressed(KEY_SPACE))
    {
        if (context.game.status == Playing)
        {
            context.game.status = Paused; // Playing 按空格变暂停。
        }
        else if (context.game.status == Paused)
        {
            context.game.status = Playing; // Paused 按空格继续。
        }
    }

    // R：重新开始当前设置下的一局游戏。
    if (IsKeyPressed(KEY_R))
    {
        InitGame(context); // 保留当前菜单设置，重新开一局。
    }
}

// 函数作用：处理主菜单里的按钮点击。
void HandleLevelSelectInput(GameContext &context)
{
    // 关卡按钮从左到右对应 Level 1 到 Level 4。
    for (int i = 0; i < LEVEL_COUNT; i++)
    {
        if (IsButtonClicked(GetLevelButtonRect(i)))
        {
            SetLevel(context, i + 1); // i 从 0 开始，所以关卡号要加 1。
        }
    }

    // 速度和难度都用加减按钮控制。
    // 真正的范围限制放在 SetSpeedByLevel 和下面的 if 里。
    if (IsButtonClicked(GetSpeedMinusButtonRect()))
    {
        SetSpeedByLevel(context, context.settings.speedLevel - 1); // 降低一级速度。
    }

    if (IsButtonClicked(GetSpeedPlusButtonRect()))
    {
        SetSpeedByLevel(context, context.settings.speedLevel + 1); // 提高一级速度。
    }

    if (IsButtonClicked(GetDifficultyMinusButtonRect()))
    {
        context.settings.difficultyLevel--; // 难度减 1。

        if (context.settings.difficultyLevel < MIN_DIFFICULTY_LEVEL)
        {
            context.settings.difficultyLevel = MIN_DIFFICULTY_LEVEL; // 不允许低于最小难度。
        }

        UpdateTargetScore(context); // 难度变化后目标分也要重算。
    }

    if (IsButtonClicked(GetDifficultyPlusButtonRect()))
    {
        context.settings.difficultyLevel++; // 难度加 1。

        if (context.settings.difficultyLevel > MAX_DIFFICULTY_LEVEL)
        {
            context.settings.difficultyLevel = MAX_DIFFICULTY_LEVEL; // 不允许高于最大难度。
        }

        UpdateTargetScore(context); // 难度变化后目标分也要重算。
    }

    // Crazy Mode 是开关按钮，点击一次取反。
    if (IsButtonClicked(GetCrazyModeButtonRect()))
    {
        context.settings.crazyMode = !context.settings.crazyMode; // true 变 false，false 变 true。
    }

    // 点击开始后，先初始化游戏，再显示 Loading 过渡。
    if (IsButtonClicked(GetStartButtonRect()))
    {
        StartSelectedGame(context); // 用当前菜单设置初始化游戏。
        StartLoading(context); // 进入游戏时显示黑屏过渡。
    }
}

// 函数作用：尝试改变蛇的方向，并阻止直接反向移动。
void TryChangeDirection(GameContext &context, Cell newDirection)
{
    // 贪吃蛇不能直接掉头。
    // 例如正在向右走时，不能直接改成向左。
    bool xWillCancel = newDirection.x + context.game.direction.x == 0; // 左右方向相反时和为 0。
    bool zWillCancel = newDirection.z + context.game.direction.z == 0; // 上下方向相反时和为 0。
    bool isReverseDirection = xWillCancel && zWillCancel; // 两个坐标都抵消，说明是直接掉头。

    if (isReverseDirection)
    {
        return;
    }

    // 方向输入不会立刻移动蛇，只是记录到 nextDirection。
    // 下一次 MoveSnake 时才真正使用。
    context.game.nextDirection = newDirection; // 记录方向，等下一次 MoveSnake 生效。
    context.game.started = true; // 第一次输入方向后，蛇开始移动。
}
