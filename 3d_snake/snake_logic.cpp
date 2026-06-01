#include "snake_logic.h"
#include "constants.h"
#include "score_file.h"
#include "settings.h"
#include <cstdlib>

void InitGame(GameContext &context)
{
    context.game.snake.clear();

    Cell start;
    start.x = context.settings.gridSize / 2;
    start.z = context.settings.gridSize / 2;

    context.game.snake.push_back(start);
    context.game.snake.push_back({start.x - 1, start.z});
    context.game.snake.push_back({start.x - 2, start.z});

    context.game.direction = {1, 0};
    context.game.nextDirection = context.game.direction;

    context.game.food = {-1, -1};
    context.game.status = Playing;
    context.game.score = 0;
    context.game.moveTimer = 0.0f;
    context.game.started = false;
    context.game.isSuperFood = false;
    context.game.growLeft = 0;

    context.effects.eatTimer = 0.0f;
    context.effects.scoreFlashTimer = 0.0f;
    context.effects.lastFoodCell = {-1, -1};

    context.crazy.blocks.clear();
    context.crazy.eventTimer = 3.0f;
    context.crazy.boostTimer = 0.0f;
    context.crazy.redFlashTimer = 0.0f;
    context.crazy.greenFlashTimer = 0.0f;
    context.crazy.boostWaiting = false;

    SpawnFood(context);
}

void StartSelectedGame(GameContext &context)
{
    SetLevel(context, context.settings.currentLevel);
    SetSpeedByLevel(context, context.settings.speedLevel);
    UpdateTargetScore(context);
    InitGame(context);
}

void UpdateGame(GameContext &context)
{
    bool gameIsPlaying = context.game.status == Playing;
    bool snakeHasStarted = context.game.started;

    if (!gameIsPlaying || !snakeHasStarted)
    {
        return;
    }

    context.game.moveTimer += GetFrameTime();

    if (context.game.moveTimer >= GetCurrentMoveInterval(context))
    {
        context.game.moveTimer = 0.0f;
        MoveSnake(context);
    }
}

void UpdateEffects(GameContext &context)
{
    float deltaTime = GetFrameTime();

    if (context.effects.eatTimer > 0.0f)
    {
        context.effects.eatTimer -= deltaTime;

        if (context.effects.eatTimer < 0.0f)
        {
            context.effects.eatTimer = 0.0f;
        }
    }

    if (context.effects.scoreFlashTimer > 0.0f)
    {
        context.effects.scoreFlashTimer -= deltaTime;

        if (context.effects.scoreFlashTimer < 0.0f)
        {
            context.effects.scoreFlashTimer = 0.0f;
        }
    }
}

bool IsSameCell(Cell a, Cell b)
{
    return a.x == b.x && a.z == b.z;
}

bool IsCellOnSnake(const GameContext &context, Cell cell)
{
    for (int i = 0; i < (int)context.game.snake.size(); i++)
    {
        if (IsSameCell(context.game.snake[i], cell))
        {
            return true;
        }
    }

    return false;
}

bool IsCellOnCrazyBlock(const GameContext &context, Cell cell)
{
    for (int i = 0; i < (int)context.crazy.blocks.size(); i++)
    {
        if (IsSameCell(context.crazy.blocks[i].cell, cell))
        {
            return true;
        }
    }

    return false;
}

bool IsCellOnSolidCrazyBlock(const GameContext &context, Cell cell)
{
    for (int i = 0; i < (int)context.crazy.blocks.size(); i++)
    {
        bool blockIsSolid = context.crazy.blocks[i].solid;
        bool sameCell = IsSameCell(context.crazy.blocks[i].cell, cell);

        if (blockIsSolid && sameCell)
        {
            return true;
        }
    }

    return false;
}

bool CheckWallCollision(const GameContext &context, Cell head)
{
    bool hitLeftWall = head.x < 0;
    bool hitRightWall = head.x >= context.settings.gridSize;
    bool hitTopWall = head.z < 0;
    bool hitBottomWall = head.z >= context.settings.gridSize;

    return hitLeftWall || hitRightWall || hitTopWall || hitBottomWall;
}

bool CheckSelfCollision(const GameContext &context, Cell head, bool willGrow)
{
    int checkEnd = (int)context.game.snake.size();

    // 没吃到食物时，尾巴本回合会离开。
    // 所以可以不检查最后一节尾巴。
    if (!willGrow)
    {
        checkEnd--;
    }

    for (int i = 1; i < checkEnd; i++)
    {
        if (IsSameCell(context.game.snake[i], head))
        {
            return true;
        }
    }

    return false;
}

