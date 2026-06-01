#include "raylib.h"
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>

// ===== 常量参数 =====
const int SCREEN_WIDTH = 2560;
const int SCREEN_HEIGHT = 1440;

const int DEFAULT_GRID_SIZE = 15;
const int MIN_SPEED_LEVEL = 1;
const int MAX_SPEED_LEVEL = 5;
const int MIN_DIFFICULTY_LEVEL = 1;
const int MAX_DIFFICULTY_LEVEL = 5;
const int LEVEL_COUNT = 4;
const int SCORE_PER_NORMAL_FOOD = 10;
const int SCORE_PER_SUPER_FOOD = 50;
const int NORMAL_FOOD_GROW = 1;
const int SUPER_FOOD_GROW = 5;
const int SPEED_UP_SCORE = 50;
const int SUPER_FOOD_CHANCE = 20; // 20% 概率出现超级果子
const char *HIGH_SCORE_FILE = "highscore.txt";

const float CUBE_SIZE = 1.0f;
const float EAT_EFFECT_TIME = 0.35f;
const float SCORE_FLASH_TIME = 0.25f;
const float BOOST_TIME = 8.0f;
const float RED_FLASH_TIME = 1.0f;
const float GREEN_FLASH_TIME = 1.0f;
const float BOOST_SPEED_RATE = 1.6f;
const float BLUE_FLASH_TIME = 1.5f;
const int MAX_CRAZY_BLOCKS = 8;

const int MENU_WIDTH = 800;
const int MENU_HEIGHT = 750;
const int MENU_Y = 150;
const char *MENU_BACKGROUND_FILE = "assets/menu_background.png";
const float LOADING_HOLD_TIME = 0.4f;
const float LOADING_FADE_OUT_TIME = 0.15f;
const float LOADING_TIME = LOADING_HOLD_TIME + LOADING_FADE_OUT_TIME;

// ===== 数据结构 =====

// 地图格子坐标。x 是左右方向，z 是前后方向。
struct Cell
{
    int x;
    int z;
};

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
    std::vector<Cell> snake; // snake[0] 永远是蛇头
    Cell direction;
    Cell nextDirection;
    Cell food;
    GameStatus status;
    int score;
    float moveTimer;
    bool started;
    bool isSuperFood;
    int growLeft; // 还有几步不删除尾巴，用来表示蛇还要增长多少节
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

// ===== 全局数据 =====
GameSettings settings;
GameData game;
EffectData effects;
CrazyModeData crazy;
AssetData assets;
Camera3D camera;
int highScore = 0;
float loadingTimer = 0.0f;

// ===== 函数声明 =====
int GetMenuX();
int GetMapSizeByLevel(int level);
std::string GetLevelName();
void SetLevel(int level);
void SetSpeedByLevel(int speedLevel);
void UpdateTargetScore();
void StartSelectedGame();
void InitGame();

bool IsSameCell(Cell a, Cell b);
bool IsCellOnSnake(Cell cell);
bool IsCellOnCrazyBlock(Cell cell);
bool IsCellOnSolidCrazyBlock(Cell cell);
bool CheckWallCollision(Cell head);
bool CheckSelfCollision(Cell head, bool willGrow);
bool CanPassWall();
bool IsBoostEventBusy();

Cell GetNextHead();
void WrapHeadIfNeeded(Cell &head);
bool CheckGameOver(Cell head, bool willGrow);
void EatFoodIfNeeded(Cell head);
void UpdateSnakeLength();
void MoveSnake();
void SpawnFood();

void HandleInput();
void HandleLevelSelectInput();
void TryChangeDirection(Cell newDirection);
void StartLoading();
void UpdateLoading();
bool IsLoading();
void UpdateGame();
void UpdateEffects();
void UpdateCrazyMode();
void UpdateBoostEvent(float deltaTime);
void UpdateBlueBlocks(float deltaTime);
void TryStartCrazyEvent();
void TriggerRandomCrazyEvent();
void StartBoostEvent();
void StartBlueBlockEvent();

Vector3 CellToWorld(Cell cell);
float GetCurrentMoveInterval();
Color GetSnakeColor(Color normalColor);
void UpdateCameraPosition();
void DrawGame3D();
void DrawMenuBackground();
void DrawGridGround();
void DrawWalls();
void DrawSnake();
void DrawSnakeHead(Vector3 pos);
void DrawSnakeBody(Vector3 pos, int index, int totalLength);
void DrawFood();
void DrawEatEffect();
void DrawCrazyBlocks();
void DrawUI();
void DrawLevelSelectUI();
void DrawLoadingScreen();
void DrawCenterMessage(const char *title, const char *subtitle, Color titleColor);
void DrawButton(Rectangle button, const char *text, bool selected);
bool IsButtonClicked(Rectangle button);
Rectangle GetLevelButtonRect(int index);
Rectangle GetSpeedMinusButtonRect();
Rectangle GetSpeedPlusButtonRect();
Rectangle GetDifficultyMinusButtonRect();
Rectangle GetDifficultyPlusButtonRect();
Rectangle GetCrazyModeButtonRect();
Rectangle GetStartButtonRect();

int LoadHighScore();
void SaveHighScore();
void UpdateHighScore();
void LoadAssets();
void UnloadAssets();

// ===== 设置与初始化 =====

int GetMenuX()
{
    return (SCREEN_WIDTH - MENU_WIDTH) / 2;
}

