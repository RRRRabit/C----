#include "loading.h"
#include "constants.h"

void StartLoading(GameContext &context)
{
    context.loading.timer = LOADING_TIME;
}

void UpdateLoading(GameContext &context)
{
    if (context.loading.timer > 0.0f)
    {
        context.loading.timer -= GetFrameTime();

        if (context.loading.timer < 0.0f)
        {
            context.loading.timer = 0.0f;
        }
    }
}

bool IsLoading(const GameContext &context)
{
    return context.loading.timer > 0.0f;
}

void DrawLoadingScreen(const GameContext &context)
{
    if (!IsLoading(context))
    {
        return;
    }

    float timePassed = LOADING_TIME - context.loading.timer;
    float alphaRate;

    if (timePassed < LOADING_HOLD_TIME)
    {
        alphaRate = 1.0f;
    }
    else
    {
        alphaRate = context.loading.timer / LOADING_FADE_OUT_TIME;
    }

    unsigned char alpha = (unsigned char)(255 * alphaRate);

    DrawRectangle(
        0,
        0,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        (Color){0, 0, 0, alpha});

    const char *text = "LOADING...";
    int fontSize = 46;
    int textWidth = MeasureText(text, fontSize);

    DrawText(
        text,
        (SCREEN_WIDTH - textWidth) / 2,
        SCREEN_HEIGHT / 2 - fontSize / 2,
        fontSize,
        (Color){255, 255, 255, alpha});
}
