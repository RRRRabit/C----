#include "draw_scene.h"
#include "constants.h"
#include "food_rule.h"
#include "snake_logic.h"

// 3D 场景绘制，只读数据，不改规则。

// 设置 3D 摄像机。
void UpdateCameraPosition(GameContext &context)
{
    // 固定摄像机：从斜上方看整个地图。
    context.camera.position = (Vector3){0.0f, 22.0f, 24.0f};
    context.camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    context.camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    context.camera.fovy = 45.0f;
    context.camera.projection = CAMERA_PERSPECTIVE;
}

// 格子坐标转 3D 世界坐标。
Vector3 CellToWorld(const GameContext &context, Cell cell)
{
    // 游戏逻辑使用整数格子坐标 Cell。
    // raylib 画 3D 物体需要世界坐标 Vector3。
    float worldX =
        (cell.x - context.settings.gridSize / 2.0f + 0.5f) *
        CUBE_SIZE;

    float worldZ =
        (cell.z - context.settings.gridSize / 2.0f + 0.5f) *
        CUBE_SIZE;

    return (Vector3){worldX, 0.5f, worldZ}; // y=0.5 贴近地面。
}

// 根据疯狂模式状态决定蛇颜色。
Color GetSnakeColor(const GameContext &context, Color normalColor)
{
    // 疯狂模式会用颜色提示状态：
    // 红色表示预警或加速，绿色表示恢复。
    bool shouldFlash = (int)(GetTime() * 10) % 2 == 0; // 简单闪烁。

    if (context.crazy.boostTimer > 0.0f)
    {
        return RED;
    }

    if (context.crazy.redFlashTimer > 0.0f && shouldFlash)
    {
        return RED;
    }

    if (context.crazy.greenFlashTimer > 0.0f && shouldFlash)
    {
        return LIME;
    }

    return normalColor;
}

// 绘制整个 3D 游戏场景。
void DrawGame3D(const GameContext &context)
{
    // 3D 绘制顺序：先画地面和墙，再画物体，最后画蛇。
    DrawGridGround(context);
    DrawWalls(context);
    DrawCrazyBlocks(context);
    DrawFood(context);
    DrawEatEffect(context);
    DrawSnake(context);
}

// 绘制棋盘格地面。
void DrawGridGround(const GameContext &context)
{
    // 棋盘格地面让玩家更容易看清蛇在哪个格子。
    for (int x = 0; x < context.settings.gridSize; x++)
    {
        for (int z = 0; z < context.settings.gridSize; z++)
        {
            Vector3 pos = CellToWorld(context, {x, z});
            pos.y = -0.06f; // 避免和蛇方块重叠闪烁。

            Color color;

            if ((x + z) % 2 == 0)
            {
                color = (Color){46, 54, 64, 255};
            }
            else
            {
                color = (Color){38, 45, 55, 255};
            }

            DrawCube(pos, CUBE_SIZE, 0.08f, CUBE_SIZE, color);

            DrawCubeWires(
                (Vector3){pos.x, -0.015f, pos.z},
                CUBE_SIZE,
                0.01f,
                CUBE_SIZE,
                (Color){70, 80, 92, 120});
        }
    }
}

