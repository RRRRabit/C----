#pragma once

#include "raylib.h"
#include <list>
#include <vector>

// 数据结构入口：先看本文件了解游戏保存了哪些数据。

// 地图格子坐标：x 左右，z 前后。
struct Cell
{
    int x;
    int z;
};

// 疯狂模式蓝方块。
struct CrazyBlock
{
    Cell cell;
    bool solid;       // false 预警，true 实体。
    float flashTimer; // 预警倒计时。
};

enum GameStatus
{
    LevelSelect,
    Playing,
    Paused,
    GameOver,
    Win
};

// 一档速度设置。
struct SpeedSetting
{
    float startInterval; // 初始移动间隔。
    float minInterval;   // 最快间隔。
    float speedUpAmount; // 每次提速减少多少间隔。
};

// 菜单设置。
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

    Cell direction;     // 当前移动方向。
    Cell nextDirection; // 下一次移动才生效。

    Cell food; // x 或 z 为 -1 表示没有食物。

    GameStatus status;
    int score;

    float moveTimer; // 累计到移动间隔后走一格。

    bool started; // 第一次按方向键后变 true。

    bool isSuperFood; // 当前食物是否是金色果子。

    int growLeft; // 剩余增长次数。
};

// 临时视觉效果的数据。
// 它们只影响显示，不决定游戏胜负。
struct EffectData
{
    float eatTimer;
    float scoreFlashTimer;
    Cell lastFoodCell;
};

// 疯狂模式状态。
struct CrazyModeData
{
    std::vector<CrazyBlock> blocks;

    float eventTimer; // 下一次随机事件倒计时。

    float boostTimer; // 加速倒计时。

    float redFlashTimer; // 加速前预警。

    float greenFlashTimer; // 加速后恢复提示。

    bool boostWaiting; // 红光结束后是否进入加速。
};

// 图片资源。
struct AssetData
{
    Texture2D menuBackground;
    bool menuBackgroundLoaded;

    bool menuBackgroundLoadFailed; // 背景图加载失败提示。
};

// Loading 过渡只需要一个倒计时。
struct LoadingData
{
    float timer;
};

// 全项目共享的总数据。
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
