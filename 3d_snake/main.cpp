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

// main.cpp 是整个程序的入口。
// 这里不要写太多游戏细节，只保留“启动、更新、绘制、关闭”四个主流程。
// 真正的游戏规则都放在其它模块里，这样新手可以先看懂整体流程。

// 函数作用：初始化窗口、资源、默认设置和游戏初始状态。
void InitProgram(GameContext &context)
{
    // 让 rand() 每次运行时产生不同随机序列。
    // 食物位置、疯狂模式事件都会用到 rand()。
    srand((unsigned int)time(nullptr)); // 用当前时间做随机种子，让每次运行的随机结果不同。

    // raylib 的窗口初始化。
    // SetTargetFPS(60) 表示游戏希望每秒刷新 60 次。
    SetConfigFlags(FLAG_MSAA_4X_HINT); // 开启抗锯齿，让 3D 方块边缘更平滑。
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "3D Snake Game"); // 创建游戏窗口。
    SetTargetFPS(60); // 固定目标帧率，计时器会更稳定。

    // 摄像机和图片资源只需要在程序开始时准备一次。
    UpdateCameraPosition(context); // 摄像机只需要初始化一次。
    LoadAssets(context); // 背景图片等资源只需要加载一次。

    // 最高分从文件读取。
    // Loading 计时器从 0 开始，表示启动时不显示 Loading。
    context.highScore = LoadHighScore(); // 从 highscore.txt 读取历史最高分。
    context.loading.timer = 0.0f; // 0 表示当前不显示 Loading 黑屏。

    // 设置默认关卡、速度、难度和模式。
    SetDefaultSettings(context); // 设置默认关卡、速度、难度、模式。

    // 游戏启动后先显示菜单，而不是直接开始游戏。
    context.game.status = LevelSelect; // 程序启动后先进入菜单界面。
}

// 函数作用：每一帧更新输入、Loading、游戏规则和疯狂模式。
void UpdateProgram(GameContext &context)
{
    // 每一帧都按这个顺序更新。
    // 先处理输入，再处理各种计时和游戏规则。
    HandleInput(context); // 先读输入，因为输入会改变方向、暂停、返回菜单。
    UpdateLoading(context); // Loading 只是一个覆盖层，不改变游戏规则。
    UpdateGame(context); // 普通贪吃蛇移动、碰撞、吃食物。
    UpdateEffects(context); // 吃食物爆炸、分数闪烁等短时间视觉效果。
    UpdateCrazyMode(context); // 疯狂模式的加速和蓝色方块。
}

// 函数作用：每一帧绘制背景、3D 场景、UI 和 Loading 覆盖层。
void DrawProgram(GameContext &context)
{
    // BeginDrawing 和 EndDrawing 必须成对出现。
    // 所有 2D/3D 绘制都写在它们中间。
    BeginDrawing(); // raylib 要求所有绘制都写在 BeginDrawing 和 EndDrawing 之间。
    ClearBackground((Color){20, 25, 40, 255}); // 默认底色，背景图失败时也不会是脏画面。

    if (context.game.status != LevelSelect)
    {
        // 游戏运行界面先画背景图，再进入 3D 模式画地图和蛇。
        DrawBackgroundImage(context); // 游戏运行界面也使用同一张背景图。

        BeginMode3D(context.camera); // 进入 3D 绘制模式。
        DrawGame3D(context); // 画地图、蛇、食物、疯狂模式方块。
        EndMode3D(); // 退出 3D 模式，之后继续画 2D UI。
    }

    // UI 永远最后画，这样文字和菜单会盖在 3D 场景上面。
    DrawUI(context); // UI 最后画，保证文字在场景上方。
    DrawLoadingScreen(context); // Loading 更最后画，保证黑屏能盖住所有内容。

    EndDrawing(); // 结束本帧绘制。
}

// 函数作用：程序退出前释放资源并关闭窗口。
void CloseProgram(GameContext &context)
{
    // 手动加载的贴图需要手动释放。
    // CloseWindow 放在最后。
    UnloadAssets(context); // 释放手动加载的图片资源。
    CloseWindow(); // 关闭 raylib 窗口。
}

// 函数作用：程序入口，创建总数据并运行主循环。
int main()
{
    // GameContext 是“游戏总数据包”。
    // 所有模块都通过它读写游戏状态，避免到处散落全局变量。
    GameContext context = {}; // 使用 {} 把所有字段初始化为 0/false/空容器。

    InitProgram(context); // 初始化窗口、资源、设置。

    // raylib 的主循环。
    // 只要窗口没有被关闭，就一直更新并绘制。
    while (!WindowShouldClose())
    {
        UpdateProgram(context); // 每帧先更新数据。
        DrawProgram(context); // 再根据最新数据绘制画面。
    }

    CloseProgram(context); // 程序退出前清理资源。

    return 0;
}
