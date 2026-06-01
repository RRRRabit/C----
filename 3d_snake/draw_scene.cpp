#include "draw_scene.h"
#include "constants.h"
#include "snake_logic.h"

void UpdateCameraPosition(GameContext &context)
{
    context.camera.position = (Vector3){0.0f, 22.0f, 24.0f};
    context.camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    context.camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    context.camera.fovy = 45.0f;
    context.camera.projection = CAMERA_PERSPECTIVE;
}

Vector3 CellToWorld(const GameContext &context, Cell cell)
{
    float worldX =
        (cell.x - context.settings.gridSize / 2.0f + 0.5f) *
        CUBE_SIZE;

    float worldZ =
        (cell.z - context.settings.gridSize / 2.0f + 0.5f) *
        CUBE_SIZE;

    return (Vector3){worldX, 0.5f, worldZ};
}

Color GetSnakeColor(const GameContext &context, Color normalColor)
{
    bool shouldFlash = (int)(GetTime() * 10) % 2 == 0;

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

void DrawGame3D(const GameContext &context)
{
    DrawGridGround(context);
    DrawWalls(context);
    DrawCrazyBlocks(context);
    DrawFood(context);
    DrawEatEffect(context);
    DrawSnake(context);
}

void DrawGridGround(const GameContext &context)
{
    for (int x = 0; x < context.settings.gridSize; x++)
    {
        for (int z = 0; z < context.settings.gridSize; z++)
        {
            Vector3 pos = CellToWorld(context, {x, z});
            pos.y = -0.06f;

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

void DrawWalls(const GameContext &context)
{
    float half = context.settings.gridSize * CUBE_SIZE / 2.0f;
    float edge = half + 0.5f;
    float y = 0.5f;

    Color wallColor = (Color){110, 118, 128, 230};

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

void DrawSnake(const GameContext &context)
{
    for (int i = 0; i < (int)context.game.snake.size(); i++)
    {
        Vector3 pos = CellToWorld(context, context.game.snake[i]);

        if (i == 0)
        {
            DrawSnakeHead(context, pos);
        }
        else
        {
            DrawSnakeBody(context, pos, i, (int)context.game.snake.size());
        }
    }
}

void DrawSnakeHead(const GameContext &context, Vector3 pos)
{
    Color headColor = GetSnakeColor(context, (Color){28, 150, 75, 255});

    DrawCube(pos, CUBE_SIZE * 0.92f, CUBE_SIZE * 0.92f, CUBE_SIZE * 0.92f, headColor);
    DrawCubeWires(pos, CUBE_SIZE * 0.92f, CUBE_SIZE * 0.92f, CUBE_SIZE * 0.92f, BLACK);

    float eyeOffsetSide = 0.18f;
    float eyeOffsetFront = 0.46f;
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

void DrawSnakeBody(const GameContext &context, Vector3 pos, int index, int totalLength)
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
    Color bodyColor = GetSnakeColor(context, normalColor);

    DrawCube(pos, size, size, size, bodyColor);
    DrawCubeWires(pos, size, size, size, (Color){10, 40, 20, 255});
}

void DrawFood(const GameContext &context)
{
    if (context.game.food.x < 0 || context.game.food.z < 0)
    {
        return;
    }

    Vector3 pos = CellToWorld(context, context.game.food);

    if (context.game.isSuperFood)
    {
        DrawSphere(pos, CUBE_SIZE * 0.52f, GOLD);
        DrawSphereWires(pos, CUBE_SIZE * 0.64f, 10, 10, ORANGE);
    }
    else
    {
        DrawSphere(pos, CUBE_SIZE * 0.40f, RED);
    }
}

void DrawEatEffect(const GameContext &context)
{
    bool noEffect = context.effects.eatTimer <= 0.0f;
    bool noLastFood = context.effects.lastFoodCell.x < 0 || context.effects.lastFoodCell.z < 0;

    if (noEffect || noLastFood)
    {
        return;
    }

    float progress = 1.0f - context.effects.eatTimer / EAT_EFFECT_TIME;
    Vector3 pos = CellToWorld(context, context.effects.lastFoodCell);
    float radius = CUBE_SIZE * (0.4f + progress * 1.2f);
    unsigned char alpha = (unsigned char)(180 * (1.0f - progress));

    DrawSphereWires(pos, radius, 12, 12, (Color){255, 210, 80, alpha});
}

void DrawCrazyBlocks(const GameContext &context)
{
    for (int i = 0; i < (int)context.crazy.blocks.size(); i++)
    {
        Vector3 pos = CellToWorld(context, context.crazy.blocks[i].cell);

        if (!context.crazy.blocks[i].solid)
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
