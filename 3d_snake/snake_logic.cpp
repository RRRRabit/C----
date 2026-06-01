#include "snake_logic.h"
#include "constants.h"
#include "score_file.h"
#include "settings.h"
#include <cstdlib>

// snake_logic.cpp 是贪吃蛇“规则”的核心文件。
// 这里负责移动、吃食物、增长、碰撞、生成食物。
// 绘制相关的代码不要放在这里，避免规则和画面混在一起。

// 函数作用：初始化或重新开始一局游戏。
void InitGame(GameContext &context)
{
    // 重新开始一局时，先清空旧蛇身。
    context.game.snake.clear(); // 清掉上一局遗留的身体格子。

    // 蛇出生在地图中央。
    // 初始长度为 3：蛇头 + 两节身体。
    Cell start;
    start.x = context.settings.gridSize / 2; // x 放在地图中间。
    start.z = context.settings.gridSize / 2; // z 也放在地图中间。

    context.game.snake.push_back(start); // 第 0 节永远是蛇头。
    context.game.snake.push_back({start.x - 1, start.z}); // 第二节身体放在蛇头左边。
    context.game.snake.push_back({start.x - 2, start.z}); // 第三节身体继续往左排。

    // 初始方向向右。
    context.game.direction = {1, 0}; // {1, 0} 表示 x 增加，也就是向右。
    context.game.nextDirection = context.game.direction; // 还没输入时，下一方向等于当前方向。

    // 重置一局游戏的基本状态。
    context.game.food = {-1, -1}; // -1 表示暂时没有有效食物。
    context.game.status = Playing; // 初始化完成后进入游戏状态。
    context.game.score = 0; // 新的一局分数从 0 开始。
    context.game.moveTimer = 0.0f; // 移动计时器清零。
    context.game.started = false; // 等玩家第一次按方向键后再开始移动。
    context.game.isSuperFood = false; // 先默认不是金色超级果子。
    context.game.growLeft = 0; // 一开始没有待增长次数。

    // 重置临时视觉效果。
    context.effects.eatTimer = 0.0f; // 吃食物爆炸效果关闭。
    context.effects.scoreFlashTimer = 0.0f; // 分数闪烁效果关闭。
    context.effects.lastFoodCell = {-1, -1}; // 没有上一次食物位置。

    // 重置疯狂模式数据。
    // 即使普通模式也重置，保证每局开始状态干净。
    context.crazy.blocks.clear(); // 删除上一局留下的蓝色方块。
    context.crazy.eventTimer = 3.0f; // 开局约 3 秒后才触发第一次疯狂事件。
    context.crazy.boostTimer = 0.0f; // 当前不处于加速中。
    context.crazy.redFlashTimer = 0.0f; // 当前没有红光预警。
    context.crazy.greenFlashTimer = 0.0f; // 当前没有绿光恢复提示。
    context.crazy.boostWaiting = false; // 当前没有“红光结束后加速”的等待状态。

    // 第一颗食物在所有数据重置后生成。
    SpawnFood(context); // 初始化完蛇身后再生成食物，避免食物刷在蛇身上。
}

// 函数作用：使用菜单里的当前设置开始游戏。
void StartSelectedGame(GameContext &context)
{
    // 从菜单进入游戏时，先应用菜单设置，再初始化一局游戏。
    SetLevel(context, context.settings.currentLevel); // 应用菜单里选中的地图大小。
    SetSpeedByLevel(context, context.settings.speedLevel); // 应用菜单里选中的速度等级。
    UpdateTargetScore(context); // 根据地图和难度重新计算目标分。
    InitGame(context); // 用这些设置开新局。
}

// 函数作用：每帧更新普通贪吃蛇移动逻辑。
void UpdateGame(GameContext &context)
{
    // 菜单、暂停、失败、胜利时不移动蛇。
    bool gameIsPlaying = context.game.status == Playing; // 只有 Playing 才允许移动。

    // 玩家还没按方向键时，蛇先停在原地。
    bool snakeHasStarted = context.game.started; // false 表示玩家还没按过方向键。

    if (!gameIsPlaying || !snakeHasStarted)
    {
        return;
    }

    context.game.moveTimer += GetFrameTime(); // 累加本帧经过的秒数。

    // moveTimer 累计到移动间隔后，蛇才移动一格。
    // 这样可以让游戏保持“按格子移动”，更容易理解。
    if (context.game.moveTimer >= GetCurrentMoveInterval(context))
    {
        context.game.moveTimer = 0.0f; // 走完一格后重新计时。
        MoveSnake(context); // 真正移动蛇一格。
    }
}

