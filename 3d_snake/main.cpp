#include "raylib.h"
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <fstream>

// ===== 常量参数 =====
const int SCREEN_WIDTH = 2560;
const int SCREEN_HEIGHT = 1440;

const int DEFAULT_GRID_SIZE = 15;
const int MIN_CUSTOM_SPEED_LEVEL = 1;
const int MAX_CUSTOM_SPEED_LEVEL = 5;
const int MIN_DIFFICULTY_LEVEL = 1;
const int MAX_DIFFICULTY_LEVEL = 5;
const float CUBE_SIZE = 1.0f;
const int SPEED_UP_SCORE = 50;            // 每获得多少分提高一次速度
const char* HIGH_SCORE_FILE = "highscore.txt";

const float FOOD_FLOAT_HEIGHT = 0.15f;
const float FOOD_FLOAT_SPEED = 4.0f;
const float EAT_EFFECT_TIME = 0.35f;
const float SCORE_FLASH_TIME = 0.25f;

// 主菜单布局常量
const int MENU_X = 880;
const int MENU_Y = 150;
const int MENU_WIDTH = 800;
const int MENU_HEIGHT = 750;

// ===== 数据结构 =====

// 地图格子坐标
struct Cell {
    int x;
    int z;
};

// 游戏状态
enum GameStatus {
    LevelSelect,
    Playing,
    Paused,
    GameOver,
    Win
};

// ===== 游戏全局数据 =====
std::vector<Cell> snake;      // 蛇身体，snake[0] 是蛇头
Cell direction;               // 当前移动方向
Cell nextDirection;           // 下一次移动时使用的方向
Cell food;                     // 食物位置
GameStatus gameStatus;         // 游戏状态
int currentLevel;              // 当前关卡编号：1 到 5，只决定地图大小
int gridSize;                  // 当前关卡地图大小
int targetScore;               // 当前关卡目标分数
int customSpeedLevel;          // 主页面里玩家自己设置的速度等级
int difficultyLevel;           // 难度等级，控制目标分数高低
int score;                     // 分数
int highScore;                 // 最高分
float moveTimer;               // 移动计时器
bool gameStarted;              // 是否已开始（第一次按键后开始移动）
float startMoveInterval;       // 当前关卡初始移动间隔
float minMoveInterval;         // 当前关卡最快移动间隔
float speedUpAmount;           // 当前关卡每次加速减少的间隔
float eatEffectTimer;           // 吃到食物后的特效计时器
float scoreFlashTimer;          // 分数高亮计时器
Cell lastFoodCell;              // 最近一次被吃掉的食物位置

// 摄像机
Camera3D camera;

// ===== 函数声明 =====
void InitGame();
void UpdateGame();
void DrawGame3D();
void DrawUI();
void HandleInput();
void MoveSnake();
void SpawnFood();
void SetLevel(int level);
void StartSelectedGame();
void SetSpeedByLevel(int speedLevel);
void SetTargetScoreByDifficulty();
void HandleLevelSelectInput();
bool IsSameCell(Cell a, Cell b);
bool IsCellOnSnake(Cell cell);
bool CheckWallCollision(Cell head);
bool CheckSelfCollision(Cell head, bool willGrow);
Vector3 CellToWorld(Cell cell);
float GetCurrentMoveInterval();
void DrawGridGround();
void DrawSnake();
void DrawSnakeHead(Vector3 pos);
void DrawSnakeBody(Vector3 pos, int index, int totalLength);
void DrawFood();
void DrawEatEffect();
void DrawWalls();
void DrawCenterMessage(const char* title, const char* subtitle, Color titleColor);
void DrawLevelSelectUI();
void DrawButton(Rectangle button, const char* text, bool selected);
bool IsButtonClicked(Rectangle button);
Rectangle GetLevelButtonRect(int index);
Rectangle GetSpeedMinusButtonRect();
Rectangle GetSpeedPlusButtonRect();
Rectangle GetDifficultyMinusButtonRect();
Rectangle GetDifficultyPlusButtonRect();
Rectangle GetStartButtonRect();
void TryChangeDirection(Cell newDirection);
void UpdateEffects();
void UpdateCameraPosition();
int LoadHighScore();
void SaveHighScore();
void UpdateHighScore();

