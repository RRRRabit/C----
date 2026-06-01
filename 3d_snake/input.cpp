#include "input.h"
#include "constants.h"
#include "draw_ui.h"
#include "loading.h"
#include "settings.h"
#include "snake_logic.h"

void HandleInput(GameContext &context)
{
    if (IsLoading(context))
    {
        return;
    }

    if (context.game.status == LevelSelect)
    {
        HandleLevelSelectInput(context);
        return;
    }

    if (IsKeyPressed(KEY_M))
    {
        context.game.status = LevelSelect;
        context.game.started = false;
        context.game.snake.clear();
        context.game.food = {-1, -1};

        StartLoading(context);
        return;
    }

    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))
    {
        TryChangeDirection(context, {0, -1});
    }

    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN))
    {
        TryChangeDirection(context, {0, 1});
    }

    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT))
    {
        TryChangeDirection(context, {-1, 0});
    }

    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT))
    {
        TryChangeDirection(context, {1, 0});
    }

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

    if (IsKeyPressed(KEY_R))
    {
        InitGame(context);
    }
}

void HandleLevelSelectInput(GameContext &context)
{
    for (int i = 0; i < LEVEL_COUNT; i++)
    {
        if (IsButtonClicked(GetLevelButtonRect(i)))
        {
            SetLevel(context, i + 1);
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

    if (IsButtonClicked(GetCrazyModeButtonRect()))
    {
        context.settings.crazyMode = !context.settings.crazyMode;
    }

    if (IsButtonClicked(GetStartButtonRect()))
    {
        StartSelectedGame(context);
        StartLoading(context);
    }
}

void TryChangeDirection(GameContext &context, Cell newDirection)
{
    bool xWillCancel = newDirection.x + context.game.direction.x == 0;
    bool zWillCancel = newDirection.z + context.game.direction.z == 0;
    bool isReverseDirection = xWillCancel && zWillCancel;

    if (isReverseDirection)
    {
        return;
    }

    context.game.nextDirection = newDirection;
    context.game.started = true;
}