// 函数作用：更新吃食物爆炸和分数闪烁的倒计时。
void UpdateEffects(GameContext &context)
{
    // 这里统一更新临时视觉效果的倒计时。
    // 倒计时减到 0 后，对应效果就不再绘制。
    float deltaTime = GetFrameTime(); // 本帧耗时，用它减少倒计时。

    if (context.effects.eatTimer > 0.0f)
    {
        context.effects.eatTimer -= deltaTime; // 倒计时越减越小，效果自然结束。

        if (context.effects.eatTimer < 0.0f)
        {
            context.effects.eatTimer = 0.0f; // 防止倒计时出现负数。
        }
    }

    if (context.effects.scoreFlashTimer > 0.0f)
    {
        context.effects.scoreFlashTimer -= deltaTime; // 分数闪烁也用同样的倒计时写法。

        if (context.effects.scoreFlashTimer < 0.0f)
        {
            context.effects.scoreFlashTimer = 0.0f; // 小于 0 后统一压回 0。
        }
    }
}

// 函数作用：判断两个格子坐标是否相同。
bool IsSameCell(Cell a, Cell b)
{
    // 两个格子的 x 和 z 都相同，才算同一个格子。
    return a.x == b.x && a.z == b.z; // 两个方向都相等，才是同一个格子。
}

// 函数作用：判断指定格子是否在蛇身上。
bool IsCellOnSnake(const GameContext &context, Cell cell)
{
    // 遍历蛇身每一节，只要有一节在这个格子上，就返回 true。
    for (int i = 0; i < (int)context.game.snake.size(); i++)
    {
        if (IsSameCell(context.game.snake[i], cell)) // 找到任意一节重合，就说明在蛇身上。
        {
            return true;
        }
    }

    return false;
}

// 函数作用：判断指定格子是否有蓝色方块。
bool IsCellOnCrazyBlock(const GameContext &context, Cell cell)
{
    // 这里不管蓝色方块是否已经变成实体。
    // 生成食物时，只要这个格子有蓝方块，就不能放食物。
    for (int i = 0; i < (int)context.crazy.blocks.size(); i++)
    {
        if (IsSameCell(context.crazy.blocks[i].cell, cell))
        {
            return true;
        }
    }

    return false;
}

// 函数作用：判断指定格子是否有实体蓝色障碍。
bool IsCellOnSolidCrazyBlock(const GameContext &context, Cell cell)
{
    // 只有 solid 为 true 的蓝方块才会撞死蛇。
    // 闪烁预警阶段不会撞死。
    for (int i = 0; i < (int)context.crazy.blocks.size(); i++)
    {
        bool blockIsSolid = context.crazy.blocks[i].solid; // true 表示已经变成会撞死的实体。
        bool sameCell = IsSameCell(context.crazy.blocks[i].cell, cell); // 判断方块位置是否相同。

        if (blockIsSolid && sameCell)
        {
            return true;
        }
    }

    return false;
}

// 函数作用：判断蛇头是否撞到地图边界。
bool CheckWallCollision(const GameContext &context, Cell head)
{
    // 坐标小于 0 或大于等于地图大小，就说明蛇头出了地图。
    bool hitLeftWall = head.x < 0; // x 小于 0，越过左边界。
    bool hitRightWall = head.x >= context.settings.gridSize; // x 太大，越过右边界。
    bool hitTopWall = head.z < 0; // z 小于 0，越过上边界。
    bool hitBottomWall = head.z >= context.settings.gridSize; // z 太大，越过下边界。

    return hitLeftWall || hitRightWall || hitTopWall || hitBottomWall;
}