// ===== 函数实现 =====

// 判断两个格子是否相同
bool IsSameCell(Cell a, Cell b) {
    return (a.x == b.x) && (a.z == b.z);
}

// 判断某个格子是否在蛇身上
bool IsCellOnSnake(Cell cell) {
    for (int i = 0; i < (int)snake.size(); i++) {
        if (IsSameCell(snake[i], cell)) {
            return true;
        }
    }
    return false;
}

// 判断蛇头是否撞墙
bool CheckWallCollision(Cell head) {
    return (head.x < 0 || head.x >= gridSize ||
            head.z < 0 || head.z >= gridSize);
}

// 判断蛇头是否撞到自己身体
bool CheckSelfCollision(Cell head, bool willGrow) {
    // 没吃到食物时，尾巴会离开原来的格子，所以最后一节可以不检查
    int checkEnd = (int)snake.size();
    if (!willGrow) {
        checkEnd--;
    }

    // 从索引 1 开始检查，因为索引 0 是蛇头自己
    for (int i = 1; i < checkEnd; i++) {
        if (IsSameCell(snake[i], head)) {
            return true;
        }
    }
    return false;
}

// 将地图格子坐标转换为 3D 世界坐标
Vector3 CellToWorld(Cell cell) {
    float worldX = (cell.x - gridSize / 2.0f + 0.5f) * CUBE_SIZE;
    float worldZ = (cell.z - gridSize / 2.0f + 0.5f) * CUBE_SIZE;
    return (Vector3){worldX, 0.5f, worldZ};
}

// 根据分数计算当前移动间隔
float GetCurrentMoveInterval() {
    int speedLevel = score / SPEED_UP_SCORE;
    float interval = startMoveInterval - speedLevel * speedUpAmount;

    if (interval < minMoveInterval) {
        interval = minMoveInterval;
    }

    return interval;
}

// 读取最高分文件。如果文件不存在，就从 0 分开始。
int LoadHighScore() {
    int savedScore = 0;
    std::ifstream inputFile(HIGH_SCORE_FILE);

    if (inputFile.is_open()) {
        inputFile >> savedScore;
        inputFile.close();
    }

    return savedScore;
}

// 保存最高分到文本文件
void SaveHighScore() {
    std::ofstream outputFile(HIGH_SCORE_FILE);

    if (outputFile.is_open()) {
        outputFile << highScore;
        outputFile.close();
    }
}

// 如果当前分数超过最高分，就更新并保存
void UpdateHighScore() {
    if (score > highScore) {
        highScore = score;
        SaveHighScore();
    }
}

// 根据速度等级设置移动速度。等级越高，蛇移动越快。
void SetSpeedByLevel(int speedLevel) {
    if (speedLevel <= 1) {
        startMoveInterval = 0.28f;
        minMoveInterval = 0.14f;
        speedUpAmount = 0.006f;
    } else if (speedLevel == 2) {
        startMoveInterval = 0.22f;
        minMoveInterval = 0.11f;
        speedUpAmount = 0.008f;
    } else if (speedLevel == 4) {
        startMoveInterval = 0.14f;
        minMoveInterval = 0.07f;
        speedUpAmount = 0.012f;
    } else if (speedLevel >= 5) {
        startMoveInterval = 0.10f;
        minMoveInterval = 0.05f;
        speedUpAmount = 0.014f;
    } else {
        startMoveInterval = 0.18f;
        minMoveInterval = 0.08f;
        speedUpAmount = 0.01f;
    }
}

// 设置关卡参数。关卡只决定地图大小。
void SetLevel(int level) {
    currentLevel = level;

    if (level == 1) {
        gridSize = 10;
    } else if (level == 2) {
        gridSize = 15;
    } else if (level == 3) {
        gridSize = 20;
    } else if (level == 4) {
        gridSize = 25;
    } else {
        currentLevel = 1;
        gridSize = DEFAULT_GRID_SIZE;
    }
}

