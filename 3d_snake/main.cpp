#include "assets.h"
#include "constants.h"
#include "crazy_mode.h"
#include "draw_scene.h"
#include "draw_ui.h"
#include "input.h"
#include "loading.h"
#include "score_file.h"
#include "settings.h"
#include "snake_logic.h"

#include <cstdlib>
#include <ctime>

// 主流程：启动、更新、绘制、关闭。

// 初始化程序。
void InitProgram(GameContext &context)
{
    srand((unsigned int)time(nullptr));

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "3D Snake Game");
    SetTargetFPS(60);

    UpdateCameraPosition(context);
    LoadAssets(context);

    context.highScore = LoadHighScore();
    context.loading.timer = 0.0f;

    SetDefaultSettings(context);

    context.game.status = LevelSelect;
}

// 每帧更新。
void UpdateProgram(GameContext &context)
{
    HandleInput(context);
    UpdateLoading(context);
    UpdateGame(context);
    UpdateEffects(context);
    UpdateCrazyMode(context);
}

// 每帧绘制。
void DrawProgram(GameContext &context)
{
    BeginDrawing();
    ClearBackground((Color){20, 25, 40, 255});

    if (context.game.status != LevelSelect)
    {
        DrawBackgroundImage(context);

        BeginMode3D(context.camera);
        DrawGame3D(context);
        EndMode3D();
    }

    // UI 最后画，盖在 3D 场景上面。
    DrawUI(context);
    DrawLoadingScreen(context);

    EndDrawing();
}

// 释放资源并关闭窗口。
void CloseProgram(GameContext &context)
{
    UnloadAssets(context);
    CloseWindow();
}

// 程序入口。
int main()
{
    GameContext context = {};

    InitProgram(context);

    while (!WindowShouldClose())
    {
        UpdateProgram(context);
        DrawProgram(context);
    }

    CloseProgram(context);

    return 0;
}
