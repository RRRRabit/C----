#include "raylib.h"
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <fstream>

// ===== 常量参数 =====
const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 700;

const int GRID_SIZE = 20;
const float CUBE_SIZE = 1.0f;
const float START_MOVE_INTERVAL = 0.18f;  // 初始移动间隔，数值越小速度越快
const float MIN_MOVE_INTERVAL = 0.08f;    // 最快速度限制
const int SPEED_UP_SCORE = 50;            // 每获得多少分提高一次速度
const float SPEED_UP_AMOUNT = 0.01f;      // 每次提高速度时减少的移动间隔
const char* HIGH_SCORE_FILE = "highscore.txt";

const float CAMERA_DISTANCE = 24.0f;
const float CAMERA_HEIGHT = 18.0f;
const float CAMERA_ROTATE_SPEED = 1.8f;
const float MOUSE_ROTATE_SPEED = 0.01f;
const float FOOD_FLOAT_HEIGHT = 0.15f;
const float FOOD_FLOAT_SPEED = 4.0f;

// ===== 数据结构 =====

// 地图格子坐标
struct Cell {
    int x;
    int z;
};

// 游戏状态
enum GameStatus {
    Playing,
    Paused,
    GameOver,
    Win
};

// ===== 游戏全局数据 =====
std::vector<Cell> snake;      // 蛇身体，snake[0] 是蛇头
std::vector<Cell> oldSnake;   // 上一次移动前的蛇身体，用来画平滑移动动画
Cell direction;               // 当前移动方向
Cell nextDirection;           // 下一次移动时使用的方向
Cell food;                     // 食物位置
GameStatus gameStatus;         // 游戏状态
int score;                     // 分数
int highScore;                 // 最高分
float moveTimer;               // 移动计时器
bool gameStarted;              // 是否已开始（第一次按键后开始移动）
float cameraAngle;             // 摄像机围绕地图旋转的角度

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
bool IsSameCell(Cell a, Cell b);
bool IsCellOnSnake(Cell cell);
bool CheckWallCollision(Cell head);
bool CheckSelfCollision(Cell head, bool willGrow);
Vector3 CellToWorld(Cell cell);
Vector3 SmoothCellToWorld(Cell oldCell, Cell newCell, float progress);
float GetMoveProgress();
float GetCurrentMoveInterval();
void DrawGridGround();
void DrawSnake();
void DrawFood();
void DrawWalls();
void TryChangeDirection(Cell newDirection);
void UpdateCameraRotation();
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
    return (head.x < 0 || head.x >= GRID_SIZE ||
            head.z < 0 || head.z >= GRID_SIZE);
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
    float worldX = (cell.x - GRID_SIZE / 2.0f + 0.5f) * CUBE_SIZE;
    float worldZ = (cell.z - GRID_SIZE / 2.0f + 0.5f) * CUBE_SIZE;
    return (Vector3){worldX, 0.5f, worldZ};
}

// 在旧位置和新位置之间取一个中间位置，让移动看起来更平滑
Vector3 SmoothCellToWorld(Cell oldCell, Cell newCell, float progress) {
    Vector3 oldPos = CellToWorld(oldCell);
    Vector3 newPos = CellToWorld(newCell);

    Vector3 result;
    result.x = oldPos.x + (newPos.x - oldPos.x) * progress;
    result.y = oldPos.y + (newPos.y - oldPos.y) * progress;
    result.z = oldPos.z + (newPos.z - oldPos.z) * progress;
    return result;
}

// 获取当前移动动画的进度，范围是 0 到 1
float GetMoveProgress() {
    if (!gameStarted) {
        return 1.0f;
    }

    float progress = moveTimer / GetCurrentMoveInterval();
    if (progress < 0.0f) {
        progress = 0.0f;
    }
    if (progress > 1.0f) {
        progress = 1.0f;
    }
    return progress;
}

