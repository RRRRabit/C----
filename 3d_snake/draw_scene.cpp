#include "draw_scene.h"
#include "constants.h"
#include "food_rule.h"
#include "snake_logic.h"

// draw_scene.cpp 只负责 3D 场景绘制。
// 这里的函数只读取 GameContext，不改变游戏规则。
// 如果想改玩法，应该去 snake_logic.cpp 或 crazy_mode.cpp。

// 函数作用：设置 3D 摄像机的位置、朝向和视野。
void UpdateCameraPosition(GameContext &context)
{
    // 固定摄像机：从斜上方看整个地图。
    // 不做旋转和缩放，代码更容易理解。
    context.camera.position = (Vector3){0.0f, 22.0f, 24.0f}; // 摄像机位置：x 居中，y 在上方，z 在前方。
    context.camera.target = (Vector3){0.0f, 0.0f, 0.0f}; // 摄像机看向地图中心。
    context.camera.up = (Vector3){0.0f, 1.0f, 0.0f}; // y 轴作为“上方”。
    context.camera.fovy = 45.0f; // 视野角度，数值越大看到的范围越广。
    context.camera.projection = CAMERA_PERSPECTIVE; // 透视摄像机，有近大远小效果。
}

// 函数作用：把整数格子坐标转换成 raylib 使用的 3D 世界坐标。
Vector3 CellToWorld(const GameContext &context, Cell cell)
{
    // 游戏逻辑使用整数格子坐标 Cell。
    // raylib 画 3D 物体需要世界坐标 Vector3。
    // 这个函数负责把“第几个格子”转换成“3D 世界中的位置”。
    float worldX =
        (cell.x - context.settings.gridSize / 2.0f + 0.5f) *
        CUBE_SIZE; // 乘方块大小，得到真正 3D 坐标。

    float worldZ =
        (cell.z - context.settings.gridSize / 2.0f + 0.5f) *
        CUBE_SIZE; // z 方向同样从格子坐标转换为世界坐标。

    return (Vector3){worldX, 0.5f, worldZ}; // y=0.5 让方块底部刚好贴近地面。
}

// 函数作用：根据疯狂模式状态决定蛇当前应该显示什么颜色。
Color GetSnakeColor(const GameContext &context, Color normalColor)
{
    // 疯狂模式会用颜色提示状态：
    // 红色表示预警或加速，绿色表示恢复。
    bool shouldFlash = (int)(GetTime() * 10) % 2 == 0; // 每秒约闪 5 次，代码简单。

    if (context.crazy.boostTimer > 0.0f)
    {
        return RED; // 加速中持续显示红色。
    }

    if (context.crazy.redFlashTimer > 0.0f && shouldFlash)
    {
        return RED; // 红光预警时，隔帧返回红色。
    }

    if (context.crazy.greenFlashTimer > 0.0f && shouldFlash)
    {
        return LIME; // 绿色恢复提示时，隔帧返回绿色。
    }

    return normalColor; // 没有特殊状态就用原本颜色。
}

// 函数作用：按顺序绘制整个 3D 游戏场景。
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