int GetMapSizeByLevel(int level)
{
    if (level == 1)
    {
        return 10;
    }
    if (level == 2)
    {
        return 15;
    }
    if (level == 3)
    {
        return 20;
    }
    if (level == 4)
    {
        return 25;
    }
    return DEFAULT_GRID_SIZE;
}

std::string GetLevelName()
{
    return "Level " + std::to_string(settings.currentLevel);
}

void SetLevel(int level)
{
    if (level < 1 || level > LEVEL_COUNT)
    {
        level = 1;
    }

    settings.currentLevel = level;
    settings.gridSize = GetMapSizeByLevel(level);
    UpdateTargetScore();
}

void SetSpeedByLevel(int speedLevel)
{
    if (speedLevel < MIN_SPEED_LEVEL)
    {
        speedLevel = MIN_SPEED_LEVEL;
    }
    if (speedLevel > MAX_SPEED_LEVEL)
    {
        speedLevel = MAX_SPEED_LEVEL;
    }

    settings.speedLevel = speedLevel;

    if (speedLevel == 1)
    {
        settings.speed = {0.28f, 0.14f, 0.006f};
    }
    else if (speedLevel == 2)
    {
        settings.speed = {0.22f, 0.11f, 0.008f};
    }
    else if (speedLevel == 3)
    {
        settings.speed = {0.18f, 0.08f, 0.010f};
    }
    else if (speedLevel == 4)
    {
        settings.speed = {0.14f, 0.07f, 0.012f};
    }
    else
    {
        settings.speed = {0.10f, 0.05f, 0.014f};
    }
}

void UpdateTargetScore()
{
    settings.targetScore = settings.gridSize * settings.difficultyLevel * 2;
}

void StartSelectedGame()
{
    SetLevel(settings.currentLevel);
    SetSpeedByLevel(settings.speedLevel);
    UpdateTargetScore();
    InitGame();
}

void InitGame()
{
    game.snake.clear();

    Cell start = {settings.gridSize / 2, settings.gridSize / 2};
    game.snake.push_back(start);
    game.snake.push_back({start.x - 1, start.z});
    game.snake.push_back({start.x - 2, start.z});

    game.direction = {1, 0};
    game.nextDirection = game.direction;
    game.food = {-1, -1};
    game.status = Playing;
    game.score = 0;
    game.moveTimer = 0.0f;
    game.started = false;
    game.isSuperFood = false;
    game.growLeft = 0;

    effects.eatTimer = 0.0f;
    effects.scoreFlashTimer = 0.0f;
    effects.lastFoodCell = {-1, -1};

    crazy.blocks.clear();
    crazy.eventTimer = 3.0f;
    crazy.boostTimer = 0.0f;
    crazy.redFlashTimer = 0.0f;
    crazy.greenFlashTimer = 0.0f;
    crazy.boostWaiting = false;

    SpawnFood();
}

// ===== 基础判断 =====

bool IsSameCell(Cell a, Cell b)
{
    return a.x == b.x && a.z == b.z;
}

bool IsCellOnSnake(Cell cell)
{
    for (int i = 0; i < (int)game.snake.size(); i++)
    {
        if (IsSameCell(game.snake[i], cell))
        {
            return true;
        }
    }
    return false;
}

bool IsCellOnCrazyBlock(Cell cell)
{
    for (int i = 0; i < (int)crazy.blocks.size(); i++)
    {
        if (IsSameCell(crazy.blocks[i].cell, cell))
        {
            return true;
        }
    }
    return false;
}

bool IsCellOnSolidCrazyBlock(Cell cell)
{
    for (int i = 0; i < (int)crazy.blocks.size(); i++)
    {
        if (crazy.blocks[i].solid && IsSameCell(crazy.blocks[i].cell, cell))
        {
            return true;
        }
    }
    return false;
}

bool CheckWallCollision(Cell head)
{
    return head.x < 0 ||
           head.x >= settings.gridSize ||
           head.z < 0 ||
           head.z >= settings.gridSize;
}

bool CheckSelfCollision(Cell head, bool willGrow)
{
    int checkEnd = (int)game.snake.size();

    // 不增长时，尾巴本回合会移开，所以可以不检查最后一节。
    if (!willGrow)
    {
        checkEnd--;
    }

    for (int i = 1; i < checkEnd; i++)
    {
        if (IsSameCell(game.snake[i], head))
        {
            return true;
        }
    }
    return false;
}

bool CanPassWall()
{
    return settings.crazyMode && crazy.boostTimer > 0.0f;
}

bool IsBoostEventBusy()
{
    return crazy.redFlashTimer > 0.0f ||
           crazy.boostTimer > 0.0f ||
           crazy.greenFlashTimer > 0.0f ||
           crazy.boostWaiting;
}

// ===== 蛇移动逻辑 =====

Cell GetNextHead()
{
    Cell head = game.snake[0];
    head.x += game.nextDirection.x;
    head.z += game.nextDirection.z;
    return head;
}

void WrapHeadIfNeeded(Cell &head)
{
    if (!CanPassWall())
    {
        return;
    }

    if (head.x < 0)
    {
        head.x = settings.gridSize - 1;
    }
    if (head.x >= settings.gridSize)
    {
        head.x = 0;
    }
    if (head.z < 0)
    {
        head.z = settings.gridSize - 1;
    }
    if (head.z >= settings.gridSize)
    {
        head.z = 0;
    }
}

bool CheckGameOver(Cell head, bool willGrow)
{
    if (!CanPassWall() && CheckWallCollision(head))
    {
        return true;
    }
    if (CheckSelfCollision(head, willGrow))
    {
        return true;
    }
    if (IsCellOnSolidCrazyBlock(head))
    {
        return true;
    }
    return false;
}

