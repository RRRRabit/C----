#include "raylib.h"
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

// ===== 常量参数 =====
const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 700;

const int GRID_SIZE = 20;
const float CUBE_SIZE = 1.0f;
const float MOVE_INTERVAL = 0.18f;

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
    GameOver
};

// ===== 游戏全局数据 =====
std::vector<Cell> snake;      // 蛇身体，snake[0] 是蛇头
Cell direction;               // 当前移动方向
Cell food;                     // 食物位置
GameStatus gameStatus;         // 游戏状态
int score;                     // 分数
float moveTimer;               // 移动计时器
bool gameStarted;              // 是否已开始（第一次按键后开始移动）

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
bool CheckSelfCollision(Cell head);
Vector3 CellToWorld(Cell cell);
void DrawGridGround();
void DrawSnake();
void DrawFood();
void DrawWalls();

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
bool CheckSelfCollision(Cell head) {
    // 从索引 1 开始检查，因为索引 0 是蛇头自己
    for (int i = 1; i < (int)snake.size(); i++) {
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

// 初始化游戏数据
void InitGame() {
    // 清空蛇身
    snake.clear();

    // 蛇初始位置：放在地图中央，朝右
    Cell start = {GRID_SIZE / 2, GRID_SIZE / 2};
    snake.push_back(start);                     // 蛇头
    snake.push_back({start.x - 1, start.z});    // 身体
    snake.push_back({start.x - 2, start.z});    // 身体

    // 初始方向：向右
    direction = {1, 0};

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
    }
}

// 处理键盘输入
void HandleInput() {
    // 方向控制 —— 不允许反向移动
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        if (direction.z != 1) {  // 不能从向下变向上
            direction = {0, -1};
            gameStarted = true;
        }
    }
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        if (direction.z != -1) {  // 不能从向上变向下
            direction = {0, 1};
            gameStarted = true;
        }
    }
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
        if (direction.x != 1) {  // 不能从向右变向左
            direction = {-1, 0};
            gameStarted = true;
        }
    }
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
        if (direction.x != -1) {  // 不能从向左变向右
            direction = {1, 0};
            gameStarted = true;
        }
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
    // 计算新蛇头位置
    Cell newHead;
    newHead.x = snake[0].x + direction.x;
    newHead.z = snake[0].z + direction.z;

    // 检查碰撞
    if (CheckWallCollision(newHead) || CheckSelfCollision(newHead)) {
        gameStatus = GameOver;
        return;
    }

    // 在蛇头前面插入新格子
    snake.insert(snake.begin(), newHead);

    // 判断是否吃到食物
    if (IsSameCell(newHead, food)) {
        score += 10;
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
    if (moveTimer >= MOVE_INTERVAL) {
        moveTimer = 0.0f;
        MoveSnake();
    }
}

// 绘制地面网格
void DrawGridGround() {
    // 绘制网格线（在地面上）
    DrawGrid(GRID_SIZE, CUBE_SIZE);

    // 绘制地面平面（略低于蛇的位置，让蛇看起来站在上面）
    float planeSize = GRID_SIZE * CUBE_SIZE / 2.0f;
    DrawPlane((Vector3){0.0f, -0.01f, 0.0f}, (Vector2){planeSize, planeSize}, DARKGRAY);
}

// 绘制蛇
void DrawSnake() {
    for (int i = 0; i < (int)snake.size(); i++) {
        Vector3 pos = CellToWorld(snake[i]);

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
    Vector3 pos = CellToWorld(food);

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
    DrawText(("Score: " + std::to_string(score)).c_str(), 20, 20, 30, WHITE);

    // 控制提示
    DrawText("WASD / Arrow Keys: Move", 20, SCREEN_HEIGHT - 80, 20, LIGHTGRAY);
    DrawText("Space: Pause | R: Restart | Esc: Quit", 20, SCREEN_HEIGHT - 50, 20, LIGHTGRAY);

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

        const char* msg2 = ("Final Score: " + std::to_string(score)).c_str();
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
    camera.position = (Vector3){0.0f, 20.0f, 14.0f};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // 初始化游戏
    InitGame();

    // 主循环
    while (!WindowShouldClose()) {
        HandleInput();
        UpdateGame();

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