// 根据分数计算当前移动间隔
float GetCurrentMoveInterval() {
    int speedLevel = score / SPEED_UP_SCORE;
    float interval = START_MOVE_INTERVAL - speedLevel * SPEED_UP_AMOUNT;

    if (interval < MIN_MOVE_INTERVAL) {
        interval = MIN_MOVE_INTERVAL;
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

// 初始化游戏数据
void InitGame() {
    // 清空蛇身
    snake.clear();

    // 蛇初始位置：放在地图中央，朝右
    Cell start = {GRID_SIZE / 2, GRID_SIZE / 2};
    snake.push_back(start);                     // 蛇头
    snake.push_back({start.x - 1, start.z});    // 身体
    snake.push_back({start.x - 2, start.z});    // 身体
    oldSnake = snake;

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

    // 生成第一个食物
    SpawnFood();
}

// 随机生成食物（确保不在蛇身上）
void SpawnFood() {
    // 收集所有不在蛇身上的空格子
    std::vector<Cell> emptyCells;
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int z = 0; z < GRID_SIZE; z++) {
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
        moveTimer = GetCurrentMoveInterval();
        UpdateHighScore();
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
    // 保存移动前的位置，绘制时会用它来做平滑动画
    oldSnake = snake;

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
        score += 10;
        UpdateHighScore();
        SpawnFood();
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

// 根据当前角度更新摄像机位置
void UpdateCameraPosition() {
    camera.position = (Vector3){
        sinf(cameraAngle) * CAMERA_DISTANCE,
        CAMERA_HEIGHT,
        cosf(cameraAngle) * CAMERA_DISTANCE
    };
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}

// 更新摄像机旋转，支持 Q/E 和鼠标右键拖动
void UpdateCameraRotation() {
    if (IsKeyDown(KEY_Q)) {
        cameraAngle -= CAMERA_ROTATE_SPEED * GetFrameTime();
    }
    if (IsKeyDown(KEY_E)) {
        cameraAngle += CAMERA_ROTATE_SPEED * GetFrameTime();
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 mouseDelta = GetMouseDelta();
        cameraAngle -= mouseDelta.x * MOUSE_ROTATE_SPEED;
    }

    UpdateCameraPosition();
}

// 绘制地面网格
void DrawGridGround() {
    // 绘制网格线（在地面上）
    DrawGrid(GRID_SIZE, CUBE_SIZE);

    // 绘制地面平面（略低于蛇的位置，让蛇看起来站在上面）
    float planeSize = GRID_SIZE * CUBE_SIZE;
    DrawPlane((Vector3){0.0f, -0.01f, 0.0f}, (Vector2){planeSize, planeSize}, DARKGRAY);
}

// 绘制蛇
void DrawSnake() {
    float progress = GetMoveProgress();

    for (int i = 0; i < (int)snake.size(); i++) {
        Cell oldCell = snake[i];
        if (i < (int)oldSnake.size()) {
            oldCell = oldSnake[i];
        } else if (!oldSnake.empty()) {
            oldCell = oldSnake.back();
        }

        Vector3 pos = SmoothCellToWorld(oldCell, snake[i], progress);

        if (i == 0) {
            // 蛇头：深绿色
            DrawCube(pos, CUBE_SIZE * 0.9f, CUBE_SIZE * 0.9f, CUBE_SIZE * 0.9f, DARKGREEN);
            // 蛇头加边框
            DrawCubeWires(pos, CUBE_SIZE * 0.9f, CUBE_SIZE * 0.9f, CUBE_SIZE * 0.9f, BLACK);
        } else {
            // 蛇身：绿色
            DrawCube(pos, CUBE_SIZE * 0.85f, CUBE_SIZE * 0.85f, CUBE_SIZE * 0.85f, GREEN);
            DrawCubeWires(pos, CUBE_SIZE * 0.85f, CUBE_SIZE * 0.85f, CUBE_SIZE * 0.85f, BLACK);
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

    // 用红色球体表示食物，比方块更醒目
    DrawSphere(pos, CUBE_SIZE * 0.4f, RED);
    // 食物加发光感的外圈
    DrawSphereWires(pos, CUBE_SIZE * 0.45f, 8, 8, MAROON);
}

// 绘制墙壁边框
void DrawWalls() {
    float half = GRID_SIZE * CUBE_SIZE / 2.0f;
    float edge = half + 0.5f;  // 墙壁放在可行走区域外侧
    float y = 0.5f;

    // 在四条边界画矮墙，表示地图边界
    for (int i = 0; i < GRID_SIZE; i++) {
        float offset = (i - GRID_SIZE / 2.0f + 0.5f) * CUBE_SIZE;

        // 上边界、下边界
        DrawCube((Vector3){offset, y, -edge}, CUBE_SIZE, CUBE_SIZE, CUBE_SIZE, (Color){100, 100, 100, 200});
        DrawCube((Vector3){offset, y, edge}, CUBE_SIZE, CUBE_SIZE, CUBE_SIZE, (Color){100, 100, 100, 200});
        // 左边界、右边界
        DrawCube((Vector3){-edge, y, offset}, CUBE_SIZE, CUBE_SIZE, CUBE_SIZE, (Color){100, 100, 100, 200});
        DrawCube((Vector3){edge, y, offset}, CUBE_SIZE, CUBE_SIZE, CUBE_SIZE, (Color){100, 100, 100, 200});
    }
}

// 绘制 3D 场景
void DrawGame3D() {
    DrawGridGround();
    DrawWalls();
    DrawFood();
    DrawSnake();
}

// 绘制 UI 界面（2D）
void DrawUI() {
    // 分数显示
    std::string scoreText = "Score: " + std::to_string(score);
    std::string highScoreText = "High Score: " + std::to_string(highScore);
    std::string speedText = "Speed Level: " + std::to_string(score / SPEED_UP_SCORE + 1);

    DrawText(scoreText.c_str(), 20, 20, 30, WHITE);
    DrawText(highScoreText.c_str(), 20, 55, 22, LIGHTGRAY);
    DrawText(speedText.c_str(), 20, 85, 22, LIGHTGRAY);

    // 控制提示
    DrawText("WASD / Arrow Keys: Move", 20, SCREEN_HEIGHT - 105, 20, LIGHTGRAY);
    DrawText("Q / E or Right Mouse Drag: Rotate View", 20, SCREEN_HEIGHT - 75, 20, LIGHTGRAY);
    DrawText("Space: Pause | R: Restart | Esc: Quit", 20, SCREEN_HEIGHT - 45, 20, LIGHTGRAY);

    // 暂停提示
    if (gameStatus == Paused) {
        const char* msg = "PAUSED";
        int fontSize = 60;
        int textWidth = MeasureText(msg, fontSize);
        DrawText(msg, (SCREEN_WIDTH - textWidth) / 2, SCREEN_HEIGHT / 2 - 100, fontSize, YELLOW);
        DrawText("Press Space to continue", (SCREEN_WIDTH - MeasureText("Press Space to continue", 25)) / 2,
                 SCREEN_HEIGHT / 2 - 30, 25, LIGHTGRAY);
    }

    // 游戏结束提示
    if (gameStatus == GameOver) {
        const char* msg1 = "GAME OVER";
        int fontSize1 = 60;
        int textWidth1 = MeasureText(msg1, fontSize1);
        DrawText(msg1, (SCREEN_WIDTH - textWidth1) / 2, SCREEN_HEIGHT / 2 - 100, fontSize1, RED);

        std::string finalScoreText = "Final Score: " + std::to_string(score);
        const char* msg2 = finalScoreText.c_str();
        int fontSize2 = 30;
        int textWidth2 = MeasureText(msg2, fontSize2);
        DrawText(msg2, (SCREEN_WIDTH - textWidth2) / 2, SCREEN_HEIGHT / 2 - 30, fontSize2, WHITE);

        const char* msg3 = "Press R to restart";
        int fontSize3 = 25;
        int textWidth3 = MeasureText(msg3, fontSize3);
        DrawText(msg3, (SCREEN_WIDTH - textWidth3) / 2, SCREEN_HEIGHT / 2 + 20, fontSize3, LIGHTGRAY);
    }

    // 胜利提示
    if (gameStatus == Win) {
        const char* msg1 = "YOU WIN";
        int fontSize1 = 60;
        int textWidth1 = MeasureText(msg1, fontSize1);
        DrawText(msg1, (SCREEN_WIDTH - textWidth1) / 2, SCREEN_HEIGHT / 2 - 100, fontSize1, GREEN);

        std::string finalScoreText = "Final Score: " + std::to_string(score);
        const char* msg2 = finalScoreText.c_str();
        int fontSize2 = 30;
        int textWidth2 = MeasureText(msg2, fontSize2);
        DrawText(msg2, (SCREEN_WIDTH - textWidth2) / 2, SCREEN_HEIGHT / 2 - 30, fontSize2, WHITE);

        const char* msg3 = "Press R to restart";
        int fontSize3 = 25;
        int textWidth3 = MeasureText(msg3, fontSize3);
        DrawText(msg3, (SCREEN_WIDTH - textWidth3) / 2, SCREEN_HEIGHT / 2 + 20, fontSize3, LIGHTGRAY);
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

    // 设置摄像机：从斜上方俯视地图中央
    cameraAngle = 0.0f;
    UpdateCameraPosition();

    // 读取历史最高分
    highScore = LoadHighScore();

    // 初始化游戏
    InitGame();

    // 主循环
    while (!WindowShouldClose()) {
        HandleInput();
        UpdateGame();
        UpdateCameraRotation();

        BeginDrawing();
        ClearBackground((Color){20, 25, 40, 255});

        BeginMode3D(camera);
        DrawGame3D();
        EndMode3D();

        DrawUI();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
