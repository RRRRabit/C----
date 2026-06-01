#include "crazy_mode.h"
#include "constants.h"
#include "snake_logic.h"
#include <cstdlib>

// crazy_mode.cpp 只负责疯狂模式的额外规则。
// 普通贪吃蛇规则仍然在 snake_logic.cpp 中。
// 这里主要是几个计时器：红光预警、加速、绿光恢复、蓝色方块。

// 函数作用：疯狂模式总入口，每帧更新所有疯狂模式事件。
void UpdateCrazyMode(GameContext &context)
{
    // 没开疯狂模式，或者游戏不在进行中，就不更新疯狂模式。
    bool crazyModeIsOff = !context.settings.crazyMode; // 菜单没有开启疯狂模式。
    bool gameIsNotPlaying = context.game.status != Playing; // 暂停、失败、胜利、菜单都不更新事件。

    if (crazyModeIsOff || gameIsNotPlaying)
    {
        return;
    }

    float deltaTime = GetFrameTime(); // 本帧经过的秒数，所有倒计时都减它。

    // 一个总入口拆成三个小步骤，比写在一个大函数里更清楚。
    UpdateBoostEvent(context, deltaTime); // 更新红光、加速、绿光。
    UpdateBlueBlocks(context, deltaTime); // 更新蓝方块预警是否结束。
    TryStartCrazyEvent(context); // 看看是否该触发下一个随机事件。
}

// 函数作用：更新红光预警、加速和绿光恢复三个计时器。
void UpdateBoostEvent(GameContext &context, float deltaTime)
{
    // boostTimer > 0 表示正在加速。
    // 倒计时结束后，进入绿色恢复提示。
    if (context.crazy.boostTimer > 0.0f)
    {
        context.crazy.boostTimer -= deltaTime; // 加速剩余时间逐帧减少。

        if (context.crazy.boostTimer <= 0.0f)
        {
            context.crazy.boostTimer = 0.0f; // 防止加速时间变成负数。
            context.crazy.greenFlashTimer = GREEN_FLASH_TIME; // 加速结束后显示绿色恢复提示。
        }
    }

    // redFlashTimer 是加速前的红色预警。
    // 红光结束后，才真正开始加速。
    if (context.crazy.redFlashTimer > 0.0f)
    {
        context.crazy.redFlashTimer -= deltaTime; // 红光预警剩余时间逐帧减少。

        if (context.crazy.redFlashTimer <= 0.0f)
        {
            context.crazy.redFlashTimer = 0.0f; // 红光结束。

            if (context.crazy.boostWaiting)
            {
                context.crazy.boostTimer = BOOST_TIME; // 红光结束后正式进入加速。
                context.crazy.boostWaiting = false; // 等待状态用完后关闭。
            }
        }
    }

    // greenFlashTimer 是加速结束后的恢复提示。
    if (context.crazy.greenFlashTimer > 0.0f)
    {
        context.crazy.greenFlashTimer -= deltaTime; // 绿色恢复提示逐帧减少。

        if (context.crazy.greenFlashTimer < 0.0f)
        {
            context.crazy.greenFlashTimer = 0.0f; // 防止出现负数。
        }
    }
}

// 函数作用：更新蓝色方块从闪烁预警到实体障碍的过程。
void UpdateBlueBlocks(GameContext &context, float deltaTime)
{
    // 蓝色方块刚出现时先闪烁，不会立刻变成实体。
    // flashTimer 结束后，solid 变成 true，蛇撞到才会失败。
    for (int i = 0; i < (int)context.crazy.blocks.size(); i++)
    {
        if (!context.crazy.blocks[i].solid) // 只有预警方块需要倒计时。
        {
            context.crazy.blocks[i].flashTimer -= deltaTime; // 蓝方块闪烁预警剩余时间。

            if (context.crazy.blocks[i].flashTimer <= 0.0f)
            {
                context.crazy.blocks[i].flashTimer = 0.0f; // 预警时间结束。
                context.crazy.blocks[i].solid = true; // 变成实体障碍，之后撞到会失败。
            }
        }
    }
}

