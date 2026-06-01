#include "loading.h"
#include "constants.h"

// Loading 不是一个新的 GameStatus。
// 它只是一个覆盖在画面最上层的倒计时黑屏。

// 函数作用：启动一次 Loading 黑屏过渡。
void StartLoading(GameContext &context)
{
    // 重新把倒计时设为完整 Loading 时长。
    context.loading.timer = LOADING_TIME; // 总时长 = 黑屏保持时间 + 渐隐时间。
}

// 函数作用：每帧减少 Loading 倒计时。
void UpdateLoading(GameContext &context)
{
    // 每帧减少一点时间，减到 0 就结束 Loading。
    if (context.loading.timer > 0.0f)
    {
        context.loading.timer -= GetFrameTime(); // 每帧扣掉经过的秒数。

        if (context.loading.timer < 0.0f)
        {
            context.loading.timer = 0.0f; // 小于 0 时统一归零，方便 IsLoading 判断。
        }
    }
}

// 函数作用：判断当前是否还处于 Loading 过渡中。
bool IsLoading(const GameContext &context)
{
    // 只要 timer 大于 0，就说明 Loading 还在显示。
    return context.loading.timer > 0.0f; // 只要倒计时没结束，就还在 Loading。
}

// 函数作用：绘制 Loading 黑屏和文字，并实现渐隐。
void DrawLoadingScreen(const GameContext &context)
{
    if (!IsLoading(context))
    {
        return;
    }

    // Loading 分两段：
    // 先保持纯黑，再逐渐透明。
    float timePassed = LOADING_TIME - context.loading.timer; // 已经过去的 Loading 时间。
    float alphaRate;

    if (timePassed < LOADING_HOLD_TIME)
    {
        alphaRate = 1.0f; // 前半段保持完全黑屏。
    }
    else
    {
        alphaRate = context.loading.timer / LOADING_FADE_OUT_TIME; // 后半段从 1 慢慢降到 0。
    }

    // alpha 控制黑屏透明度。
    // 255 是完全不透明，0 是完全透明。
    unsigned char alpha = (unsigned char)(255 * alphaRate); // 把 0-1 的比例转成 0-255 的透明度。

    DrawRectangle(
        0,
        0,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        (Color){0, 0, 0, alpha});

    const char *text = "LOADING..."; // Loading 显示的文字。
    int fontSize = 46; // 文字大小。
    int textWidth = MeasureText(text, fontSize); // 用于水平居中。

    DrawText(
        text,
        (SCREEN_WIDTH - textWidth) / 2,
        SCREEN_HEIGHT / 2 - fontSize / 2,
        fontSize,
        (Color){255, 255, 255, alpha});
}
