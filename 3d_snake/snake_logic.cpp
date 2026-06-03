#include "snake_logic.h"
#include "constants.h"
#include "food_rule.h"
#include "score_file.h"
#include "settings.h"
#include <cstdlib>

// 贪吃蛇规则：移动、吃食物、增长、碰撞、生成食物。

// 初始化或重新开始一局。
void InitGame(GameContext &context)
{
    context.game.snake.clear();

    // 蛇出生。
    Cell start;
    start.x = context.settings.gridSize / 2;
    start.z = context.settings.gridSize / 2;

    context.game.snake.push_back(start); // 蛇头。
    context.game.snake.push_back({start.x - 1, start.z});
    context.game.snake.push_back({start.x - 2, start.z});

    // 初始方向向右。
    context.game.direction = {1, 0}; // x 增加，向右。
    context.game.nextDirection = context.game.direction;

    // 重置一局游戏的基本状态。
    context.game.food = {-1, -1}; // 暂无食物。
    context.game.status = Playing;
    context.game.score = 0;
    context.game.moveTimer = 0.0f;
    context.game.started = false;
    context.game.isSuperFood = false;
    context.game.growLeft = 0; // 剩余增长次数。

    // 重置临时视觉效果。
    context.effects.eatTimer = 0.0f;
    context.effects.scoreFlashTimer = 0.0f;
    context.effects.lastFoodCell = {-1, -1};

    // 疯狂模式数据也重置，保证每局干净。
    context.crazy.blocks.clear();
    context.crazy.eventTimer = 3.0f; // 首次事件延迟。
    context.crazy.boostTimer = 0.0f;
    context.crazy.redFlashTimer = 0.0f;
    context.crazy.greenFlashTimer = 0.0f;
    context.crazy.boostWaiting = false;

    SpawnFood(context); // 避免食物刷在蛇身上。
}

// 使用菜单设置开始游戏。
void StartSelectedGame(GameContext &context)
{
    SetLevel(context, context.settings.currentLevel);
    SetSpeedByLevel(context, context.settings.speedLevel);
    UpdateTargetScore(context);
    InitGame(context);
}

// 每帧更新蛇的移动。
void UpdateGame(GameContext &context)
{
    bool gameIsPlaying = context.game.status == Playing;

    bool snakeHasStarted = context.game.started; // 未按方向键前不移动。

    if (!gameIsPlaying || !snakeHasStarted)
    {
        return;
    }

    context.game.moveTimer += GetFrameTime();

    // moveTimer 累计到移动间隔后，蛇才走一格。
    if (context.game.moveTimer >= GetCurrentMoveInterval(context))
    {
        context.game.moveTimer = 0.0f;
        MoveSnake(context);
    }
}

// 更新临时视觉效果。
void UpdateEffects(GameContext &context)
{
    float deltaTime = GetFrameTime();

    if (context.effects.eatTimer > 0.0f)
    {
        context.effects.eatTimer -= deltaTime;

        if (context.effects.eatTimer < 0.0f)
        {
            context.effects.eatTimer = 0.0f; // 防止负数。
        }
    }

    if (context.effects.scoreFlashTimer > 0.0f)
    {
        context.effects.scoreFlashTimer -= deltaTime;

        if (context.effects.scoreFlashTimer < 0.0f)
        {
            context.effects.scoreFlashTimer = 0.0f; // 防止负数。
        }
    }
}

// 判断两个格子是否相同。
bool IsSameCell(Cell a, Cell b)
{
    return a.x == b.x && a.z == b.z;
}

// 判断格子是否在蛇身上。
bool IsCellOnSnake(const GameContext &context, Cell cell)
{
    for (Cell snakeCell : context.game.snake)
    {
        if (IsSameCell(snakeCell, cell))
        {
            return true;
        }
    }

    return false;
}

// 判断格子是否有蓝色方块。
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

// 判断格子是否有实体蓝色障碍。
bool IsCellOnSolidCrazyBlock(const GameContext &context, Cell cell)
{
    // 只有 solid 为 true 的蓝方块才会撞死蛇。
    for (int i = 0; i < (int)context.crazy.blocks.size(); i++)
    {
        bool blockIsSolid = context.crazy.blocks[i].solid; // 实体障碍。
        bool sameCell = IsSameCell(context.crazy.blocks[i].cell, cell);

        if (blockIsSolid && sameCell)
        {
            return true;
        }
    }

    return false;
}