void EatFoodIfNeeded(Cell head)
{
    if (!IsSameCell(head, game.food))
    {
        return;
    }

    effects.lastFoodCell = game.food;
    effects.eatTimer = EAT_EFFECT_TIME;
    effects.scoreFlashTimer = SCORE_FLASH_TIME;

    if (game.isSuperFood)
    {
        game.score += SCORE_PER_SUPER_FOOD;
        game.growLeft += SUPER_FOOD_GROW;
    }
    else
    {
        game.score += SCORE_PER_NORMAL_FOOD;
        game.growLeft += NORMAL_FOOD_GROW;
    }

    UpdateHighScore();

    if (!settings.crazyMode && game.score >= settings.targetScore)
    {
        game.status = Win;
        game.food = {-1, -1};
        return;
    }

    SpawnFood();
}

void UpdateSnakeLength()
{
    if (game.growLeft > 0)
    {
        game.growLeft--;
    }
    else
    {
        game.snake.pop_back();
    }
}

void MoveSnake()
{
    game.direction = game.nextDirection;

    Cell newHead = GetNextHead();
    WrapHeadIfNeeded(newHead);

    bool willGrow = IsSameCell(newHead, game.food);
    if (CheckGameOver(newHead, willGrow))
    {
        game.status = GameOver;
        return;
    }

    game.snake.insert(game.snake.begin(), newHead);
    EatFoodIfNeeded(newHead);
    UpdateSnakeLength();
}

void SpawnFood()
{
    std::vector<Cell> emptyCells;

    for (int x = 0; x < settings.gridSize; x++)
    {
        for (int z = 0; z < settings.gridSize; z++)
        {
            Cell cell = {x, z};
            if (!IsCellOnSnake(cell) && !IsCellOnCrazyBlock(cell))
            {
                emptyCells.push_back(cell);
            }
        }
    }

    if (emptyCells.empty())
    {
        game.food = {-1, -1};
        game.status = Win;
        UpdateHighScore();
        return;
    }

    int index = rand() % emptyCells.size();
    game.food = emptyCells[index];
    game.isSuperFood = rand() % 100 < SUPER_FOOD_CHANCE;
}

// ===== 输入与更新 =====

void TryChangeDirection(Cell newDirection)
{
    bool isReverse = newDirection.x + game.direction.x == 0 &&
                     newDirection.z + game.direction.z == 0;

    if (isReverse)
    {
        return;
    }

    game.nextDirection = newDirection;
    game.started = true;
}

void HandleInput()
{
    if (IsLoading())
    {
        return;
    }

    if (game.status == LevelSelect)
    {
        HandleLevelSelectInput();
        return;
    }

    if (IsKeyPressed(KEY_M))
    {
        game.status = LevelSelect;
        game.started = false;
        game.snake.clear();
        game.food = {-1, -1};
        StartLoading();
        return;
    }

    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))
    {
        TryChangeDirection({0, -1});
    }
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN))
    {
        TryChangeDirection({0, 1});
    }
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT))
    {
        TryChangeDirection({-1, 0});
    }
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT))
    {
        TryChangeDirection({1, 0});
    }

    if (IsKeyPressed(KEY_SPACE))
    {
        if (game.status == Playing)
        {
            game.status = Paused;
        }
        else if (game.status == Paused)
        {
            game.status = Playing;
        }
    }

    if (IsKeyPressed(KEY_R))
    {
        InitGame();
    }
}

void HandleLevelSelectInput()
{
    for (int i = 0; i < LEVEL_COUNT; i++)
    {
        if (IsButtonClicked(GetLevelButtonRect(i)))
        {
            SetLevel(i + 1);
        }
    }

    if (IsButtonClicked(GetSpeedMinusButtonRect()))
    {
        SetSpeedByLevel(settings.speedLevel - 1);
    }
    if (IsButtonClicked(GetSpeedPlusButtonRect()))
    {
        SetSpeedByLevel(settings.speedLevel + 1);
    }

    if (IsButtonClicked(GetDifficultyMinusButtonRect()))
    {
        settings.difficultyLevel--;
        if (settings.difficultyLevel < MIN_DIFFICULTY_LEVEL)
        {
            settings.difficultyLevel = MIN_DIFFICULTY_LEVEL;
        }
        UpdateTargetScore();
    }
    if (IsButtonClicked(GetDifficultyPlusButtonRect()))
    {
        settings.difficultyLevel++;
        if (settings.difficultyLevel > MAX_DIFFICULTY_LEVEL)
        {
            settings.difficultyLevel = MAX_DIFFICULTY_LEVEL;
        }
        UpdateTargetScore();
    }

    if (IsButtonClicked(GetCrazyModeButtonRect()))
    {
        settings.crazyMode = !settings.crazyMode;
    }

    if (IsButtonClicked(GetStartButtonRect()))
    {
        StartSelectedGame();
        StartLoading();
    }
}

void StartLoading()
{
    loadingTimer = LOADING_TIME;
}

void UpdateLoading()
{
    if (loadingTimer > 0.0f)
    {
        loadingTimer -= GetFrameTime();
        if (loadingTimer < 0.0f)
        {
            loadingTimer = 0.0f;
        }
    }
}

bool IsLoading()
{
    return loadingTimer > 0.0f;
}

