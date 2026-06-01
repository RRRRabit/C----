#include "crazy_mode.h"
#include "constants.h"
#include "snake_logic.h"
#include <cstdlib>

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

void UpdateBoostEvent(GameContext &context, float deltaTime)
{
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
            context.crazy.greenFlashTimer = 0.0f;
        }
    }
}

void UpdateBlueBlocks(GameContext &context, float deltaTime)
{
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

void TryStartCrazyEvent(GameContext &context)
{
    context.crazy.eventTimer -= GetFrameTime();

    if (context.crazy.eventTimer > 0.0f)
    {
        return;
    }

    if (IsBoostEventBusy(context))
    {
        StartBlueBlockEvent(context);
    }
    else
    {
        TriggerRandomCrazyEvent(context);
    }

    context.crazy.eventTimer = 2.0f + (float)(rand() % 3);
}

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

void StartBoostEvent(GameContext &context)
{
    if (IsBoostEventBusy(context))
    {
        return;
    }

    context.crazy.redFlashTimer = RED_FLASH_TIME;
    context.crazy.boostWaiting = true;
}

void StartBlueBlockEvent(GameContext &context)
{
    if ((int)context.crazy.blocks.size() >= MAX_CRAZY_BLOCKS)
    {
        return;
    }

    std::vector<Cell> emptyCells;

    for (int x = 0; x < context.settings.gridSize; x++)
    {
        for (int z = 0; z < context.settings.gridSize; z++)
        {
            Cell cell = {x, z};

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
    block.solid = false;
    block.flashTimer = BLUE_FLASH_TIME;

    context.crazy.blocks.push_back(block);
}

bool IsBoostEventBusy(const GameContext &context)
{
    bool redWarningIsActive = context.crazy.redFlashTimer > 0.0f;
    bool boostIsActive = context.crazy.boostTimer > 0.0f;
    bool greenRecoverIsActive = context.crazy.greenFlashTimer > 0.0f;
    bool boostIsWaiting = context.crazy.boostWaiting;

    return redWarningIsActive ||
           boostIsActive ||
           greenRecoverIsActive ||
           boostIsWaiting;
}