bool CanPassWall(const GameContext &context)
{
    bool crazyModeIsOn = context.settings.crazyMode;
    bool boostIsActive = context.crazy.boostTimer > 0.0f;

    return crazyModeIsOn && boostIsActive;
}

Cell GetNextHead(const GameContext &context)
{
    Cell head = context.game.snake[0];

    head.x += context.game.nextDirection.x;
    head.z += context.game.nextDirection.z;

    return head;
}

void WrapHeadIfNeeded(GameContext &context, Cell &head)
{
    if (!CanPassWall(context))
    {
        return;
    }

    if (head.x < 0)
    {
        head.x = context.settings.gridSize - 1;
    }

    if (head.x >= context.settings.gridSize)
    {
        head.x = 0;
    }

    if (head.z < 0)
    {
        head.z = context.settings.gridSize - 1;
    }

    if (head.z >= context.settings.gridSize)
    {
        head.z = 0;
    }
}

bool CheckGameOver(GameContext &context, Cell head, bool willGrow)
{
    bool snakeCanPassWall = CanPassWall(context);
    bool snakeHitWall = CheckWallCollision(context, head);
    bool snakeHitSelf = CheckSelfCollision(context, head, willGrow);
    bool snakeHitBlueBlock = IsCellOnSolidCrazyBlock(context, head);

    if (!snakeCanPassWall && snakeHitWall)
    {
        return true;
    }

    if (snakeHitSelf)
    {
        return true;
    }

    if (snakeHitBlueBlock)
    {
        return true;
    }

    return false;
}

void EatFoodIfNeeded(GameContext &context, Cell head)
{
    bool ateFood = IsSameCell(head, context.game.food);

    if (!ateFood)
    {
        return;
    }

    context.effects.lastFoodCell = context.game.food;
    context.effects.eatTimer = EAT_EFFECT_TIME;
    context.effects.scoreFlashTimer = SCORE_FLASH_TIME;

    if (context.game.isSuperFood)
    {
        context.game.score += SCORE_PER_SUPER_FOOD;
        context.game.growLeft += SUPER_FOOD_GROW;
    }
    else
    {
        context.game.score += SCORE_PER_NORMAL_FOOD;
        context.game.growLeft += NORMAL_FOOD_GROW;
    }

    UpdateHighScore(context);

    bool normalMode = !context.settings.crazyMode;
    bool reachedTargetScore = context.game.score >= context.settings.targetScore;

    if (normalMode && reachedTargetScore)
    {
        context.game.status = Win;
        context.game.food = {-1, -1};
        return;
    }

    SpawnFood(context);
}

void UpdateSnakeLength(GameContext &context)
{
    if (context.game.growLeft > 0)
    {
        context.game.growLeft--;
    }
    else
    {
        context.game.snake.pop_back();
    }
}

void MoveSnake(GameContext &context)
{
    context.game.direction = context.game.nextDirection;

    Cell newHead = GetNextHead(context);
    WrapHeadIfNeeded(context, newHead);

    bool willGrow = IsSameCell(newHead, context.game.food);
    bool gameOver = CheckGameOver(context, newHead, willGrow);

    if (gameOver)
    {
        context.game.status = GameOver;
        return;
    }

    context.game.snake.insert(context.game.snake.begin(), newHead);

    EatFoodIfNeeded(context, newHead);
    UpdateSnakeLength(context);
}

void SpawnFood(GameContext &context)
{
    std::vector<Cell> emptyCells;

    for (int x = 0; x < context.settings.gridSize; x++)
    {
        for (int z = 0; z < context.settings.gridSize; z++)
        {
            Cell cell = {x, z};

            bool cellOnSnake = IsCellOnSnake(context, cell);
            bool cellOnBlock = IsCellOnCrazyBlock(context, cell);

            if (!cellOnSnake && !cellOnBlock)
            {
                emptyCells.push_back(cell);
            }
        }
    }

    if (emptyCells.empty())
    {
        context.game.food = {-1, -1};
        context.game.status = Win;
        UpdateHighScore(context);
        return;
    }

    int index = rand() % emptyCells.size();

    context.game.food = emptyCells[index];
    context.game.isSuperFood = rand() % 100 < SUPER_FOOD_CHANCE;
}

float GetCurrentMoveInterval(const GameContext &context)
{
    int scoreSpeedLevel = context.game.score / SPEED_UP_SCORE;

    float interval =
        context.settings.speed.startInterval -
        scoreSpeedLevel * context.settings.speed.speedUpAmount;

    if (interval < context.settings.speed.minInterval)
    {
        interval = context.settings.speed.minInterval;
    }

    if (context.crazy.boostTimer > 0.0f)
    {
        interval = interval / BOOST_SPEED_RATE;
    }

    return interval;
}