void UpdateGame()
{
    if (game.status != Playing || !game.started)
    {
        return;
    }

    game.moveTimer += GetFrameTime();
    if (game.moveTimer >= GetCurrentMoveInterval())
    {
        game.moveTimer = 0.0f;
        MoveSnake();
    }
}

void UpdateEffects()
{
    float deltaTime = GetFrameTime();

    if (effects.eatTimer > 0.0f)
    {
        effects.eatTimer -= deltaTime;
        if (effects.eatTimer < 0.0f)
        {
            effects.eatTimer = 0.0f;
        }
    }

    if (effects.scoreFlashTimer > 0.0f)
    {
        effects.scoreFlashTimer -= deltaTime;
        if (effects.scoreFlashTimer < 0.0f)
        {
            effects.scoreFlashTimer = 0.0f;
        }
    }
}

void UpdateCrazyMode()
{
    if (!settings.crazyMode || game.status != Playing)
    {
        return;
    }

    float deltaTime = GetFrameTime();
    UpdateBoostEvent(deltaTime);
    UpdateBlueBlocks(deltaTime);
    TryStartCrazyEvent();
}

void UpdateBoostEvent(float deltaTime)
{
    if (crazy.boostTimer > 0.0f)
    {
        crazy.boostTimer -= deltaTime;
        if (crazy.boostTimer <= 0.0f)
        {
            crazy.boostTimer = 0.0f;
            crazy.greenFlashTimer = GREEN_FLASH_TIME;
        }
    }

    if (crazy.redFlashTimer > 0.0f)
    {
        crazy.redFlashTimer -= deltaTime;
        if (crazy.redFlashTimer <= 0.0f)
        {
            crazy.redFlashTimer = 0.0f;
            if (crazy.boostWaiting)
            {
                crazy.boostTimer = BOOST_TIME;
                crazy.boostWaiting = false;
            }
        }
    }

    if (crazy.greenFlashTimer > 0.0f)
    {
        crazy.greenFlashTimer -= deltaTime;
        if (crazy.greenFlashTimer < 0.0f)
        {
            crazy.greenFlashTimer = 0.0f;
        }
    }
}

void UpdateBlueBlocks(float deltaTime)
{
    for (int i = 0; i < (int)crazy.blocks.size(); i++)
    {
        if (!crazy.blocks[i].solid)
        {
            crazy.blocks[i].flashTimer -= deltaTime;
            if (crazy.blocks[i].flashTimer <= 0.0f)
            {
                crazy.blocks[i].flashTimer = 0.0f;
                crazy.blocks[i].solid = true;
            }
        }
    }
}

void TryStartCrazyEvent()
{
    crazy.eventTimer -= GetFrameTime();
    if (crazy.eventTimer > 0.0f)
    {
        return;
    }

    if (IsBoostEventBusy())
    {
        StartBlueBlockEvent();
    }
    else
    {
        TriggerRandomCrazyEvent();
    }

    crazy.eventTimer = 2.0f + (float)(rand() % 3);
}

void TriggerRandomCrazyEvent()
{
    if (rand() % 2 == 0)
    {
        StartBoostEvent();
    }
    else
    {
        StartBlueBlockEvent();
    }
}

void StartBoostEvent()
{
    if (IsBoostEventBusy())
    {
        return;
    }

    crazy.redFlashTimer = RED_FLASH_TIME;
    crazy.boostWaiting = true;
}

void StartBlueBlockEvent()
{
    if ((int)crazy.blocks.size() >= MAX_CRAZY_BLOCKS)
    {
        return;
    }

    std::vector<Cell> emptyCells;
    for (int x = 0; x < settings.gridSize; x++)
    {
        for (int z = 0; z < settings.gridSize; z++)
        {
            Cell cell = {x, z};
            if (!IsCellOnSnake(cell) &&
                !IsSameCell(cell, game.food) &&
                !IsCellOnCrazyBlock(cell))
            {
                emptyCells.push_back(cell);
            }
        }
    }

    if (emptyCells.empty())
    {
        return;
    }

    int index = rand() % emptyCells.size();
    CrazyBlock block;
    block.cell = emptyCells[index];
    block.solid = false;
    block.flashTimer = BLUE_FLASH_TIME;
    crazy.blocks.push_back(block);
}

// ===== 绘制辅助 =====

Vector3 CellToWorld(Cell cell)
{
    float worldX = (cell.x - settings.gridSize / 2.0f + 0.5f) * CUBE_SIZE;
    float worldZ = (cell.z - settings.gridSize / 2.0f + 0.5f) * CUBE_SIZE;
    return (Vector3){worldX, 0.5f, worldZ};
}

float GetCurrentMoveInterval()
{
    int scoreSpeedLevel = game.score / SPEED_UP_SCORE;
    float interval = settings.speed.startInterval - scoreSpeedLevel * settings.speed.speedUpAmount;

    if (interval < settings.speed.minInterval)
    {
        interval = settings.speed.minInterval;
    }

    if (crazy.boostTimer > 0.0f)
    {
        interval = interval / BOOST_SPEED_RATE;
    }

    return interval;
}

Color GetSnakeColor(Color normalColor)
{
    bool shouldFlash = (int)(GetTime() * 10) % 2 == 0;

    if (crazy.boostTimer > 0.0f)
    {
        return RED;
    }
    if (crazy.redFlashTimer > 0.0f && shouldFlash)
    {
        return RED;
    }
    if (crazy.greenFlashTimer > 0.0f && shouldFlash)
    {
        return LIME;
    }
    return normalColor;
}

