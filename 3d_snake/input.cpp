#include "input.h"
#include "constants.h"
#include "draw_ui.h"
#include "loading.h"
#include "settings.h"
#include "snake_logic.h"

// 玩家输入转成游戏动作。

// 处理键盘和鼠标输入。
void HandleInput(GameContext &context)
{
    // Loading 期间不响应输入，避免玩家在黑屏时误操作。
    if (IsLoading(context))
    {
        return;
    }

    if (context.game.status == LevelSelect)
    {
        HandleLevelSelectInput(context);
        return;
    }

    // M：从游戏返回菜单。
    if (IsKeyPressed(KEY_M))
    {
        context.game.status = LevelSelect;
        context.game.started = false;
        context.game.snake.clear();
        context.game.food = {-1, -1};

        StartLoading(context);
        return;
    }

    // 方向键和 WASD 都可以控制蛇。
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))
    {
        TryChangeDirection(context, {0, -1}); // 上。
    }

    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN))
    {
        TryChangeDirection(context, {0, 1}); // 下。
    }

    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT))
    {
        TryChangeDirection(context, {-1, 0}); // 左。
    }

    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT))
    {
        TryChangeDirection(context, {1, 0}); // 右。
    }

    // 空格：暂停和继续。
    if (IsKeyPressed(KEY_SPACE))
    {
        if (context.game.status == Playing)
        {
            context.game.status = Paused;
        }
        else if (context.game.status == Paused)
        {
            context.game.status = Playing;
        }
    }

    // R：重新开始当前设置下的一局游戏。
    if (IsKeyPressed(KEY_R))
    {
        InitGame(context);
    }
}

// 处理主菜单按钮。
void HandleLevelSelectInput(GameContext &context)
{
    // 关卡按钮从左到右对应 Level 1 到 Level 4。
    for (int i = 0; i < LEVEL_COUNT; i++)
    {
        if (IsButtonClicked(GetLevelButtonRect(i)))
        {
            SetLevel(context, i + 1); // i 从 0 开始。
        }
    }

    if (IsButtonClicked(GetSpeedMinusButtonRect()))
    {
        SetSpeedByLevel(context, context.settings.speedLevel - 1);
    }

    if (IsButtonClicked(GetSpeedPlusButtonRect()))
    {
        SetSpeedByLevel(context, context.settings.speedLevel + 1);
    }

    if (IsButtonClicked(GetDifficultyMinusButtonRect()))
    {
        context.settings.difficultyLevel--;

        if (context.settings.difficultyLevel < MIN_DIFFICULTY_LEVEL)
        {
            context.settings.difficultyLevel = MIN_DIFFICULTY_LEVEL;
        }

        UpdateTargetScore(context);
    }

    if (IsButtonClicked(GetDifficultyPlusButtonRect()))
    {
        context.settings.difficultyLevel++;

        if (context.settings.difficultyLevel > MAX_DIFFICULTY_LEVEL)
        {
            context.settings.difficultyLevel = MAX_DIFFICULTY_LEVEL;
        }

        UpdateTargetScore(context);
    }

    // Crazy Mode 是开关按钮，点击一次取反。
    if (IsButtonClicked(GetCrazyModeButtonRect()))
    {
        context.settings.crazyMode = !context.settings.crazyMode;
    }

    // 点击开始后，先初始化游戏，再显示 Loading 过渡。
    if (IsButtonClicked(GetStartButtonRect()))
    {
        StartSelectedGame(context);
        StartLoading(context);
    }
}

// 改变方向，并阻止直接反向。
void TryChangeDirection(GameContext &context, Cell newDirection)
{
    // 贪吃蛇不能直接掉头。
    // 例如正在向右走时，不能直接改成向左。
    bool xWillCancel = newDirection.x + context.game.direction.x == 0;
    bool zWillCancel = newDirection.z + context.game.direction.z == 0;
    bool isReverseDirection = xWillCancel && zWillCancel;

    if (isReverseDirection)
    {
        return;
    }

    // 先存到 nextDirection，下一次 MoveSnake 才生效。
    context.game.nextDirection = newDirection;
    context.game.started = true;
}