// 判断蛇头是否越界。
bool CollisionChecker::IsWallHit(const GameContext &context, Cell head) const
{
    bool hitLeftWall = head.x < 0;
    bool hitRightWall = head.x >= context.settings.gridSize;
    bool hitTopWall = head.z < 0;
    bool hitBottomWall = head.z >= context.settings.gridSize;

    return hitLeftWall || hitRightWall || hitTopWall || hitBottomWall;
}

// 判断蛇头是否撞到自己。
bool CollisionChecker::IsSelfHit(const GameContext &context, Cell head, bool willGrow) const
{
    int checkEnd = (int)context.game.snake.size(); // 检查到第几节。

    // 没吃到食物时，尾巴本回合会离开。
    // 所以可以不检查最后一节尾巴。
    if (!willGrow)
    {
        checkEnd--;
    }

    int index = 0;

    for (Cell snakeCell : context.game.snake)
    {
        bool isHeadCell = index == 0;
        bool reachedCheckEnd = index >= checkEnd;

        if (reachedCheckEnd)
        {
            break;
        }

        if (!isHeadCell && IsSameCell(snakeCell, head))
        {
            return true;
        }

        index++;
    }

    return false;
}

// 判断当前是否允许穿墙。
bool CollisionChecker::CanPassWall(const GameContext &context) const
{
    // 只有疯狂模式 + 加速中，才允许从墙的一边穿到另一边。
    bool crazyModeIsOn = context.settings.crazyMode;
    bool boostIsActive = context.crazy.boostTimer > 0.0f; // 正在加速。

    return crazyModeIsOn && boostIsActive;
}

