#include "crazy_mode.h"
#include "constants.h"
#include "snake_logic.h"
#include <cstdlib>

// 疯狂模式额外规则：加速事件和蓝色方块。

// 每帧更新疯狂模式。
void UpdateCrazyMode(GameContext &context)
{
    bool crazyModeIsOff = !context.settings.crazyMode;
    bool gameIsNotPlaying = context.game.status != Playing;

    if (crazyModeIsOff || gameIsNotPlaying)
    {
        return;
    }

    float deltaTime = GetFrameTime();

    UpdateBoostEvent(context, deltaTime);
    UpdateBlueBlocks(context, deltaTime);
    TryStartCrazyEvent(context);
}

// 更新加速事件计时器。
void UpdateBoostEvent(GameContext &context, float deltaTime)
{
    // 流程：红光预警 -> 加速 -> 绿光恢复。
    if (context.crazy.boostTimer > 0.0f)
    {
        context.crazy.boostTimer -= deltaTime;

        if (context.crazy.boostTimer <= 0.0f)
        {
            context.crazy.boostTimer = 0.0f;
            context.crazy.greenFlashTimer = GREEN_FLASH_TIME;
        }
    }

    if (context.crazy.redFlashTimer > 0.0f)
    {
        context.crazy.redFlashTimer -= deltaTime;

        if (context.crazy.redFlashTimer <= 0.0f)
        {
            context.crazy.redFlashTimer = 0.0f;

            if (context.crazy.boostWaiting)
            {
                context.crazy.boostTimer = BOOST_TIME;
                context.crazy.boostWaiting = false;
            }
        }
    }

    if (context.crazy.greenFlashTimer > 0.0f)
    {
        context.crazy.greenFlashTimer -= deltaTime;

        if (context.crazy.greenFlashTimer < 0.0f)
        {
            context.crazy.greenFlashTimer = 0.0f; // 防止负数。
        }
    }
}

// 更新蓝方块预警。
void UpdateBlueBlocks(GameContext &context, float deltaTime)
{
    // 蓝色方块刚出现时先闪烁，不会立刻变成实体。
    // flashTimer 结束后，solid 变成 true，蛇撞到才会失败。
    for (int i = 0; i < (int)context.crazy.blocks.size(); i++)
    {
        if (!context.crazy.blocks[i].solid)
        {
            context.crazy.blocks[i].flashTimer -= deltaTime;

            if (context.crazy.blocks[i].flashTimer <= 0.0f)
            {
                context.crazy.blocks[i].flashTimer = 0.0f;
                context.crazy.blocks[i].solid = true;
            }
        }
    }
}

// 尝试启动新的疯狂事件。
void TryStartCrazyEvent(GameContext &context)
{
    context.crazy.eventTimer -= GetFrameTime();

    if (context.crazy.eventTimer > 0.0f)
    {
        return;
    }

    // 加速流程还没结束时，不再启动新的加速。
    // 这样避免红光、加速、绿光状态互相叠在一起。
    if (IsBoostEventBusy(context))
    {
        StartBlueBlockEvent(context);
    }
    else
    {
        TriggerRandomCrazyEvent(context);
    }

    context.crazy.eventTimer = 2.0f + (float)(rand() % 3); // 2 到 4 秒。
}

// 随机选择疯狂事件。
void TriggerRandomCrazyEvent(GameContext &context)
{
    bool chooseBoost = rand() % 2 == 0;

    if (chooseBoost)
    {
        StartBoostEvent(context);
    }
    else
    {
        StartBlueBlockEvent(context);
    }
}

// 开始加速预警。
void StartBoostEvent(GameContext &context)
{
    if (IsBoostEventBusy(context))
    {
        return;
    }

    context.crazy.redFlashTimer = RED_FLASH_TIME;
    context.crazy.boostWaiting = true; // 红光结束后进入加速。
}

// 生成蓝方块。
void StartBlueBlockEvent(GameContext &context)
{
    if ((int)context.crazy.blocks.size() >= MAX_CRAZY_BLOCKS)
    {
        return;
    }

    // 和生成食物一样，先收集所有可用空格子，再随机选一个。
    std::vector<Cell> emptyCells;

    for (int x = 0; x < context.settings.gridSize; x++)
    {
        for (int z = 0; z < context.settings.gridSize; z++)
        {
            Cell cell = {x, z}; // 候选位置。

            bool cellOnSnake = IsCellOnSnake(context, cell);
            bool cellIsFood = IsSameCell(cell, context.game.food);
            bool cellOnBlock = IsCellOnCrazyBlock(context, cell);

            if (!cellOnSnake && !cellIsFood && !cellOnBlock)
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

    // 新蓝方块先闪烁预警，暂时不是实体。
    block.solid = false;
    block.flashTimer = BLUE_FLASH_TIME;

    context.crazy.blocks.push_back(block);
}

// 判断加速流程是否还没结束。
bool IsBoostEventBusy(const GameContext &context)
{
    // 只要红光、加速、绿光、等待加速任意一个还在，
    // 就认为整个加速事件流程还没结束。
    bool redWarningIsActive = context.crazy.redFlashTimer > 0.0f;
    bool boostIsActive = context.crazy.boostTimer > 0.0f;
    bool greenRecoverIsActive = context.crazy.greenFlashTimer > 0.0f;
    bool boostIsWaiting = context.crazy.boostWaiting;

    return redWarningIsActive ||
           boostIsActive ||
           greenRecoverIsActive ||
           boostIsWaiting;
}