// 难度越高，目标分数越高。
void SetTargetScoreByDifficulty() {
    targetScore = gridSize * difficultyLevel * 2;
}

// 使用主页面选择的地图、速度和难度开始游戏
void StartSelectedGame() {
    SetLevel(currentLevel);
    SetSpeedByLevel(customSpeedLevel);
    SetTargetScoreByDifficulty();
}

// 初始化游戏数据
void InitGame() {
    // 清空蛇身
    snake.clear();

    // 蛇初始位置：放在地图中央，朝右
    Cell start = {gridSize / 2, gridSize / 2};
    snake.push_back(start);                     // 蛇头
    snake.push_back({start.x - 1, start.z});    // 身体
    snake.push_back({start.x - 2, start.z});    // 身体

    // 初始方向：向右
    direction = {1, 0};
    nextDirection = direction;

    // 重置分数
    score = 0;

    // 重置计时器
    moveTimer = 0.0f;

    // 游戏状态设为等待开始
    gameStatus = Playing;

    // 游戏未开始移动（等待第一次按键）
    gameStarted = false;

    // 清空临时特效
    eatEffectTimer = 0.0f;
    scoreFlashTimer = 0.0f;
    lastFoodCell = {-1, -1};

    // 生成第一个食物
    SpawnFood();
}

// 随机生成食物（确保不在蛇身上）
void SpawnFood() {
    // 收集所有不在蛇身上的空格子
    std::vector<Cell> emptyCells;
    for (int x = 0; x < gridSize; x++) {
        for (int z = 0; z < gridSize; z++) {
            Cell cell = {x, z};
            if (!IsCellOnSnake(cell)) {
                emptyCells.push_back(cell);
            }
        }
    }

    // 如果有空格子，随机选一个
    if (!emptyCells.empty()) {
        int index = rand() % emptyCells.size();
        food = emptyCells[index];
    } else {
        // 没有空格子，说明蛇已经占满地图，玩家获胜
        food = {-1, -1};
        gameStatus = Win;
        UpdateHighScore();
    }
}

// ===== 主菜单按钮坐标函数 =====

Rectangle GetLevelButtonRect(int index) {
    int buttonWidth = 155;
    int buttonHeight = 65;
    int buttonGap = 25;
    int totalWidth = buttonWidth * 4 + buttonGap * 3;
    int startX = MENU_X + (MENU_WIDTH - totalWidth) / 2;
    int y = MENU_Y + 190;
    return {
        (float)(startX + index * (buttonWidth + buttonGap)),
        (float)y,
        (float)buttonWidth,
        (float)buttonHeight
    };
}

Rectangle GetSpeedMinusButtonRect() {
    return { (float)(MENU_X + 110), (float)(MENU_Y + 400), 65.0f, 50.0f };
}

Rectangle GetSpeedPlusButtonRect() {
    return { (float)(MENU_X + MENU_WIDTH - 175), (float)(MENU_Y + 400), 65.0f, 50.0f };
}

Rectangle GetDifficultyMinusButtonRect() {
    return { (float)(MENU_X + 110), (float)(MENU_Y + 550), 65.0f, 50.0f };
}

Rectangle GetDifficultyPlusButtonRect() {
    return { (float)(MENU_X + MENU_WIDTH - 175), (float)(MENU_Y + 550), 65.0f, 50.0f };
}

Rectangle GetStartButtonRect() {
    int buttonWidth = 240;
    int buttonHeight = 70;
    return {
        (float)(MENU_X + (MENU_WIDTH - buttonWidth) / 2),
        (float)(MENU_Y + 650),
        (float)buttonWidth,
        (float)buttonHeight
    };
}