// 绘制边界墙。
void DrawWalls(const GameContext &context)
{
    float half = context.settings.gridSize * CUBE_SIZE / 2.0f;
    float edge = half + 0.5f; // 地图外侧半格。
    float y = 0.5f;

    Color wallColor = (Color){110, 118, 128, 230};

    // 疯狂模式加速时可以穿墙，所以墙变成淡红色作为提示。
    if (CanPassWall(context))
    {
        wallColor = (Color){220, 90, 90, 160};
    }

    for (int i = 0; i < context.settings.gridSize; i++)
    {
        float offset =
            (i - context.settings.gridSize / 2.0f + 0.5f) *
            CUBE_SIZE;

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

// 绘制整条蛇。
void DrawSnake(const GameContext &context)
{
    // snake.front() 是蛇头，其余都是身体。
    int index = 0;
    int totalLength = (int)context.game.snake.size();

    for (Cell snakeCell : context.game.snake)
    {
        Vector3 pos = CellToWorld(context, snakeCell);

        if (index == 0)
        {
            DrawSnakeHead(context, pos);
        }
        else
        {
            DrawSnakeBody(context, pos, index, totalLength);
        }

        index++;
    }
}

// 绘制蛇头。
void DrawSnakeHead(const GameContext &context, Vector3 pos)
{
    Color headColor = GetSnakeColor(context, (Color){28, 150, 75, 255});

    // 加速时给蛇头画红色光圈，强调疯狂模式状态。
    if (context.crazy.boostTimer > 0.0f)
    {
        DrawSphereWires(pos, CUBE_SIZE * 0.72f, 10, 10, (Color){255, 80, 60, 180});
        DrawSphereWires(pos, CUBE_SIZE * 0.95f, 10, 10, (Color){255, 40, 40, 90});
    }

    DrawCube(pos, CUBE_SIZE * 0.92f, CUBE_SIZE * 0.92f, CUBE_SIZE * 0.92f, headColor);
    DrawCubeWires(pos, CUBE_SIZE * 0.92f, CUBE_SIZE * 0.92f, CUBE_SIZE * 0.92f, BLACK);

    // 眼睛根据移动方向摆放，让玩家更容易判断蛇头朝向。
    float eyeOffsetSide = 0.18f;  // 眼睛左右间距。
    float eyeOffsetFront = 0.46f; // 眼睛前移距离。
    float eyeY = pos.y + 0.18f;

    Vector3 leftEye = pos;
    Vector3 rightEye = pos;

    if (context.game.direction.x != 0)
    {
        leftEye.x += context.game.direction.x * eyeOffsetFront;
        rightEye.x += context.game.direction.x * eyeOffsetFront;
        leftEye.z -= eyeOffsetSide;
        rightEye.z += eyeOffsetSide;
    }
    else
    {
        leftEye.z += context.game.direction.z * eyeOffsetFront;
        rightEye.z += context.game.direction.z * eyeOffsetFront;
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

// 绘制一节蛇身。
void DrawSnakeBody(const GameContext &context, Vector3 pos, int index, int totalLength)
{
    // 身体越靠近尾巴越小一点，让蛇看起来有层次。
    float size = CUBE_SIZE * 0.84f;

    if (totalLength > 1)
    {
        float tailProgress = (float)index / (float)(totalLength - 1); // 0 头，1 尾。
        size = CUBE_SIZE * (0.86f - tailProgress * 0.14f);
    }

    int green = 180 - index * 3; // 越靠后越暗。

    if (green < 95)
    {
        green = 95;
    }

    Color normalColor = (Color){55, (unsigned char)green, 85, 255};
    Color bodyColor = GetSnakeColor(context, normalColor);

    // 外层线框只负责视觉效果，不影响碰撞。
    DrawCubeWires(pos, size * 1.18f, size * 1.18f, size * 1.18f, (Color){90, 255, 140, 120});
    DrawCube(pos, size, size, size, bodyColor);
    DrawCubeWires(pos, size, size, size, (Color){10, 40, 20, 255});
}

// 绘制食物。
void DrawFood(const GameContext &context)
{
    // food 为 -1 表示当前没有食物，比如已经胜利。
    if (context.game.food.x < 0 || context.game.food.z < 0)
    {
        return;
    }

    Vector3 pos = CellToWorld(context, context.game.food);

    // 多态：普通果子和金色果子各自绘制。
    const FoodRule &foodRule = GetFoodRule(context.game.isSuperFood);

    foodRule.Draw(pos);
}

// 绘制吃食物爆炸效果。
void DrawEatEffect(const GameContext &context)
{
    // 吃到食物后，短时间显示两圈扩散线框。
    // 普通果子和超级果子使用同一种爆炸效果，保持逻辑简单。
    bool noEffect = context.effects.eatTimer <= 0.0f;
    bool noLastFood = context.effects.lastFoodCell.x < 0 || context.effects.lastFoodCell.z < 0;

    if (noEffect || noLastFood)
    {
        return;
    }

    // progress 从 0 逐渐变到 1。
    // 半径越来越大，透明度越来越低，看起来就是扩散。
    float progress = 1.0f - context.effects.eatTimer / EAT_EFFECT_TIME; // 0 开始，1 结束。
    Vector3 pos = CellToWorld(context, context.effects.lastFoodCell);
    float smallRadius = CUBE_SIZE * (0.4f + progress * 1.2f);
    float bigRadius = CUBE_SIZE * (0.7f + progress * 2.0f);
    unsigned char strongAlpha = (unsigned char)(220 * (1.0f - progress));
    unsigned char weakAlpha = (unsigned char)(120 * (1.0f - progress));

    DrawSphereWires(pos, smallRadius, 12, 12, (Color){255, 220, 70, strongAlpha});
    DrawSphereWires(pos, bigRadius, 12, 12, (Color){255, 90, 40, weakAlpha});
}

// 绘制疯狂模式蓝方块。
void DrawCrazyBlocks(const GameContext &context)
{
    // 蓝色方块分两种状态：
    // 1. solid == false：闪烁预警
    // 2. solid == true：实体障碍
    for (int i = 0; i < (int)context.crazy.blocks.size(); i++)
    {
        Vector3 pos = CellToWorld(context, context.crazy.blocks[i].cell);

        if (!context.crazy.blocks[i].solid)
        {
            bool showWarning = (int)(GetTime() * 8) % 2 == 0; // 预警闪烁。

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