// 函数作用：判断蛇头是否撞到自己的身体。
bool CheckSelfCollision(const GameContext &context, Cell head, bool willGrow)
{
    int checkEnd = (int)context.game.snake.size(); // 默认检查到最后一节身体。

    // 没吃到食物时，尾巴本回合会离开。
    // 所以可以不检查最后一节尾巴。
    if (!willGrow)
    {
        checkEnd--; // 不增长时尾巴会走开，所以最后一节不用检查。
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

// 函数作用：判断当前状态下蛇是否可以穿墙。
bool CanPassWall(const GameContext &context)
{
    // 只有疯狂模式 + 加速中，才允许从墙的一边穿到另一边。
    bool crazyModeIsOn = context.settings.crazyMode; // 菜单里是否开启疯狂模式。
    bool boostIsActive = context.crazy.boostTimer > 0.0f; // boostTimer 大于 0 表示正在加速。

    return crazyModeIsOn && boostIsActive;
}

// 函数作用：根据当前方向计算下一格蛇头位置。
Cell GetNextHead(const GameContext &context)
{
    // 新蛇头 = 当前蛇头 + 下一次移动方向。
    Cell head = context.game.snake[0]; // snake[0] 是当前蛇头。

    head.x += context.game.nextDirection.x; // x 加方向的 x，得到下一格 x。
    head.z += context.game.nextDirection.z; // z 加方向的 z，得到下一格 z。

    return head;
}

// 函数作用：允许穿墙时把越界蛇头移动到地图另一边。
void WrapHeadIfNeeded(GameContext &context, Cell &head)
{
    // 不能穿墙时，不做任何处理。
    if (!CanPassWall(context))
    {
        return;
    }

    // 能穿墙时，超出哪一边，就从相反边回来。
    if (head.x < 0)
    {
        head.x = context.settings.gridSize - 1; // 从左边出去，从最右边回来。
    }

    if (head.x >= context.settings.gridSize)
    {
        head.x = 0; // 从右边出去，从最左边回来。
    }

    if (head.z < 0)
    {
        head.z = context.settings.gridSize - 1; // 从上边出去，从最下边回来。
    }

    if (head.z >= context.settings.gridSize)
    {
        head.z = 0; // 从下边出去，从最上边回来。
    }
}

// 函数作用：统一判断本次移动是否会导致游戏失败。
bool CheckGameOver(GameContext &context, Cell head, bool willGrow)
{
    // 这里把复杂条件拆成几个 bool，方便新手阅读。
    bool snakeCanPassWall = CanPassWall(context); // 当前是否允许穿墙。
    bool snakeHitWall = CheckWallCollision(context, head); // 当前蛇头是否越界。
    bool snakeHitSelf = CheckSelfCollision(context, head, willGrow); // 当前蛇头是否撞身体。
    bool snakeHitBlueBlock = IsCellOnSolidCrazyBlock(context, head); // 当前蛇头是否撞实体蓝方块。

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

// 函数作用：如果蛇头吃到食物，就处理加分、增长和新食物。
void EatFoodIfNeeded(GameContext &context, Cell head)
{
    // 没吃到食物，就直接返回。
    bool ateFood = IsSameCell(head, context.game.food); // 蛇头和食物同格才算吃到。

    if (!ateFood)
    {
        return;
    }

    // 记录被吃掉的食物位置，用来画爆炸效果。
    context.effects.lastFoodCell = context.game.food; // 记录位置给爆炸效果使用。
    context.effects.eatTimer = EAT_EFFECT_TIME; // 启动吃食物爆炸倒计时。
    context.effects.scoreFlashTimer = SCORE_FLASH_TIME; // 启动分数变色倒计时。

    // 超级果子只负责更高分数和更多增长。
    // 它是否出现，由 SpawnFood() 决定。
    if (context.game.isSuperFood)
    {
        context.game.score += SCORE_PER_SUPER_FOOD; // 金色果子加更多分。
        context.game.growLeft += SUPER_FOOD_GROW; // 金色果子让蛇多长几节。
    }
    else
    {
        context.game.score += SCORE_PER_NORMAL_FOOD; // 普通果子加基础分。
        context.game.growLeft += NORMAL_FOOD_GROW; // 普通果子增长 1 节。
    }

    UpdateHighScore(context); // 如果当前分更高，就保存最高分。

    // 普通模式达到目标分数就胜利。
    // 疯狂模式没有目标分数，所以不会在这里胜利。
    bool normalMode = !context.settings.crazyMode; // 疯狂模式不走目标分胜利逻辑。
    bool reachedTargetScore = context.game.score >= context.settings.targetScore; // 是否达到菜单设定目标。

    if (normalMode && reachedTargetScore)
    {
        context.game.status = Win; // 普通模式达到目标分后胜利。
        context.game.food = {-1, -1}; // 胜利后不再显示食物。
        return;
    }

    SpawnFood(context); // 吃完旧食物后立刻生成下一颗。
}

// 函数作用：根据 growLeft 判断蛇是否需要增长。
void UpdateSnakeLength(GameContext &context)
{
    // growLeft > 0 表示本回合不删除尾巴。
    // 蛇头已经加了新格子，不删除尾巴就等于变长。
    if (context.game.growLeft > 0)
    {
        context.game.growLeft--; // 消耗一次“本回合不删尾巴”的机会。
    }
    else
    {
        context.game.snake.pop_back(); // 删除尾巴，蛇长度保持不变。
    }
}

// 函数作用：让蛇按当前方向移动一格。
void MoveSnake(GameContext &context)
{
    // 把玩家最近输入的方向正式变成当前移动方向。
    context.game.direction = context.game.nextDirection; // 正式采用上一帧记录的输入方向。

    // 先算出新蛇头，再根据疯狂模式决定是否穿墙。
    Cell newHead = GetNextHead(context); // 先算出下一格蛇头。
    WrapHeadIfNeeded(context, newHead); // 如果允许穿墙，就把越界坐标包回地图内。

    // willGrow 会影响撞自己检测。
    // 不增长时，尾巴会移开；增长时，尾巴不会移开。
    bool willGrow = IsSameCell(newHead, context.game.food); // 吃到食物时，本回合不会删尾巴。
    bool gameOver = CheckGameOver(context, newHead, willGrow); // 插入新蛇头前先检查会不会失败。

    if (gameOver)
    {
        context.game.status = GameOver; // 失败后状态改变，UpdateGame 下次不会继续移动。
        return;
    }

    // 确认没撞死后，才把新蛇头插到蛇身最前面。
    context.game.snake.insert(context.game.snake.begin(), newHead); // 新蛇头插到 vector 最前面。

    EatFoodIfNeeded(context, newHead); // 如果新蛇头压到食物，就处理加分和增长。
    UpdateSnakeLength(context); // 最后决定删不删尾巴。
}

// 函数作用：在安全空格子里随机生成新的食物。
void SpawnFood(GameContext &context)
{
    // 先收集所有可以放食物的空格子。
    // 这样随机时不会生成到蛇身或蓝色方块上。
    std::vector<Cell> emptyCells;

    for (int x = 0; x < context.settings.gridSize; x++)
    {
        for (int z = 0; z < context.settings.gridSize; z++)
        {
            Cell cell = {x, z}; // 当前正在检查的候选格子。

            bool cellOnSnake = IsCellOnSnake(context, cell); // 食物不能刷在蛇身上。
            bool cellOnBlock = IsCellOnCrazyBlock(context, cell); // 食物不能刷在蓝方块上。

            if (!cellOnSnake && !cellOnBlock)
            {
                emptyCells.push_back(cell); // 只把安全空格子加入候选列表。
            }
        }
    }

    if (emptyCells.empty())
    {
        // 没有空格子，说明地图已经被占满，玩家胜利。
        context.game.food = {-1, -1}; // 没地方放食物时隐藏食物。
        context.game.status = Win; // 地图被填满，直接胜利。
        UpdateHighScore(context); // 胜利时也检查最高分。
        return;
    }

    int index = rand() % emptyCells.size(); // 在所有空格子里随机选一个下标。

    context.game.food = emptyCells[index]; // 把选中的空格子作为食物位置。

    // 金色超级果子只在疯狂模式出现。
    // 普通模式下 canSpawnSuperFood 为 false，所以结果一定是普通果子。
    bool canSpawnSuperFood = context.settings.crazyMode; // 只有疯狂模式允许金色超级果子。
    bool randomHitSuperFoodChance = rand() % 100 < SUPER_FOOD_CHANCE; // 0-99 随机数，小于概率就命中。

    context.game.isSuperFood =
        canSpawnSuperFood && // 普通模式这里是 false，所以不会生成金果。
        randomHitSuperFoodChance; // 疯狂模式下再看随机概率。
}

// 函数作用：计算当前分数和疯狂模式下的蛇移动间隔。
float GetCurrentMoveInterval(const GameContext &context)
{
    // 分数越高，scoreSpeedLevel 越高，移动间隔越短。
    int scoreSpeedLevel = context.game.score / SPEED_UP_SCORE; // 每得到 SPEED_UP_SCORE 分，速度提升一级。

    float interval =
        context.settings.speed.startInterval - // 初始移动间隔。
        scoreSpeedLevel * context.settings.speed.speedUpAmount; // 分数越高，间隔减少越多。

    if (interval < context.settings.speed.minInterval)
    {
        interval = context.settings.speed.minInterval; // 不允许速度无限变快。
    }

    if (context.crazy.boostTimer > 0.0f)
    {
        // 疯狂模式加速时，移动间隔除以倍率，所以蛇会更快。
        interval = interval / BOOST_SPEED_RATE; // 间隔变短，实际速度变快；倍率当前是 1.6。
    }

    return interval;
}