void UpdateCameraPosition()
{
    camera.position = (Vector3){0.0f, 22.0f, 24.0f};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}

void DrawMenuBackground()
{
    if (!assets.menuBackgroundLoaded)
    {
        ClearBackground((Color){13, 18, 30, 255});
        return;
    }

    Rectangle source = {
        0.0f,
        0.0f,
        (float)assets.menuBackground.width,
        (float)assets.menuBackground.height};
    Rectangle target = {0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT};

    DrawTexturePro(assets.menuBackground, source, target, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
}

void DrawGame3D()
{
    DrawGridGround();
    DrawWalls();
    DrawCrazyBlocks();
    DrawFood();
    DrawEatEffect();
    DrawSnake();
}

void DrawGridGround()
{
    for (int x = 0; x < settings.gridSize; x++)
    {
        for (int z = 0; z < settings.gridSize; z++)
        {
            Vector3 pos = CellToWorld({x, z});
            pos.y = -0.06f;

            Color color = (x + z) % 2 == 0
                              ? (Color){46, 54, 64, 255}
                              : (Color){38, 45, 55, 255};

            DrawCube(pos, CUBE_SIZE, 0.08f, CUBE_SIZE, color);
            DrawCubeWires((Vector3){pos.x, -0.015f, pos.z}, CUBE_SIZE, 0.01f, CUBE_SIZE, (Color){70, 80, 92, 120});
        }
    }
}

void DrawWalls()
{
    float half = settings.gridSize * CUBE_SIZE / 2.0f;
    float edge = half + 0.5f;
    float y = 0.5f;

    Color wallColor = (Color){110, 118, 128, 230};
    if (CanPassWall())
    {
        wallColor = (Color){220, 90, 90, 160};
    }

    for (int i = 0; i < settings.gridSize; i++)
    {
        float offset = (i - settings.gridSize / 2.0f + 0.5f) * CUBE_SIZE;
        DrawCube((Vector3){offset, y, -edge}, CUBE_SIZE, CUBE_SIZE * 1.2f, CUBE_SIZE, wallColor);
        DrawCube((Vector3){offset, y, edge}, CUBE_SIZE, CUBE_SIZE * 1.2f, CUBE_SIZE, wallColor);
        DrawCube((Vector3){-edge, y, offset}, CUBE_SIZE, CUBE_SIZE * 1.2f, CUBE_SIZE, wallColor);
        DrawCube((Vector3){edge, y, offset}, CUBE_SIZE, CUBE_SIZE * 1.2f, CUBE_SIZE, wallColor);
    }

    Color cornerColor = (Color){245, 190, 80, 255};
    DrawCube((Vector3){-edge, 1.05f, -edge}, CUBE_SIZE * 1.2f, CUBE_SIZE * 1.6f, CUBE_SIZE * 1.2f, cornerColor);
    DrawCube((Vector3){edge, 1.05f, -edge}, CUBE_SIZE * 1.2f, CUBE_SIZE * 1.6f, CUBE_SIZE * 1.2f, cornerColor);
    DrawCube((Vector3){-edge, 1.05f, edge}, CUBE_SIZE * 1.2f, CUBE_SIZE * 1.6f, CUBE_SIZE * 1.2f, cornerColor);
    DrawCube((Vector3){edge, 1.05f, edge}, CUBE_SIZE * 1.2f, CUBE_SIZE * 1.6f, CUBE_SIZE * 1.2f, cornerColor);
}

void DrawSnake()
{
    for (int i = 0; i < (int)game.snake.size(); i++)
    {
        Vector3 pos = CellToWorld(game.snake[i]);
        if (i == 0)
        {
            DrawSnakeHead(pos);
        }
        else
        {
            DrawSnakeBody(pos, i, (int)game.snake.size());
        }
    }
}

void DrawSnakeHead(Vector3 pos)
{
    Color headColor = GetSnakeColor((Color){28, 150, 75, 255});
    DrawCube(pos, CUBE_SIZE * 0.92f, CUBE_SIZE * 0.92f, CUBE_SIZE * 0.92f, headColor);
    DrawCubeWires(pos, CUBE_SIZE * 0.92f, CUBE_SIZE * 0.92f, CUBE_SIZE * 0.92f, BLACK);

    float eyeOffsetSide = 0.18f;
    float eyeOffsetFront = 0.46f;
    float eyeY = pos.y + 0.18f;
    Vector3 leftEye = pos;
    Vector3 rightEye = pos;

    if (game.direction.x != 0)
    {
        leftEye.x += game.direction.x * eyeOffsetFront;
        rightEye.x += game.direction.x * eyeOffsetFront;
        leftEye.z -= eyeOffsetSide;
        rightEye.z += eyeOffsetSide;
    }
    else
    {
        leftEye.z += game.direction.z * eyeOffsetFront;
        rightEye.z += game.direction.z * eyeOffsetFront;
        leftEye.x -= eyeOffsetSide;
        rightEye.x += eyeOffsetSide;
    }

    leftEye.y = eyeY;
    rightEye.y = eyeY;
    DrawSphere(leftEye, 0.08f, WHITE);
    DrawSphere(rightEye, 0.08f, WHITE);
    DrawSphere(leftEye, 0.035f, BLACK);
    DrawSphere(rightEye, 0.035f, BLACK);
}

void DrawSnakeBody(Vector3 pos, int index, int totalLength)
{
    float size = CUBE_SIZE * 0.84f;
    if (totalLength > 1)
    {
        float tailProgress = (float)index / (float)(totalLength - 1);
        size = CUBE_SIZE * (0.86f - tailProgress * 0.14f);
    }

    int green = 180 - index * 3;
    if (green < 95)
    {
        green = 95;
    }

    Color normalColor = (Color){55, (unsigned char)green, 85, 255};
    Color bodyColor = GetSnakeColor(normalColor);
    DrawCube(pos, size, size, size, bodyColor);
    DrawCubeWires(pos, size, size, size, (Color){10, 40, 20, 255});
}

void DrawFood()
{
    if (game.food.x < 0 || game.food.z < 0)
    {
        return;
    }

    Vector3 pos = CellToWorld(game.food);

    if (game.isSuperFood)
    {
        DrawSphere(pos, CUBE_SIZE * 0.52f, GOLD);
        DrawSphereWires(pos, CUBE_SIZE * 0.64f, 10, 10, ORANGE);
    }
    else
    {
        DrawSphere(pos, CUBE_SIZE * 0.40f, RED);
    }
}

void DrawEatEffect()
{
    if (effects.eatTimer <= 0.0f || effects.lastFoodCell.x < 0 || effects.lastFoodCell.z < 0)
    {
        return;
    }

    float progress = 1.0f - effects.eatTimer / EAT_EFFECT_TIME;
    Vector3 pos = CellToWorld(effects.lastFoodCell);
    float radius = CUBE_SIZE * (0.4f + progress * 1.2f);
    unsigned char alpha = (unsigned char)(180 * (1.0f - progress));

    DrawSphereWires(pos, radius, 12, 12, (Color){255, 210, 80, alpha});
}

void DrawCrazyBlocks()
{
    for (int i = 0; i < (int)crazy.blocks.size(); i++)
    {
        Vector3 pos = CellToWorld(crazy.blocks[i].cell);

        if (!crazy.blocks[i].solid)
        {
            bool showWarning = (int)(GetTime() * 8) % 2 == 0;
            if (showWarning)
            {
                DrawCube(pos, CUBE_SIZE * 0.9f, CUBE_SIZE * 0.9f, CUBE_SIZE * 0.9f, SKYBLUE);
                DrawCubeWires(pos, CUBE_SIZE, CUBE_SIZE, CUBE_SIZE, BLUE);
            }
        }
        else
        {
            DrawCube(pos, CUBE_SIZE * 0.95f, CUBE_SIZE * 0.95f, CUBE_SIZE * 0.95f, BLUE);
            DrawCubeWires(pos, CUBE_SIZE * 0.95f, CUBE_SIZE * 0.95f, CUBE_SIZE * 0.95f, DARKBLUE);
        }
    }
}

// ===== 菜单和 UI =====

Rectangle GetLevelButtonRect(int index)
{
    int buttonWidth = 155;
    int buttonHeight = 65;
    int buttonGap = 25;
    int totalWidth = buttonWidth * LEVEL_COUNT + buttonGap * (LEVEL_COUNT - 1);
    int startX = GetMenuX() + (MENU_WIDTH - totalWidth) / 2;
    int y = MENU_Y + 190;

    return {
        (float)(startX + index * (buttonWidth + buttonGap)),
        (float)y,
        (float)buttonWidth,
        (float)buttonHeight};
}

Rectangle GetSpeedMinusButtonRect()
{
    return {(float)(GetMenuX() + 110), (float)(MENU_Y + 400), 65.0f, 50.0f};
}

Rectangle GetSpeedPlusButtonRect()
{
    return {(float)(GetMenuX() + MENU_WIDTH - 175), (float)(MENU_Y + 400), 65.0f, 50.0f};
}

Rectangle GetDifficultyMinusButtonRect()
{
    return {(float)(GetMenuX() + 110), (float)(MENU_Y + 550), 65.0f, 50.0f};
}

Rectangle GetDifficultyPlusButtonRect()
{
    return {(float)(GetMenuX() + MENU_WIDTH - 175), (float)(MENU_Y + 550), 65.0f, 50.0f};
}

Rectangle GetCrazyModeButtonRect()
{
    int buttonWidth = 260;
    int buttonHeight = 55;
    return {
        (float)(GetMenuX() + (MENU_WIDTH - buttonWidth) / 2),
        (float)(MENU_Y + 615),
        (float)buttonWidth,
        (float)buttonHeight};
}

Rectangle GetStartButtonRect()
{
    int buttonWidth = 240;
    int buttonHeight = 70;
    return {
        (float)(GetMenuX() + (MENU_WIDTH - buttonWidth) / 2),
        (float)(MENU_Y + 680),
        (float)buttonWidth,
        (float)buttonHeight};
}

bool IsButtonClicked(Rectangle button)
{
    return CheckCollisionPointRec(GetMousePosition(), button) &&
           IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void DrawButton(Rectangle button, const char *text, bool selected)
{
    bool mouseOver = CheckCollisionPointRec(GetMousePosition(), button);

    Color fillColor = (Color){45, 55, 70, 230};
    Color lineColor = (Color){170, 180, 195, 220};
    Color textColor = LIGHTGRAY;

    if (selected)
    {
        fillColor = (Color){60, 120, 80, 240};
        lineColor = (Color){230, 230, 160, 255};
        textColor = WHITE;
    }
    else if (mouseOver)
    {
        fillColor = (Color){65, 75, 92, 240};
        textColor = WHITE;
    }

    DrawRectangleRec(button, fillColor);
    DrawRectangleLines((int)button.x, (int)button.y, (int)button.width, (int)button.height, lineColor);

    int fontSize = 22;
    int textWidth = MeasureText(text, fontSize);
    int textX = (int)(button.x + (button.width - textWidth) / 2);
    int textY = (int)(button.y + (button.height - fontSize) / 2);
    DrawText(text, textX, textY, fontSize, textColor);
}

void DrawLevelSelectUI()
{
    DrawMenuBackground();

    int menuX = GetMenuX();
    DrawRectangle(menuX, MENU_Y, MENU_WIDTH, MENU_HEIGHT, (Color){10, 15, 30, 200});
    DrawRectangleLines(menuX, MENU_Y, MENU_WIDTH, MENU_HEIGHT, (Color){180, 180, 200, 160});

    const char *title = "SNAKE MENU";
    int titleSize = 72;
    int titleWidth = MeasureText(title, titleSize);
    DrawText(title, (SCREEN_WIDTH - titleWidth) / 2, MENU_Y + 50, titleSize, YELLOW);

    const char *mapLabel = "Choose Map";
    int mapLabelSize = 32;
    int mapLabelWidth = MeasureText(mapLabel, mapLabelSize);
    DrawText(mapLabel, (SCREEN_WIDTH - mapLabelWidth) / 2, MENU_Y + 140, mapLabelSize, WHITE);

    for (int i = 0; i < LEVEL_COUNT; i++)
    {
        Rectangle button = GetLevelButtonRect(i);
        std::string buttonText = "Level " + std::to_string(i + 1);
        DrawButton(button, buttonText.c_str(), settings.currentLevel == i + 1);

        int mapSize = GetMapSizeByLevel(i + 1);
        std::string mapText = std::to_string(mapSize) + " x " + std::to_string(mapSize);
        int mapTextWidth = MeasureText(mapText.c_str(), 22);
        DrawText(mapText.c_str(),
                 (int)(button.x + (button.width - mapTextWidth) / 2),
                 (int)(button.y + button.height + 8),
                 22, LIGHTGRAY);
    }

    const char *speedLabel = "Speed";
    int speedLabelSize = 32;
    int speedLabelWidth = MeasureText(speedLabel, speedLabelSize);
    DrawText(speedLabel, (SCREEN_WIDTH - speedLabelWidth) / 2, MENU_Y + 340, speedLabelSize, WHITE);

    DrawButton(GetSpeedMinusButtonRect(), "-", false);
    DrawButton(GetSpeedPlusButtonRect(), "+", false);
    std::string speedText = "Level " + std::to_string(settings.speedLevel);
    int speedTextWidth = MeasureText(speedText.c_str(), 34);
    DrawText(speedText.c_str(), (SCREEN_WIDTH - speedTextWidth) / 2, MENU_Y + 408, 34, WHITE);

    const char *diffLabel = "Difficulty Target";
    int diffLabelSize = 32;
    int diffLabelWidth = MeasureText(diffLabel, diffLabelSize);
    DrawText(diffLabel, (SCREEN_WIDTH - diffLabelWidth) / 2, MENU_Y + 490, diffLabelSize, WHITE);

    DrawButton(GetDifficultyMinusButtonRect(), "-", false);
    DrawButton(GetDifficultyPlusButtonRect(), "+", false);
    std::string difficultyText = "Level " + std::to_string(settings.difficultyLevel) +
                                 "  Target " + std::to_string(settings.targetScore);
    int diffTextWidth = MeasureText(difficultyText.c_str(), 32);
    DrawText(difficultyText.c_str(), (SCREEN_WIDTH - diffTextWidth) / 2, MENU_Y + 558, 32, WHITE);

    std::string crazyText = settings.crazyMode ? "Crazy Mode: ON" : "Crazy Mode: OFF";
    DrawButton(GetCrazyModeButtonRect(), crazyText.c_str(), settings.crazyMode);
    DrawButton(GetStartButtonRect(), "START", false);
}

void DrawUI()
{
    if (game.status == LevelSelect)
    {
        DrawLevelSelectUI();
        return;
    }

    std::string scoreText = "Score: " + std::to_string(game.score);
    Color scoreColor = effects.scoreFlashTimer > 0.0f ? YELLOW : WHITE;
    int scoreSize = 42;
    int scoreWidth = MeasureText(scoreText.c_str(), scoreSize);
    DrawText(scoreText.c_str(), (SCREEN_WIDTH - scoreWidth) / 2, 25, scoreSize, scoreColor);

    std::string highScoreText = "High Score: " + std::to_string(highScore);
    std::string targetText = settings.crazyMode ? "Target: Infinite" : "Target: " + std::to_string(settings.targetScore);
    std::string levelText = "Level: " + GetLevelName() +
                            " | Map: " + std::to_string(settings.gridSize) + " x " + std::to_string(settings.gridSize);

    std::string modeText = settings.crazyMode ? "Mode: Crazy" : "Mode: Normal";
    if (crazy.boostTimer > 0.0f)
    {
        modeText += " | Boost";
    }
    else if (crazy.redFlashTimer > 0.0f)
    {
        modeText += " | Warning";
    }
    else if (crazy.greenFlashTimer > 0.0f)
    {
        modeText += " | Recover";
    }

    std::string infoText = highScoreText + "   |   " + targetText + "   |   " + levelText + "   |   " + modeText;
    int infoSize = 24;
    int infoWidth = MeasureText(infoText.c_str(), infoSize);
    DrawText(infoText.c_str(), (SCREEN_WIDTH - infoWidth) / 2, 78, infoSize, LIGHTGRAY);

    std::string speedText = "Speed Level: " + std::to_string(game.score / SPEED_UP_SCORE + 1);
    int speedWidth = MeasureText(speedText.c_str(), infoSize);
    DrawText(speedText.c_str(), (SCREEN_WIDTH - speedWidth) / 2, 110, infoSize, LIGHTGRAY);

    DrawText("WASD / Arrow Keys: Move", 20, SCREEN_HEIGHT - 120, 20, LIGHTGRAY);
    DrawText("M: Back to Menu", 20, SCREEN_HEIGHT - 85, 20, LIGHTGRAY);
    DrawText("Space: Pause | R: Restart | Esc: Quit", 20, SCREEN_HEIGHT - 45, 20, LIGHTGRAY);

    if (game.status == Paused)
    {
        DrawCenterMessage("PAUSED", "Press Space to continue", YELLOW);
    }
    if (game.status == GameOver)
    {
        std::string resultText = "Final Score: " + std::to_string(game.score) + " | R restart | M menu";
        DrawCenterMessage("GAME OVER", resultText.c_str(), RED);
    }
    if (game.status == Win)
    {
        std::string resultText = GetLevelName() + " Clear | Score: " + std::to_string(game.score) + " | M menu";
        DrawCenterMessage("YOU WIN", resultText.c_str(), GREEN);
    }
    if (!game.started && game.status == Playing)
    {
        const char *msg = "Press WASD or Arrow Keys to start";
        int fontSize = 25;
        int textWidth = MeasureText(msg, fontSize);
        DrawText(msg, (SCREEN_WIDTH - textWidth) / 2, SCREEN_HEIGHT / 2 + 200, fontSize, (Color){200, 200, 200, 180});
    }
}

void DrawLoadingScreen()
{
    if (!IsLoading())
    {
        return;
    }

    float timePassed = LOADING_TIME - loadingTimer;
    float alphaRate;

    if (timePassed < LOADING_HOLD_TIME)
    {
        alphaRate = 1.0f;
    }
    else
    {
        alphaRate = loadingTimer / LOADING_FADE_OUT_TIME;
    }

    unsigned char alpha = (unsigned char)(255 * alphaRate);
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, alpha});

    const char *text = "LOADING...";
    int fontSize = 46;
    int textWidth = MeasureText(text, fontSize);
    DrawText(text,
             (SCREEN_WIDTH - textWidth) / 2,
             SCREEN_HEIGHT / 2 - fontSize / 2,
             fontSize,
             (Color){255, 255, 255, alpha});
}

