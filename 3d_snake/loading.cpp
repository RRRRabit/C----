#include "loading.h"
#include "constants.h"

// Loading 是覆盖在画面最上层的倒计时黑屏。

// 启动 Loading 过渡。
void StartLoading(GameContext &context)
{
    context.loading.timer = LOADING_TIME; // 黑屏保持 + 渐隐。
}

// 更新 Loading 倒计时。
void UpdateLoading(GameContext &context)
{
    if (context.loading.timer > 0.0f)
    {
        context.loading.timer -= GetFrameTime();

        if (context.loading.timer < 0.0f)
        {
            context.loading.timer = 0.0f; // 防止负数。
        }
    }
}

// 判断是否正在 Loading。
bool IsLoading(const GameContext &context)
{
    return context.loading.timer > 0.0f;
}

// 绘制 Loading 黑屏和渐隐。
void DrawLoadingScreen(const GameContext &context)
{
    if (!IsLoading(context))
    {
        return;
    }

    // 两段：先纯黑，再渐隐。
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

    unsigned char alpha = (unsigned char)(255 * alphaRate); // 0-255 透明度。

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