// 在主页面处理鼠标点击
void HandleLevelSelectInput() {
    for (int i = 0; i < 4; i++) {
        if (IsButtonClicked(GetLevelButtonRect(i))) {
            SetLevel(i + 1);
        }
    }

    if (IsButtonClicked(GetSpeedMinusButtonRect())) {
        customSpeedLevel--;
        if (customSpeedLevel < MIN_CUSTOM_SPEED_LEVEL) {
            customSpeedLevel = MIN_CUSTOM_SPEED_LEVEL;
        }
    }
    if (IsButtonClicked(GetSpeedPlusButtonRect())) {
        customSpeedLevel++;
        if (customSpeedLevel > MAX_CUSTOM_SPEED_LEVEL) {
            customSpeedLevel = MAX_CUSTOM_SPEED_LEVEL;
        }
    }

    if (IsButtonClicked(GetDifficultyMinusButtonRect())) {
        difficultyLevel--;
        if (difficultyLevel < MIN_DIFFICULTY_LEVEL) {
            difficultyLevel = MIN_DIFFICULTY_LEVEL;
        }
    }
    if (IsButtonClicked(GetDifficultyPlusButtonRect())) {
        difficultyLevel++;
        if (difficultyLevel > MAX_DIFFICULTY_LEVEL) {
            difficultyLevel = MAX_DIFFICULTY_LEVEL;
        }
    }

    if (IsButtonClicked(GetStartButtonRect())) {
        StartSelectedGame();
        InitGame();
    }
}

// 尝试改变蛇的移动方向
void TryChangeDirection(Cell newDirection) {
    // 不允许直接反方向移动
    if (newDirection.x + direction.x == 0 &&
        newDirection.z + direction.z == 0) {
        return;
    }

    nextDirection = newDirection;
    gameStarted = true;
}

// 处理键盘输入
void HandleInput() {
    if (gameStatus == LevelSelect) {
        HandleLevelSelectInput();
        return;
    }

    // 返回主页面
    if (IsKeyPressed(KEY_M)) {
        gameStatus = LevelSelect;
        gameStarted = false;
        snake.clear();
        food = {-1, -1};
        return;
    }

    // 方向控制
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        TryChangeDirection({0, -1});
    }
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        TryChangeDirection({0, 1});
    }
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
        TryChangeDirection({-1, 0});
    }
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
        TryChangeDirection({1, 0});
    }

    // 暂停 / 继续
    if (IsKeyPressed(KEY_SPACE)) {
        if (gameStatus == Playing) {
            gameStatus = Paused;
        } else if (gameStatus == Paused) {
            gameStatus = Playing;
        }
    }

    // 重新开始
    if (IsKeyPressed(KEY_R)) {
        InitGame();
    }
}

// 移动蛇一格
void MoveSnake() {
    // 使用玩家最近一次输入的方向
    direction = nextDirection;

    // 计算新蛇头位置
    Cell newHead;
    newHead.x = snake[0].x + direction.x;
    newHead.z = snake[0].z + direction.z;

    bool willGrow = IsSameCell(newHead, food);

    // 检查碰撞
    if (CheckWallCollision(newHead) || CheckSelfCollision(newHead, willGrow)) {
        gameStatus = GameOver;
        return;
    }

    // 在蛇头前面插入新格子
    snake.insert(snake.begin(), newHead);

    // 判断是否吃到食物
    if (IsSameCell(newHead, food)) {
        lastFoodCell = food;
        eatEffectTimer = EAT_EFFECT_TIME;
        scoreFlashTimer = SCORE_FLASH_TIME;
        score += 10;
        UpdateHighScore();

        if (score >= targetScore) {
            gameStatus = Win;
            food = {-1, -1};
        } else {
            SpawnFood();
        }
        // 吃到食物时不移除尾部，蛇身自然变长
    } else {
        // 没吃到食物则移除尾部，保持蛇身长度不变
        snake.pop_back();
    }
}

// 更新游戏逻辑
void UpdateGame() {
    // 只有游戏进行中且已开始才更新
    if (gameStatus != Playing || !gameStarted) {
        return;
    }

    // 计时器累加
    moveTimer += GetFrameTime();

    // 到达移动间隔时移动蛇
    if (moveTimer >= GetCurrentMoveInterval()) {
        moveTimer = 0.0f;
        MoveSnake();
    }
}