// 判断本次移动是否失败。
bool CollisionChecker::IsGameOver(GameContext &context, Cell head, bool willGrow) const
{
    bool snakeCanPassWall = CanPassWall(context);
    bool snakeHitWall = IsWallHit(context, head);
    bool snakeHitSelf = IsSelfHit(context, head, willGrow);
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

// 计算下一格蛇头。
Cell SnakeMover::GetNextHead(const GameContext &context) const
{
    Cell head = context.game.snake.front(); // 当前蛇头。

    head.x += context.game.nextDirection.x;
    head.z += context.game.nextDirection.z;

    return head;
}

// 穿墙时把蛇头移到另一边。
void SnakeMover::WrapHeadIfNeeded(GameContext &context, Cell &head) const
{
    CollisionChecker checker;

    if (!checker.CanPassWall(context))
    {
        return;
    }

    // 超出哪边，就从相反边回来。
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

// 根据 growLeft 决定蛇是否增长。
void SnakeMover::UpdateLength(GameContext &context) const
{
    // growLeft > 0 表示本回合不删除尾巴。
    // 蛇头已经加了新格子，不删除尾巴就等于变长。
    if (context.game.growLeft > 0)
    {
        context.game.growLeft--;
    }
    else
    {
        context.game.snake.pop_back();
    }
}

// 让蛇移动一格。
void SnakeMover::MoveOneStep(GameContext &context) const
{
    CollisionChecker checker;

    context.game.direction = context.game.nextDirection; // 正式采用输入方向。

    // 先算出新蛇头，再根据疯狂模式决定是否穿墙。
    Cell newHead = GetNextHead(context);
    WrapHeadIfNeeded(context, newHead);

    // willGrow 会影响撞自己检测。
    // 不增长时，尾巴会移开；增长时，尾巴不会移开。
    bool willGrow = IsSameCell(newHead, context.game.food); // 吃到食物就会增长。
    bool gameOver = checker.IsGameOver(context, newHead, willGrow);

    if (gameOver)
    {
        context.game.status = GameOver;
        return;
    }

    context.game.snake.insert(context.game.snake.begin(), newHead); // 头部插入。

    EatFoodIfNeeded(context, newHead);
    UpdateLength(context);
}

// 判断蛇头是否越界。
bool CheckWallCollision(const GameContext &context, Cell head)
{
    CollisionChecker checker;
    return checker.IsWallHit(context, head);
}

// 判断蛇头是否撞到自己。
bool CheckSelfCollision(const GameContext &context, Cell head, bool willGrow)
{
    CollisionChecker checker;
    return checker.IsSelfHit(context, head, willGrow);
}

// 判断当前是否允许穿墙。
bool CanPassWall(const GameContext &context)
{
    CollisionChecker checker;
    return checker.CanPassWall(context);
}

// 计算下一格蛇头。
Cell GetNextHead(const GameContext &context)
{
    SnakeMover mover;
    return mover.GetNextHead(context);
}

// 穿墙时把蛇头移到另一边。
void WrapHeadIfNeeded(GameContext &context, Cell &head)
{
    SnakeMover mover;
    mover.WrapHeadIfNeeded(context, head);
}

// 判断本次移动是否失败。
bool CheckGameOver(GameContext &context, Cell head, bool willGrow)
{
    CollisionChecker checker;
    return checker.IsGameOver(context, head, willGrow);
}

// 吃到食物后处理加分、增长和新食物。
void EatFoodIfNeeded(GameContext &context, Cell head)
{
    bool ateFood = IsSameCell(head, context.game.food);

    if (!ateFood)
    {
        return;
    }

    // 记录食物位置，用来画爆炸效果。
    context.effects.lastFoodCell = context.game.food;
    context.effects.eatTimer = EAT_EFFECT_TIME;
    context.effects.scoreFlashTimer = SCORE_FLASH_TIME;

    // 食物分数和增长节数由 FoodRule 类体系决定。
    // 父类引用调用虚函数，自动执行对应子类版本。
    const FoodRule &foodRule = GetFoodRule(context.game.isSuperFood);

    context.game.score += foodRule.GetScore();
    context.game.growLeft += foodRule.GetGrowCount();

    UpdateHighScore(context);

    // 普通模式达到目标分数就胜利。
    // 疯狂模式没有目标分数，所以不会按分数胜利。
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

// 根据 growLeft 决定蛇是否增长。
void UpdateSnakeLength(GameContext &context)
{
    SnakeMover mover;
    mover.UpdateLength(context);
}

// 让蛇移动一格。
void MoveSnake(GameContext &context)
{
    SnakeMover mover;
    mover.MoveOneStep(context);
}

// 判断食物能不能放在这个格子。
bool FoodSpawner::IsFoodCellSafe(const GameContext &context, Cell cell) const
{
    bool cellOnSnake = IsCellOnSnake(context, cell);
    bool cellOnBlock = IsCellOnCrazyBlock(context, cell);

    return !cellOnSnake && !cellOnBlock;
}

// 在安全空格子里生成食物。
void FoodSpawner::Spawn(GameContext &context) const
{
    // 先收集所有可以放食物的空格子。
    // 这样随机时不会生成到蛇身或蓝色方块上。
    std::vector<Cell> emptyCells;

    for (int x = 0; x < context.settings.gridSize; x++)
    {
        for (int z = 0; z < context.settings.gridSize; z++)
        {
            Cell cell = {x, z}; // 候选格子。

            if (IsFoodCellSafe(context, cell))
            {
                emptyCells.push_back(cell);
            }
        }
    }

    if (emptyCells.empty())
    {
        // 没有空格子，地图被占满，玩家胜利。
        context.game.food = {-1, -1};
        context.game.status = Win;
        UpdateHighScore(context);
        return;
    }

    int index = rand() % emptyCells.size();

    context.game.food = emptyCells[index];

    // 金色超级果子只在疯狂模式出现。
    bool canSpawnSuperFood = context.settings.crazyMode;
    bool randomHitSuperFoodChance = rand() % 100 < SUPER_FOOD_CHANCE;

    context.game.isSuperFood =
        canSpawnSuperFood &&
        randomHitSuperFoodChance;
}

// 在安全空格子里生成食物。
void SpawnFood(GameContext &context)
{
    FoodSpawner spawner;
    spawner.Spawn(context);
}

// 计算当前移动间隔。
float GetCurrentMoveInterval(const GameContext &context)
{
    int scoreSpeedLevel = context.game.score / SPEED_UP_SCORE; // 分数速度等级。

    float interval =
        context.settings.speed.startInterval -
        scoreSpeedLevel * context.settings.speed.speedUpAmount;

    if (interval < context.settings.speed.minInterval)
    {
        interval = context.settings.speed.minInterval;
    }

    if (context.crazy.boostTimer > 0.0f)
    {
        interval = interval / BOOST_SPEED_RATE; // 疯狂模式加速。
    }

    return interval;
}
