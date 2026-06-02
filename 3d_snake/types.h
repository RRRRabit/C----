#pragma once

#include "raylib.h"
#include <list>
#include <vector>

// types.h 只放数据结构，不放具体规则。
// 新手可以先看这个文件，了解游戏需要保存哪些数据。

// 地图格子坐标。
// x 表示左右方向，z 表示前后方向。
struct Cell
{
    int x;
    int z;
};

// 疯狂模式里的蓝色方块。
// solid 为 false 时只是闪烁预警。
// solid 为 true 时会撞死蛇。
struct CrazyBlock
{
    Cell cell;
    bool solid;
    float flashTimer;
};

enum GameStatus
{
    // 正在菜单界面。
    LevelSelect,

    // 正在游戏。
    Playing,

    // 暂停。
    Paused,

    // 游戏失败。
    GameOver,

    // 普通模式达到目标分数，或者地图被蛇占满。
    Win
};

// 一档速度设置包含三个值：
// startInterval：刚开始多久移动一格。
// minInterval：速度再快也不能小于这个间隔。
// speedUpAmount：每次分数提高后减少多少移动间隔。
struct SpeedSetting
{
    float startInterval;
    float minInterval;
    float speedUpAmount;
};

// 玩家在菜单里选择的设置，以及由设置计算出的参数。
struct GameSettings
{
    int currentLevel;
    int gridSize;
    int speedLevel;
    int difficultyLevel;
    int targetScore;
    bool crazyMode;
    SpeedSetting speed;
};

// 一局游戏中会变化的主要数据。
struct GameData
{
    // snake.front() 永远是蛇头。
    // std::list 是动态链表，适合“头部插入、尾部删除”的蛇身移动。
    std::list<Cell> snake;

    // direction 是当前正在移动的方向。
    // nextDirection 是玩家刚输入、下一次移动时才生效的方向。
    Cell direction;
    Cell nextDirection;

    // 当前食物所在格子。
    // 如果 x 或 z 为 -1，表示当前没有食物。
    Cell food;

    GameStatus status;
    int score;

    // moveTimer 用来累计时间。
    // 累计到移动间隔后，蛇才真正走一格。
    float moveTimer;

    // 玩家第一次按方向键之前，蛇不自动移动。
    bool started;

    // 当前食物是否是金色超级果子。
    bool isSuperFood;

    // growLeft 表示接下来还有几次移动不删除尾巴。
    // 不删除尾巴，蛇就会变长。
    int growLeft;
};

// 临时视觉效果的数据。
// 它们只影响显示，不决定游戏胜负。
struct EffectData
{
    float eatTimer;
    float scoreFlashTimer;
    Cell lastFoodCell;
};

// 疯狂模式的额外状态。
// 普通模式下这些数据基本不会起作用。
struct CrazyModeData
{
    // 蓝色障碍方块。
    std::vector<CrazyBlock> blocks;

    // 下一次随机疯狂事件的倒计时。
    float eventTimer;

    // boostTimer > 0 时，蛇处于加速状态。
    float boostTimer;

    // 红光是加速前的预警。
    float redFlashTimer;

    // 绿光是加速结束后的恢复提示。
    float greenFlashTimer;

    // 红光结束后是否准备进入加速。
    bool boostWaiting;
};

// 图片资源。
struct AssetData
{
    Texture2D menuBackground;
    bool menuBackgroundLoaded;

    // 背景图加载失败时为 true。
    // UI 会根据它显示一行提示文字。
    bool menuBackgroundLoadFailed;
};

// Loading 过渡只需要一个倒计时。
struct LoadingData
{
    float timer;
};

// GameContext 是全项目最重要的结构。
// 它把所有游戏数据集中放在一起，然后传给各个模块使用。
// 这样不用写一大堆全局变量，也方便知道数据从哪里来。
struct GameContext
{
    GameSettings settings;
    GameData game;
    EffectData effects;
    CrazyModeData crazy;
    AssetData assets;
    LoadingData loading;
    Camera3D camera;
    int highScore;
};