// 更新一些简单的临时动画计时器
void UpdateEffects() {
    float deltaTime = GetFrameTime();

    if (eatEffectTimer > 0.0f) {
        eatEffectTimer -= deltaTime;
        if (eatEffectTimer < 0.0f) {
            eatEffectTimer = 0.0f;
        }
    }

    if (scoreFlashTimer > 0.0f) {
        scoreFlashTimer -= deltaTime;
        if (scoreFlashTimer < 0.0f) {
            scoreFlashTimer = 0.0f;
        }
    }
}

// 设置固定摄像机位置
void UpdateCameraPosition() {
    camera.position = (Vector3){0.0f, 22.0f, 24.0f};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}

// 绘制地面网格
void DrawGridGround() {
    // 用棋盘格让地图方向和格子更清楚
    for (int x = 0; x < gridSize; x++) {
        for (int z = 0; z < gridSize; z++) {
            float worldX = (x - gridSize / 2.0f + 0.5f) * CUBE_SIZE;
            float worldZ = (z - gridSize / 2.0f + 0.5f) * CUBE_SIZE;

            Color color;
            if ((x + z) % 2 == 0) {
                color = (Color){46, 54, 64, 255};
            } else {
                color = (Color){38, 45, 55, 255};
            }

            DrawCube((Vector3){worldX, -0.06f, worldZ}, CUBE_SIZE, 0.08f, CUBE_SIZE, color);
            DrawCubeWires((Vector3){worldX, -0.015f, worldZ}, CUBE_SIZE, 0.01f, CUBE_SIZE, (Color){70, 80, 92, 120});
        }
    }
}

// 绘制蛇头。眼睛会跟随移动方向变化。
void DrawSnakeHead(Vector3 pos) {
    DrawCube(pos, CUBE_SIZE * 0.92f, CUBE_SIZE * 0.92f, CUBE_SIZE * 0.92f, (Color){28, 150, 75, 255});
    DrawCubeWires(pos, CUBE_SIZE * 0.92f, CUBE_SIZE * 0.92f, CUBE_SIZE * 0.92f, BLACK);

    float eyeOffsetSide = 0.18f;
    float eyeOffsetFront = 0.46f;
    float eyeY = pos.y + 0.18f;

    Vector3 leftEye = pos;
    Vector3 rightEye = pos;

    if (direction.x != 0) {
        leftEye.x += direction.x * eyeOffsetFront;
        rightEye.x += direction.x * eyeOffsetFront;
        leftEye.z -= eyeOffsetSide;
        rightEye.z += eyeOffsetSide;
    } else {
        leftEye.z += direction.z * eyeOffsetFront;
        rightEye.z += direction.z * eyeOffsetFront;
        leftEye.x -= eyeOffsetSide;
        rightEye.x += eyeOffsetSide;
    }

    leftEye.y = eyeY;
    rightEye.y = eyeY;

    DrawSphere(leftEye, 0.08f, WHITE);
    DrawSphere(rightEye, 0.08f, WHITE);
    DrawSphere((Vector3){leftEye.x, leftEye.y, leftEye.z}, 0.035f, BLACK);
    DrawSphere((Vector3){rightEye.x, rightEye.y, rightEye.z}, 0.035f, BLACK);
}

// 绘制蛇身。越靠近尾巴，方块稍微小一点。
void DrawSnakeBody(Vector3 pos, int index, int totalLength) {
    float size = CUBE_SIZE * 0.84f;
    if (totalLength > 1) {
        float tailProgress = (float)index / (float)(totalLength - 1);
        size = CUBE_SIZE * (0.86f - tailProgress * 0.14f);
    }

    int green = 180 - index * 3;
    if (green < 95) {
        green = 95;
    }

    Color bodyColor = (Color){55, (unsigned char)green, 85, 255};
    DrawCube(pos, size, size, size, bodyColor);
    DrawCubeWires(pos, size, size, size, (Color){10, 40, 20, 255});
}

// 绘制蛇
void DrawSnake() {
    for (int i = 0; i < (int)snake.size(); i++) {
        Vector3 pos = CellToWorld(snake[i]);

        if (i == 0) {
            DrawSnakeHead(pos);
        } else {
            DrawSnakeBody(pos, i, (int)snake.size());
        }
    }
}