void DrawCenterMessage(const char *title, const char *subtitle, Color titleColor)
{
    int boxWidth = 520;
    int boxHeight = 170;
    int boxX = (SCREEN_WIDTH - boxWidth) / 2;
    int boxY = SCREEN_HEIGHT / 2 - 125;

    DrawRectangle(boxX, boxY, boxWidth, boxHeight, (Color){0, 0, 0, 150});
    DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, (Color){220, 220, 220, 140});

    int titleSize = 58;
    int subtitleSize = 26;
    int titleWidth = MeasureText(title, titleSize);
    int subtitleWidth = MeasureText(subtitle, subtitleSize);

    DrawText(title, (SCREEN_WIDTH - titleWidth) / 2, boxY + 30, titleSize, titleColor);
    DrawText(subtitle, (SCREEN_WIDTH - subtitleWidth) / 2, boxY + 105, subtitleSize, LIGHTGRAY);
}

// ===== 最高分 =====

int LoadHighScore()
{
    int savedScore = 0;
    std::ifstream inputFile(HIGH_SCORE_FILE);

    if (inputFile.is_open())
    {
        inputFile >> savedScore;
        inputFile.close();
    }

    return savedScore;
}

void SaveHighScore()
{
    std::ofstream outputFile(HIGH_SCORE_FILE);

    if (outputFile.is_open())
    {
        outputFile << highScore;
        outputFile.close();
    }
}