// 函数作用：绘制棋盘格地面。
void DrawGridGround(const GameContext &context)
{
    // 棋盘格地面让玩家更容易看清蛇在哪个格子。
    for (int x = 0; x < context.settings.gridSize; x++)
    {
        for (int z = 0; z < context.settings.gridSize; z++)
        {
            Vector3 pos = CellToWorld(context, {x, z}); // 当前地面格子的中心位置。
            pos.y = -0.06f; // 地面稍微低一点，避免和蛇方块重叠闪烁。

            Color color;

            if ((x + z) % 2 == 0)
            {
                color = (Color){46, 54, 64, 255}; // 偶数格颜色稍亮。
            }
            else
            {
                color = (Color){38, 45, 55, 255}; // 奇数格颜色稍暗。
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

// 函数作用：绘制地图四周的边界墙。
void DrawWalls(const GameContext &context)
{
    // 墙画在地图可行走区域外侧，用来提示边界。
    float half = context.settings.gridSize * CUBE_SIZE / 2.0f; // 地图宽度的一半。
    float edge = half + 0.5f; // 墙放在地图外侧半格位置。
    float y = 0.5f; // 墙中心高度。

    Color wallColor = (Color){110, 118, 128, 230}; // 普通状态墙颜色。

    // 疯狂模式加速时可以穿墙，所以墙变成淡红色作为提示。
    if (CanPassWall(context))
    {
        wallColor = (Color){220, 90, 90, 160}; // 可穿墙时变淡红，提示规则变化。
    }

    for (int i = 0; i < context.settings.gridSize; i++)
    {
        float offset =
            (i - context.settings.gridSize / 2.0f + 0.5f) *
            CUBE_SIZE; // 每次循环对应一段墙的位置。

        DrawCube((Vector3){offset, y, -edge}, CUBE_SIZE, CUBE_SIZE * 1.2f, CUBE_SIZE, wallColor);
        DrawCube((Vector3){offset, y, edge}, CUBE_SIZE, CUBE_SIZE * 1.2f, CUBE_SIZE, wallColor);
        DrawCube((Vector3){-edge, y, offset}, CUBE_SIZE, CUBE_SIZE * 1.2f, CUBE_SIZE, wallColor);
        DrawCube((Vector3){edge, y, offset}, CUBE_SIZE, CUBE_SIZE * 1.2f, CUBE_SIZE, wallColor);
    }

    // 四个角落用黄色方块标出来，方便判断地图范围。
    Color cornerColor = (Color){245, 190, 80, 255}; // 黄色角标更醒目。

    DrawCube((Vector3){-edge, 1.05f, -edge}, CUBE_SIZE * 1.2f, CUBE_SIZE * 1.6f, CUBE_SIZE * 1.2f, cornerColor);
    DrawCube((Vector3){edge, 1.05f, -edge}, CUBE_SIZE * 1.2f, CUBE_SIZE * 1.6f, CUBE_SIZE * 1.2f, cornerColor);
    DrawCube((Vector3){-edge, 1.05f, edge}, CUBE_SIZE * 1.2f, CUBE_SIZE * 1.6f, CUBE_SIZE * 1.2f, cornerColor);
    DrawCube((Vector3){edge, 1.05f, edge}, CUBE_SIZE * 1.2f, CUBE_SIZE * 1.6f, CUBE_SIZE * 1.2f, cornerColor);
}

// 函数作用：遍历蛇身并绘制整条蛇。
void DrawSnake(const GameContext &context)
{
    // snake.front() 是蛇头，其余都是身体。
    int index = 0;
    int totalLength = (int)context.game.snake.size();

    for (Cell snakeCell : context.game.snake)
    {
        Vector3 pos = CellToWorld(context, snakeCell); // 把蛇身格子转成 3D 坐标。

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

// 函数作用：绘制蛇头、眼睛和加速时的红色光圈。
void DrawSnakeHead(const GameContext &context, Vector3 pos)
{
    Color headColor = GetSnakeColor(context, (Color){28, 150, 75, 255}); // 正常蛇头是绿色，疯狂状态可能变色。

    // 加速时给蛇头画红色光圈，强调疯狂模式状态。
    if (context.crazy.boostTimer > 0.0f)
    {
        DrawSphereWires(pos, CUBE_SIZE * 0.72f, 10, 10, (Color){255, 80, 60, 180}); // 内圈红光。
        DrawSphereWires(pos, CUBE_SIZE * 0.95f, 10, 10, (Color){255, 40, 40, 90}); // 外圈红光，半透明更弱。
    }

    DrawCube(pos, CUBE_SIZE * 0.92f, CUBE_SIZE * 0.92f, CUBE_SIZE * 0.92f, headColor); // 0.92 让蛇头比格子略小。
    DrawCubeWires(pos, CUBE_SIZE * 0.92f, CUBE_SIZE * 0.92f, CUBE_SIZE * 0.92f, BLACK); // 黑色线框增强边界。

    // 眼睛根据移动方向摆放，让玩家更容易判断蛇头朝向。
    float eyeOffsetSide = 0.18f; // 两只眼睛左右分开的距离。
    float eyeOffsetFront = 0.46f; // 眼睛朝蛇头前方偏移的距离。
    float eyeY = pos.y + 0.18f; // 眼睛比蛇头中心略高。

    Vector3 leftEye = pos; // 先从蛇头中心复制，再根据方向偏移。
    Vector3 rightEye = pos;

    if (context.game.direction.x != 0)
    {
        leftEye.x += context.game.direction.x * eyeOffsetFront; // 左右移动时，眼睛沿 x 方向放到前面。
        rightEye.x += context.game.direction.x * eyeOffsetFront;
        leftEye.z -= eyeOffsetSide; // 两只眼睛沿 z 方向分开。
        rightEye.z += eyeOffsetSide;
    }
    else
    {
        leftEye.z += context.game.direction.z * eyeOffsetFront; // 上下移动时，眼睛沿 z 方向放到前面。
        rightEye.z += context.game.direction.z * eyeOffsetFront;
        leftEye.x -= eyeOffsetSide; // 两只眼睛沿 x 方向分开。
        rightEye.x += eyeOffsetSide;
    }

    leftEye.y = eyeY; // 最后统一设置眼睛高度。
    rightEye.y = eyeY;

    DrawSphere(leftEye, 0.08f, WHITE);
    DrawSphere(rightEye, 0.08f, WHITE);
    DrawSphere(leftEye, 0.035f, BLACK);
    DrawSphere(rightEye, 0.035f, BLACK);
}

// 函数作用：绘制一节蛇身，并根据位置调整大小和颜色。
void DrawSnakeBody(const GameContext &context, Vector3 pos, int index, int totalLength)
{
    // 身体越靠近尾巴越小一点，让蛇看起来有层次。
    float size = CUBE_SIZE * 0.84f; // 默认身体比一个格子略小。

    if (totalLength > 1)
    {
        float tailProgress = (float)index / (float)(totalLength - 1); // 0 靠近头，1 靠近尾。
        size = CUBE_SIZE * (0.86f - tailProgress * 0.14f); // 越靠后越小，但变化很轻。
    }

    // 越靠后的身体颜色稍暗一点。
    int green = 180 - index * 3; // 越靠后绿色值越低，看起来更暗。

    if (green < 95)
    {
        green = 95; // 最暗也保留一点绿色，避免尾巴发黑。
    }

    Color normalColor = (Color){55, (unsigned char)green, 85, 255}; // 当前身体节的正常颜色。
    Color bodyColor = GetSnakeColor(context, normalColor); // 疯狂模式可能覆盖成红/绿。

    // 外层线框是简单能量感，不影响碰撞体积。
    DrawCubeWires(pos, size * 1.18f, size * 1.18f, size * 1.18f, (Color){90, 255, 140, 120}); // 外层绿色线框，制造轻微能量感。
    DrawCube(pos, size, size, size, bodyColor); // 真正的蛇身体方块。
    DrawCubeWires(pos, size, size, size, (Color){10, 40, 20, 255}); // 内层深色边线。
}

// 函数作用：绘制普通食物或金色超级果子。
void DrawFood(const GameContext &context)
{
    // food 为 -1 表示当前没有食物，比如已经胜利。
    if (context.game.food.x < 0 || context.game.food.z < 0)
    {
        return;
    }

    Vector3 pos = CellToWorld(context, context.game.food); // 食物所在格子的 3D 坐标。

    // 食物怎么画由 FoodRule 类体系决定。
    // 这里用父类引用调用 Draw()，会自动执行普通果子或金色果子的绘制函数。
    const FoodRule &foodRule = GetFoodRule(context.game.isSuperFood);

    foodRule.Draw(pos);
}

// 函数作用：绘制吃到食物后的扩散爆炸效果。
void DrawEatEffect(const GameContext &context)
{
    // 吃到食物后，短时间显示两圈扩散线框。
    // 普通果子和超级果子使用同一种爆炸效果，保持逻辑简单。
    bool noEffect = context.effects.eatTimer <= 0.0f; // 没有倒计时就不画爆炸。
    bool noLastFood = context.effects.lastFoodCell.x < 0 || context.effects.lastFoodCell.z < 0; // 没有记录位置也不画。

    if (noEffect || noLastFood)
    {
        return;
    }

    // progress 从 0 逐渐变到 1。
    // 半径越来越大，透明度越来越低，看起来就是扩散。
    float progress = 1.0f - context.effects.eatTimer / EAT_EFFECT_TIME; // 0 表示刚开始，1 表示快结束。
    Vector3 pos = CellToWorld(context, context.effects.lastFoodCell); // 爆炸中心就是被吃掉的食物位置。
    float smallRadius = CUBE_SIZE * (0.4f + progress * 1.2f); // 小圈从 0.4 格扩散到 1.6 格。
    float bigRadius = CUBE_SIZE * (0.7f + progress * 2.0f); // 大圈从 0.7 格扩散到 2.7 格。
    unsigned char strongAlpha = (unsigned char)(220 * (1.0f - progress)); // 越接近结束越透明。
    unsigned char weakAlpha = (unsigned char)(120 * (1.0f - progress)); // 外圈本来就更淡。

    DrawSphereWires(pos, smallRadius, 12, 12, (Color){255, 220, 70, strongAlpha});
    DrawSphereWires(pos, bigRadius, 12, 12, (Color){255, 90, 40, weakAlpha});
}

// 函数作用：绘制疯狂模式里的蓝色预警方块和实体障碍。
void DrawCrazyBlocks(const GameContext &context)
{
    // 蓝色方块分两种状态：
    // 1. solid == false：闪烁预警
    // 2. solid == true：实体障碍
    for (int i = 0; i < (int)context.crazy.blocks.size(); i++)
    {
        Vector3 pos = CellToWorld(context, context.crazy.blocks[i].cell); // 蓝方块所在格子的 3D 坐标。

        if (!context.crazy.blocks[i].solid)
        {
            bool showWarning = (int)(GetTime() * 8) % 2 == 0; // 预警阶段闪烁显示。

            if (showWarning)
            {
                DrawCube(pos, CUBE_SIZE * 0.9f, CUBE_SIZE * 0.9f, CUBE_SIZE * 0.9f, SKYBLUE); // 预警方块略小。
                DrawCubeWires(pos, CUBE_SIZE, CUBE_SIZE, CUBE_SIZE, BLUE); // 外框提示这里即将危险。
            }
        }
        else
        {
            DrawCube(pos, CUBE_SIZE * 0.95f, CUBE_SIZE * 0.95f, CUBE_SIZE * 0.95f, BLUE); // 实体障碍方块。
            DrawCubeWires(pos, CUBE_SIZE * 0.95f, CUBE_SIZE * 0.95f, CUBE_SIZE * 0.95f, DARKBLUE); // 深蓝边线。
        }
    }
}