// 绘制食物
void DrawFood() {
    if (food.x < 0 || food.z < 0) {
        return;
    }

    Vector3 pos = CellToWorld(food);
    pos.y += sinf((float)GetTime() * FOOD_FLOAT_SPEED) * FOOD_FLOAT_HEIGHT;

    float pulse = (sinf((float)GetTime() * 6.0f) + 1.0f) * 0.5f;
    float ringRadius = CUBE_SIZE * (0.48f + pulse * 0.12f);

    // 用红色球体表示食物，比方块更醒目
    DrawSphere(pos, CUBE_SIZE * 0.4f, RED);
    // 食物加发光感的外圈
    DrawSphereWires(pos, ringRadius, 10, 10, (Color){255, 170, 120, 255});
    DrawCylinder((Vector3){pos.x, 0.02f, pos.z}, ringRadius, ringRadius, 0.03f, 24, (Color){180, 45, 45, 120});
}

// 绘制吃到食物后的短暂扩散效果
void DrawEatEffect() {
    if (eatEffectTimer <= 0.0f || lastFoodCell.x < 0 || lastFoodCell.z < 0) {
        return;
    }

    float progress = 1.0f - eatEffectTimer / EAT_EFFECT_TIME;
    Vector3 pos = CellToWorld(lastFoodCell);
    float radius = CUBE_SIZE * (0.4f + progress * 1.2f);
    unsigned char alpha = (unsigned char)(180 * (1.0f - progress));

    DrawSphereWires(pos, radius, 12, 12, (Color){255, 210, 80, alpha});
}

// 绘制墙壁边框
void DrawWalls() {
    float half = gridSize * CUBE_SIZE / 2.0f;
    float edge = half + 0.5f;  // 墙壁放在可行走区域外侧
    float y = 0.5f;

    // 在四条边界画矮墙，表示地图边界
    for (int i = 0; i < gridSize; i++) {
        float offset = (i - gridSize / 2.0f + 0.5f) * CUBE_SIZE;

        // 上边界、下边界
        DrawCube((Vector3){offset, y, -edge}, CUBE_SIZE, CUBE_SIZE * 1.2f, CUBE_SIZE, (Color){110, 118, 128, 230});
        DrawCube((Vector3){offset, y, edge}, CUBE_SIZE, CUBE_SIZE * 1.2f, CUBE_SIZE, (Color){110, 118, 128, 230});
        // 左边界、右边界
        DrawCube((Vector3){-edge, y, offset}, CUBE_SIZE, CUBE_SIZE * 1.2f, CUBE_SIZE, (Color){110, 118, 128, 230});
        DrawCube((Vector3){edge, y, offset}, CUBE_SIZE, CUBE_SIZE * 1.2f, CUBE_SIZE, (Color){110, 118, 128, 230});
    }

    // 四个角落做成明显一些的标记，方便旋转视角后辨认地图方向
    Color cornerColor = (Color){245, 190, 80, 255};
    DrawCube((Vector3){-edge, 1.05f, -edge}, CUBE_SIZE * 1.2f, CUBE_SIZE * 1.6f, CUBE_SIZE * 1.2f, cornerColor);
    DrawCube((Vector3){edge, 1.05f, -edge}, CUBE_SIZE * 1.2f, CUBE_SIZE * 1.6f, CUBE_SIZE * 1.2f, cornerColor);
    DrawCube((Vector3){-edge, 1.05f, edge}, CUBE_SIZE * 1.2f, CUBE_SIZE * 1.6f, CUBE_SIZE * 1.2f, cornerColor);
    DrawCube((Vector3){edge, 1.05f, edge}, CUBE_SIZE * 1.2f, CUBE_SIZE * 1.6f, CUBE_SIZE * 1.2f, cornerColor);
}

// 绘制 3D 场景
void DrawGame3D() {
    DrawGridGround();
    DrawWalls();
    DrawFood();
    DrawEatEffect();
    DrawSnake();
}

