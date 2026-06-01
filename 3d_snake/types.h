#pragma once

#include "raylib.h"
#include <vector>

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
    LevelSelect,
    Playing,
    Paused,
    GameOver,
    Win
};

struct SpeedSetting
{
    float startInterval;
    float minInterval;
    float speedUpAmount;
};

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

struct GameData
{
    std::vector<Cell> snake;
    Cell direction;
    Cell nextDirection;
    Cell food;
    GameStatus status;
    int score;
    float moveTimer;
    bool started;
    bool isSuperFood;

    // growLeft 表示接下来还有几次移动不删除尾巴。
    // 不删除尾巴，蛇就会变长。
    int growLeft;
};

struct EffectData
{
    float eatTimer;
    float scoreFlashTimer;
    Cell lastFoodCell;
};

struct CrazyModeData
{
    std::vector<CrazyBlock> blocks;
    float eventTimer;
    float boostTimer;
    float redFlashTimer;
    float greenFlashTimer;
    bool boostWaiting;
};

struct AssetData
{
    Texture2D menuBackground;
    bool menuBackgroundLoaded;
};

struct LoadingData
{
    float timer;
};

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