void UpdateHighScore()
{
    if (game.score > highScore)
    {
        highScore = game.score;
        SaveHighScore();
    }
}

void LoadAssets()
{
    assets.menuBackground = LoadTexture(MENU_BACKGROUND_FILE);
    assets.menuBackgroundLoaded = assets.menuBackground.id != 0;
}

void UnloadAssets()
{
    if (assets.menuBackgroundLoaded)
    {
        UnloadTexture(assets.menuBackground);
        assets.menuBackgroundLoaded = false;
    }
}

// ===== 主函数 =====

int main()
{
    srand((unsigned int)time(nullptr));

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "3D Snake Game");
    SetTargetFPS(60);

    UpdateCameraPosition();
    LoadAssets();
    highScore = LoadHighScore();

    settings.currentLevel = 1;
    settings.difficultyLevel = 3;
    settings.crazyMode = false;
    SetLevel(1);
    SetSpeedByLevel(3);
    game.status = LevelSelect;

    while (!WindowShouldClose())
    {
        HandleInput();
        UpdateLoading();
        UpdateGame();
        UpdateEffects();
        UpdateCrazyMode();

        BeginDrawing();
        ClearBackground((Color){20, 25, 40, 255});

        if (game.status != LevelSelect)
        {
            DrawMenuBackground();
            BeginMode3D(camera);
            DrawGame3D();
            EndMode3D();
        }

        DrawUI();
        DrawLoadingScreen();
        EndDrawing();
    }

    UnloadAssets();
    CloseWindow();
    return 0;
}