// 绘制居中的状态提示，暂停、失败、胜利都可以复用
void DrawCenterMessage(const char* title, const char* subtitle, Color titleColor) {
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

// 判断按钮是否被鼠标左键点击
bool IsButtonClicked(Rectangle button) {
    return CheckCollisionPointRec(GetMousePosition(), button) &&
           IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

// 绘制一个简单按钮
void DrawButton(Rectangle button, const char* text, bool selected) {
    bool mouseOver = CheckCollisionPointRec(GetMousePosition(), button);

    Color fillColor = (Color){45, 55, 70, 230};
    Color lineColor = (Color){170, 180, 195, 220};
    Color textColor = LIGHTGRAY;

    if (selected) {
        fillColor = (Color){60, 120, 80, 240};
        lineColor = (Color){230, 230, 160, 255};
        textColor = WHITE;
    } else if (mouseOver) {
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

// 绘制简单选关界面
void DrawLevelSelectUI() {
    // 半透明菜单面板
    DrawRectangle(MENU_X, MENU_Y, MENU_WIDTH, MENU_HEIGHT, (Color){10, 15, 30, 200});
    DrawRectangleLines(MENU_X, MENU_Y, MENU_WIDTH, MENU_HEIGHT, (Color){180, 180, 200, 160});

    // 标题
    const char* title = "SNAKE MENU";
    int titleSize = 72;
    int titleWidth = MeasureText(title, titleSize);
    DrawText(title, (SCREEN_WIDTH - titleWidth) / 2, MENU_Y + 50, titleSize, YELLOW);

    // 地图关卡区域
    const char* mapLabel = "Choose Map";
    int mapLabelSize = 32;
    int mapLabelWidth = MeasureText(mapLabel, mapLabelSize);
    DrawText(mapLabel, (SCREEN_WIDTH - mapLabelWidth) / 2, MENU_Y + 140, mapLabelSize, WHITE);

    for (int i = 0; i < 4; i++) {
        Rectangle btn = GetLevelButtonRect(i);
        std::string text = "Level " + std::to_string(i + 1);
        DrawButton(btn, text.c_str(), currentLevel == i + 1);

        std::string mapText = std::to_string(10 + i * 5) + " x " + std::to_string(10 + i * 5);
        int mapTextWidth = MeasureText(mapText.c_str(), 22);
        DrawText(mapText.c_str(),
                 (int)(btn.x + (btn.width - mapTextWidth) / 2),
                 (int)(btn.y + btn.height + 8),
                 22, LIGHTGRAY);
    }

    // 速度调节区域
    const char* speedLabel = "Speed";
    int speedLabelSize = 32;
    int speedLabelWidth = MeasureText(speedLabel, speedLabelSize);
    DrawText(speedLabel, (SCREEN_WIDTH - speedLabelWidth) / 2, MENU_Y + 340, speedLabelSize, WHITE);

    DrawButton(GetSpeedMinusButtonRect(), "-", false);
    DrawButton(GetSpeedPlusButtonRect(), "+", false);
    std::string speedText = "Level " + std::to_string(customSpeedLevel);
    int speedTextSize = 34;
    int speedTextWidth = MeasureText(speedText.c_str(), speedTextSize);
    DrawText(speedText.c_str(), (SCREEN_WIDTH - speedTextWidth) / 2, MENU_Y + 408, speedTextSize, WHITE);

    // 难度目标区域
    const char* diffLabel = "Difficulty Target";
    int diffLabelSize = 32;
    int diffLabelWidth = MeasureText(diffLabel, diffLabelSize);
    DrawText(diffLabel, (SCREEN_WIDTH - diffLabelWidth) / 2, MENU_Y + 490, diffLabelSize, WHITE);

    DrawButton(GetDifficultyMinusButtonRect(), "-", false);
    DrawButton(GetDifficultyPlusButtonRect(), "+", false);
    SetTargetScoreByDifficulty();
    std::string difficultyText = "Level " + std::to_string(difficultyLevel) +
                                 "  Target " + std::to_string(targetScore);
    int diffTextSize = 32;
    int diffTextWidth = MeasureText(difficultyText.c_str(), diffTextSize);
    DrawText(difficultyText.c_str(), (SCREEN_WIDTH - diffTextWidth) / 2, MENU_Y + 558, diffTextSize, WHITE);

    // 开始按钮
    DrawButton(GetStartButtonRect(), "START", false);
}

// 绘制 UI 界面（2D）
void DrawUI() {
    if (gameStatus == LevelSelect) {
        DrawLevelSelectUI();
        return;
    }

    // 分数显示
    std::string scoreText = "Score: " + std::to_string(score);
    std::string highScoreText = "High Score: " + std::to_string(highScore);
    std::string speedText = "Speed Level: " + std::to_string(score / SPEED_UP_SCORE + 1);
    std::string levelName = "Custom";
    if (currentLevel == 1) {
        levelName = "Level 1";
    } else if (currentLevel == 2) {
        levelName = "Level 2";
    } else if (currentLevel == 3) {
        levelName = "Level 3";
    } else if (currentLevel == 4) {
        levelName = "Level 4";
    }
    std::string levelText = "Level: " + levelName +
                            " | Map: " + std::to_string(gridSize) + " x " + std::to_string(gridSize);
    std::string targetText = "Target: " + std::to_string(targetScore);

    Color scoreColor = WHITE;
    if (scoreFlashTimer > 0.0f) {
        scoreColor = YELLOW;
    }

    DrawText(scoreText.c_str(), 20, 20, 30, scoreColor);
    DrawText(highScoreText.c_str(), 20, 55, 22, LIGHTGRAY);
    DrawText(speedText.c_str(), 20, 85, 22, LIGHTGRAY);
    DrawText(targetText.c_str(), 20, 115, 22, LIGHTGRAY);
    DrawText(levelText.c_str(), 20, 145, 22, LIGHTGRAY);

    // 控制提示
    DrawText("WASD / Arrow Keys: Move", 20, SCREEN_HEIGHT - 120, 20, LIGHTGRAY);
    DrawText("M: Back to Menu", 20, SCREEN_HEIGHT - 85, 20, LIGHTGRAY);
    DrawText("Space: Pause | R: Restart | Esc: Quit", 20, SCREEN_HEIGHT - 45, 20, LIGHTGRAY);

    // 暂停提示
    if (gameStatus == Paused) {
        DrawCenterMessage("PAUSED", "Press Space to continue", YELLOW);
    }

    // 游戏结束提示
    if (gameStatus == GameOver) {
        std::string resultText = "Final Score: " + std::to_string(score) + " | R restart | M menu";
        DrawCenterMessage("GAME OVER", resultText.c_str(), RED);
    }

    // 胜利提示
    if (gameStatus == Win) {
        std::string resultText = levelName + " Clear | Score: " + std::to_string(score) + " | M menu";
        DrawCenterMessage("YOU WIN", resultText.c_str(), GREEN);
    }

    // 首次提示（等待按键开始）
    if (!gameStarted && gameStatus == Playing) {
        const char* msg = "Press WASD or Arrow Keys to start";
        int fontSize = 25;
        int textWidth = MeasureText(msg, fontSize);
        DrawText(msg, (SCREEN_WIDTH - textWidth) / 2, SCREEN_HEIGHT / 2 + 200, fontSize, (Color){200, 200, 200, 180});
    }
}

// ===== 主函数 =====
int main() {
    // 初始化随机数种子
    srand((unsigned int)time(nullptr));

    // 创建窗口
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "3D Snake Game");
    SetTargetFPS(60);

    // 设置摄像机：固定在斜上方俯视地图中央
    UpdateCameraPosition();

    // 读取历史最高分
    highScore = LoadHighScore();

    // 默认显示普通关卡参数，启动后先进入选关界面
    customSpeedLevel = 3;
    difficultyLevel = 3;
    SetLevel(1);
    SetTargetScoreByDifficulty();
    gameStatus = LevelSelect;

    // 主循环
    while (!WindowShouldClose()) {
        HandleInput();
        UpdateGame();
        UpdateEffects();

        BeginDrawing();
        ClearBackground((Color){20, 25, 40, 255});

        if (gameStatus != LevelSelect) {
            BeginMode3D(camera);
            DrawGame3D();
            EndMode3D();
        }

        DrawUI();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