// 函数作用：事件倒计时结束后尝试启动新的疯狂模式事件。
void TryStartCrazyEvent(GameContext &context)
{
    // eventTimer 控制下一次疯狂事件什么时候发生。
    context.crazy.eventTimer -= GetFrameTime(); // 随机事件总倒计时。

    if (context.crazy.eventTimer > 0.0f)
    {
        return;
    }

    // 加速流程还没结束时，不再启动新的加速。
    // 这样避免红光、加速、绿光状态互相叠在一起。
    if (IsBoostEventBusy(context))
    {
        StartBlueBlockEvent(context); // 加速流程忙时，只生成蓝方块，避免状态重叠。
    }
    else
    {
        TriggerRandomCrazyEvent(context); // 不忙时，随机选择一种疯狂事件。
    }

    // 下一次事件在 2 到 4 秒后发生。
    context.crazy.eventTimer = 2.0f + (float)(rand() % 3); // 下一次事件间隔为 2、3、4 秒。
}

// 函数作用：随机选择启动加速事件或蓝色方块事件。
void TriggerRandomCrazyEvent(GameContext &context)
{
    // 随机二选一：加速事件，或者蓝色方块事件。
    bool chooseBoost = rand() % 2 == 0; // 随机结果只有 0 或 1，做到二选一。

    if (chooseBoost)
    {
        StartBoostEvent(context);
    }
    else
    {
        StartBlueBlockEvent(context);
    }
}

// 函数作用：开始一次加速事件的红光预警阶段。
void StartBoostEvent(GameContext &context)
{
    // 如果已有加速相关流程，就不要重复启动。
    if (IsBoostEventBusy(context))
    {
        return;
    }

    // 先显示红光预警，不直接加速。
    context.crazy.redFlashTimer = RED_FLASH_TIME; // 先给玩家红光预警时间。
    context.crazy.boostWaiting = true; // 标记红光结束后要进入加速。
}

// 函数作用：在随机空格子生成一个新的蓝色方块。
void StartBlueBlockEvent(GameContext &context)
{
    // 限制蓝色方块数量，避免地图过快被塞满。
    if ((int)context.crazy.blocks.size() >= MAX_CRAZY_BLOCKS) // 达到上限就不再生成。
    {
        return;
    }

    // 和生成食物一样，先收集所有可用空格子，再随机选一个。
    std::vector<Cell> emptyCells;

    for (int x = 0; x < context.settings.gridSize; x++)
    {
        for (int z = 0; z < context.settings.gridSize; z++)
        {
            Cell cell = {x, z}; // 当前候选位置。

            bool cellOnSnake = IsCellOnSnake(context, cell); // 不能生成在蛇身上。
            bool cellIsFood = IsSameCell(cell, context.game.food); // 不能生成在食物上。
            bool cellOnBlock = IsCellOnCrazyBlock(context, cell); // 不能和旧蓝方块重叠。

            if (!cellOnSnake && !cellIsFood && !cellOnBlock)
            {
                emptyCells.push_back(cell); // 安全位置加入候选列表。
            }
        }
    }

    if (emptyCells.empty())
    {
        return;
    }

    int index = rand() % emptyCells.size(); // 从候选空格子里随机选一个。

    CrazyBlock block;
    block.cell = emptyCells[index]; // 记录蓝方块所在格子。

    // 新蓝方块先闪烁预警，暂时不是实体。
    block.solid = false; // 刚生成时只是预警，不会撞死。
    block.flashTimer = BLUE_FLASH_TIME; // 预警持续时间。

    context.crazy.blocks.push_back(block); // 放入蓝方块列表，之后每帧绘制和更新。
}

// 函数作用：判断加速事件的红光、加速或绿光流程是否还没结束。
bool IsBoostEventBusy(const GameContext &context)
{
    // 只要红光、加速、绿光、等待加速任意一个还在，
    // 就认为整个加速事件流程还没结束。
    bool redWarningIsActive = context.crazy.redFlashTimer > 0.0f; // 红光预警还在。
    bool boostIsActive = context.crazy.boostTimer > 0.0f; // 加速还在。
    bool greenRecoverIsActive = context.crazy.greenFlashTimer > 0.0f; // 绿光恢复还在。
    bool boostIsWaiting = context.crazy.boostWaiting; // 红光结束后还准备进入加速。

    return redWarningIsActive ||
           boostIsActive ||
           greenRecoverIsActive ||
           boostIsWaiting;
}
