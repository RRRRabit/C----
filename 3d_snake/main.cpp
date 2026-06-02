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

// “启动、更新、绘制、关闭”四个主流程。

// 函数作用：初始化窗口、资源、默认设置和游戏初始状态。
void InitProgram(GameContext &context)
{
    srand((unsigned int)time(nullptr)); // 用当前时间做随机种子。

    // raylib 的窗口初始化。
    SetConfigFlags(FLAG_MSAA_4X_HINT); // 开启抗锯齿。
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "3D Snake Game");
    SetTargetFPS(60);

    UpdateCameraPosition(context);
    LoadAssets(context); // 背景图片加载。

    context.highScore = LoadHighScore();
    context.loading.timer = 0.0f; // Loading 计时器从 0 开始，表示启动时不显示 Loading。

    // 设置默认关卡、速度、难度和模式。
    SetDefaultSettings(context);

    context.game.status = LevelSelect; // 显示菜单。
}

// 函数作用：每一帧更新输入、Loading、游戏规则和疯狂模式。
void UpdateProgram(GameContext &context)
{
    HandleInput(context); // 先读输入。
    UpdateLoading(context);
    UpdateGame(context);
    UpdateEffects(context); // 视觉效果。
    UpdateCrazyMode(context);
}

// 函数作用：每一帧绘制背景、3D 场景、UI 和 Loading 覆盖层。
void DrawProgram(GameContext &context)
{
    BeginDrawing();                            // raylib 要求所有绘制都写在 BeginDrawing 和 EndDrawing 之间。
    ClearBackground((Color){20, 25, 40, 255}); // 默认底色。

    if (context.game.status != LevelSelect)
    {
        // 游戏运行界面先画背景图，再进入 3D 模式画地图和蛇。
        DrawBackgroundImage(context);

        BeginMode3D(context.camera);
        DrawGame3D(context);
        EndMode3D();
    }

    // UI 永远最后画，这样文字和菜单会盖在 3D 场景上面。
    DrawUI(context);
    DrawLoadingScreen(context);

    EndDrawing();
}

// 函数作用：程序退出前释放资源并关闭窗口。
void CloseProgram(GameContext &context)
{
    UnloadAssets(context); // 释放手动加载的图片资源。
    CloseWindow();
}

// 函数作用：程序入口，创建总数据并运行主循环。
int main()
{
    // GameContext 是“游戏总数据包”。
    GameContext context = {};

    InitProgram(context);

    // raylib 的主循环。
    // 只要窗口没有被关闭，就一直更新并绘制。
    while (!WindowShouldClose())
    {
        UpdateProgram(context);
        DrawProgram(context);
    }

    CloseProgram(context);

    return 0;
}
