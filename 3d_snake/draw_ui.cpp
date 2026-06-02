#include "draw_ui.h"
#include "assets.h"
#include "constants.h"
#include "settings.h"

// 函数作用：绘制资源加载失败提示。
void DrawAssetWarning(const GameContext &context)
{
    if (!context.assets.menuBackgroundLoadFailed)
    {
        return;
    }

    DrawText(
        "Warning: background image failed to load",
        20,
        20,
        24,
        (Color){255, 210, 80, 255});
}

// 函数作用：计算某个关卡按钮在屏幕上的矩形区域。
Rectangle GetLevelButtonRect(int index)
{
    // index 从 0 开始，所以 index 0 对应 Level 1。
    int buttonWidth = 155; // 单个关卡按钮宽度。
    int buttonHeight = 65; // 单个关卡按钮高度。
    int buttonGap = 25; // 关卡按钮之间的间距。

    int totalWidth =
        buttonWidth * LEVEL_COUNT + buttonGap * (LEVEL_COUNT - 1);

    int startX = GetMenuX() + (MENU_WIDTH - totalWidth) / 2; // 让这一排按钮在菜单里居中。
    int y = MENU_Y + 190; // 关卡按钮所在的纵向位置。

    return {
        (float)(startX + index * (buttonWidth + buttonGap)), // 第 index 个按钮的 x。
        (float)y, // 按钮 y。
        (float)buttonWidth, // 按钮宽度。
        (float)buttonHeight
    };
}

// 函数作用：返回速度减号按钮的矩形区域。
Rectangle GetSpeedMinusButtonRect()
{
    return {
        (float)(GetMenuX() + 110),
        (float)(MENU_Y + 400),
        65.0f,
        50.0f};
}

// 函数作用：返回速度加号按钮的矩形区域。
Rectangle GetSpeedPlusButtonRect()
{
    return {
        (float)(GetMenuX() + MENU_WIDTH - 175),
        (float)(MENU_Y + 400),
        65.0f,
        50.0f};
}

// 函数作用：返回难度减号按钮的矩形区域。
Rectangle GetDifficultyMinusButtonRect()
{
    return {
        (float)(GetMenuX() + 110),
        (float)(MENU_Y + 550),
        65.0f,
        50.0f};
}

// 函数作用：返回难度加号按钮的矩形区域。
Rectangle GetDifficultyPlusButtonRect()
{
    return {
        (float)(GetMenuX() + MENU_WIDTH - 175),
        (float)(MENU_Y + 550),
        65.0f,
        50.0f};
}

// 函数作用：返回疯狂模式开关按钮的矩形区域。
Rectangle GetCrazyModeButtonRect()
{
    int buttonWidth = 260; // Crazy Mode 按钮宽度。
    int buttonHeight = 55; // Crazy Mode 按钮高度。

    return {
        (float)(GetMenuX() + (MENU_WIDTH - buttonWidth) / 2),
        (float)(MENU_Y + 615),
        (float)buttonWidth,
        (float)buttonHeight};
}

// 函数作用：返回 START 按钮的矩形区域。
Rectangle GetStartButtonRect()
{
    int buttonWidth = 240; // START 按钮宽度。
    int buttonHeight = 70; // START 按钮高度。

    return {
        (float)(GetMenuX() + (MENU_WIDTH - buttonWidth) / 2),
        (float)(MENU_Y + 680),
        (float)buttonWidth,
        (float)buttonHeight};
}

// 函数作用：判断鼠标是否点击了指定按钮区域。
bool IsButtonClicked(Rectangle button)
{
    // 鼠标在按钮矩形内，并且按下左键，才算点击按钮。
    bool mouseOnButton = CheckCollisionPointRec(GetMousePosition(), button); // 鼠标坐标是否在按钮矩形内。
    bool leftMousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT); // 左键是否刚按下。

    return mouseOnButton && leftMousePressed;
}

// 函数作用：绘制一个通用菜单按钮。
void DrawButton(Rectangle button, const char *text, bool selected)
{
    // 一个通用按钮函数。
    // selected 用来表示当前选中的关卡或模式。
    bool mouseOver = CheckCollisionPointRec(GetMousePosition(), button); // 用于悬停变色。

    Color fillColor = (Color){45, 55, 70, 230}; // 默认按钮底色。
    Color lineColor = (Color){170, 180, 195, 220}; // 默认边框颜色。
    Color textColor = LIGHTGRAY; // 默认文字颜色。

    if (selected)
    {
        fillColor = (Color){60, 120, 80, 240}; // 被选中的按钮用绿色。
        lineColor = (Color){230, 230, 160, 255}; // 被选中的按钮边框更亮。
        textColor = WHITE; // 被选中的按钮文字更亮。
    }
    else if (mouseOver)
    {
        fillColor = (Color){65, 75, 92, 240}; // 鼠标悬停时略微变亮。
        textColor = WHITE;
    }

    DrawRectangleRec(button, fillColor);

    DrawRectangleLines(
        (int)button.x,
        (int)button.y,
        (int)button.width,
        (int)button.height,
        lineColor);

    int fontSize = 22; // 按钮文字大小。
    int textWidth = MeasureText(text, fontSize); // 用于计算水平居中。
    int textX = (int)(button.x + (button.width - textWidth) / 2); // 文字 x 居中。
    int textY = (int)(button.y + (button.height - fontSize) / 2); // 文字 y 近似居中。

    DrawText(text, textX, textY, fontSize, textColor);
}

// 函数作用：绘制主菜单界面。
void DrawLevelSelectUI(const GameContext &context)
{
    // 菜单界面先画背景，再画半透明菜单面板和按钮。
    DrawBackgroundImage(context);

    int menuX = GetMenuX(); // 菜单面板左上角 x。

    DrawRectangle(
        menuX,
        MENU_Y,
        MENU_WIDTH,
        MENU_HEIGHT,
        (Color){10, 15, 30, 200});

    DrawRectangleLines(
        menuX,
        MENU_Y,
        MENU_WIDTH,
        MENU_HEIGHT,
        (Color){180, 180, 200, 160});

    const char *title = "SNAKE MENU"; // 菜单标题。
    int titleSize = 72; // 标题字号。
    int titleWidth = MeasureText(title, titleSize); // 标题宽度，用于居中。

    DrawText(title, (SCREEN_WIDTH - titleWidth) / 2, MENU_Y + 50, titleSize, YELLOW);

    const char *mapLabel = "Choose Map";
    int mapLabelSize = 32;
    int mapLabelWidth = MeasureText(mapLabel, mapLabelSize);

    DrawText(mapLabel, (SCREEN_WIDTH - mapLabelWidth) / 2, MENU_Y + 140, mapLabelSize, WHITE);

    for (int i = 0; i < LEVEL_COUNT; i++)
    {
        // 关卡按钮和地图大小文字一起画。
        Rectangle button = GetLevelButtonRect(i); // 当前关卡按钮的位置。

        std::string buttonText = "Level " + std::to_string(i + 1); // 按钮显示的关卡名。

        DrawButton(
            button,
            buttonText.c_str(),
            context.settings.currentLevel == i + 1);

        int mapSize = GetMapSizeByLevel(i + 1); // 当前关卡对应的地图边长。

        std::string mapText =
            std::to_string(mapSize) +
            " x " +
            std::to_string(mapSize);

        int mapTextWidth = MeasureText(mapText.c_str(), 22); // 地图大小文字宽度，用于居中。

        DrawText(
            mapText.c_str(),
            (int)(button.x + (button.width - mapTextWidth) / 2),
            (int)(button.y + button.height + 8),
            22,
            LIGHTGRAY);
    }

    const char *speedLabel = "Speed";
    int speedLabelSize = 32;
    int speedLabelWidth = MeasureText(speedLabel, speedLabelSize);

    DrawText(speedLabel, (SCREEN_WIDTH - speedLabelWidth) / 2, MENU_Y + 340, speedLabelSize, WHITE);

    DrawButton(GetSpeedMinusButtonRect(), "-", false);
    DrawButton(GetSpeedPlusButtonRect(), "+", false);

    std::string speedText = "Level " + std::to_string(context.settings.speedLevel); // 当前速度等级文字。
    int speedTextWidth = MeasureText(speedText.c_str(), 34); // 用于居中。

    DrawText(speedText.c_str(), (SCREEN_WIDTH - speedTextWidth) / 2, MENU_Y + 408, 34, WHITE);

    const char *diffLabel = "Difficulty Target";
    int diffLabelSize = 32;
    int diffLabelWidth = MeasureText(diffLabel, diffLabelSize);

    DrawText(diffLabel, (SCREEN_WIDTH - diffLabelWidth) / 2, MENU_Y + 490, diffLabelSize, WHITE);

    DrawButton(GetDifficultyMinusButtonRect(), "-", false);
    DrawButton(GetDifficultyPlusButtonRect(), "+", false);

    std::string difficultyText =
        "Level " +
        std::to_string(context.settings.difficultyLevel) +
        "  Target " +
        std::to_string(context.settings.targetScore);

    int diffTextWidth = MeasureText(difficultyText.c_str(), 32);

    DrawText(difficultyText.c_str(), (SCREEN_WIDTH - diffTextWidth) / 2, MENU_Y + 558, 32, WHITE);

    std::string crazyText; // Crazy Mode 开关按钮文字。

    if (context.settings.crazyMode)
    {
        crazyText = "Crazy Mode: ON"; // 开启时显示 ON。
    }
    else
    {
        crazyText = "Crazy Mode: OFF"; // 关闭时显示 OFF。
    }

    DrawButton(GetCrazyModeButtonRect(), crazyText.c_str(), context.settings.crazyMode);
    DrawButton(GetStartButtonRect(), "START", false);
    DrawAssetWarning(context);
}

// 函数作用：根据当前游戏状态绘制菜单、分数和状态提示。
void DrawUI(const GameContext &context)
{
    // 如果当前在菜单状态，整个 UI 就是菜单。
    if (context.game.status == LevelSelect)
    {
        DrawLevelSelectUI(context);
        return;
    }

    // 游戏中先画分数，再画一行详细信息。
    std::string scoreText = "Score: " + std::to_string(context.game.score); // 当前分数文字。

    Color scoreColor = WHITE; // 默认分数颜色。

    // 吃到食物后，分数短暂变黄。
    if (context.effects.scoreFlashTimer > 0.0f)
    {
        scoreColor = YELLOW; // 吃到食物后短暂变黄。
    }

    int scoreSize = 42; // 分数字号。
    int scoreWidth = MeasureText(scoreText.c_str(), scoreSize); // 分数文字宽度，用于居中。

    DrawText(scoreText.c_str(), (SCREEN_WIDTH - scoreWidth) / 2, 25, scoreSize, scoreColor);

    std::string highScoreText = "High Score: " + std::to_string(context.highScore); // 最高分文字。

    std::string targetText;

    // 疯狂模式没有目标分数，所以显示 Infinite。
    if (context.settings.crazyMode)
    {
        targetText = "Target: Infinite"; // 疯狂模式没有通关目标分。
    }
    else
    {
        targetText = "Target: " + std::to_string(context.settings.targetScore); // 普通模式显示目标分。
    }

    std::string levelText =
        "Level: " +
        GetLevelName(context) +
        " | Map: " +
        std::to_string(context.settings.gridSize) +
        " x " +
        std::to_string(context.settings.gridSize);

    std::string modeText;

    // modeText 会附带疯狂模式的当前状态提示。
    if (context.settings.crazyMode)
    {
        modeText = "Mode: Crazy"; // 疯狂模式文字。
    }
    else
    {
        modeText = "Mode: Normal"; // 普通模式文字。
    }

    if (context.crazy.boostTimer > 0.0f)
    {
        modeText += " | Boost"; // 当前正在加速。
    }
    else if (context.crazy.redFlashTimer > 0.0f)
    {
        modeText += " | Warning"; // 当前红光预警。
    }
    else if (context.crazy.greenFlashTimer > 0.0f)
    {
        modeText += " | Recover"; // 当前绿光恢复提示。
    }

    std::string infoText =
        highScoreText +
        "   |   " +
        targetText +
        "   |   " +
        levelText +
        "   |   " +
        modeText;

    int infoSize = 24; // 顶部信息字号。
    int infoWidth = MeasureText(infoText.c_str(), infoSize); // 用于居中。

    DrawText(infoText.c_str(), (SCREEN_WIDTH - infoWidth) / 2, 78, infoSize, LIGHTGRAY);

    std::string speedText =
        "Speed Level: " +
        std::to_string(context.game.score / SPEED_UP_SCORE + 1); // 分数越高，显示的速度等级越高。

    int speedWidth = MeasureText(speedText.c_str(), infoSize);

    DrawText(speedText.c_str(), (SCREEN_WIDTH - speedWidth) / 2, 110, infoSize, LIGHTGRAY);

    // 屏幕左下角显示操作提示。
    DrawText("WASD / Arrow Keys: Move", 20, SCREEN_HEIGHT - 120, 20, LIGHTGRAY);
    DrawText("M: Back to Menu", 20, SCREEN_HEIGHT - 85, 20, LIGHTGRAY);
    DrawText("Space: Pause | R: Restart | Esc: Quit", 20, SCREEN_HEIGHT - 45, 20, LIGHTGRAY);

    if (context.game.status == Paused)
    {
        DrawCenterMessage("PAUSED", "Press Space to continue", YELLOW);
    }

    if (context.game.status == GameOver)
    {
        std::string resultText =
            "Final Score: " +
            std::to_string(context.game.score) +
            " | R restart | M menu";

        DrawCenterMessage("GAME OVER", resultText.c_str(), RED);
    }

    if (context.game.status == Win)
    {
        std::string resultText =
            GetLevelName(context) +
            " Clear | Score: " +
            std::to_string(context.game.score) +
            " | M menu";

        DrawCenterMessage("YOU WIN", resultText.c_str(), GREEN);
    }

    if (!context.game.started && context.game.status == Playing)
    {
        const char *message = "Press WASD or Arrow Keys to start";
        int fontSize = 25; // 开始提示字号。
        int textWidth = MeasureText(message, fontSize); // 用于居中。

        DrawText(
            message,
            (SCREEN_WIDTH - textWidth) / 2,
            SCREEN_HEIGHT / 2 + 200,
            fontSize,
            (Color){200, 200, 200, 180});
    }

    DrawAssetWarning(context);
}

// 函数作用：绘制暂停、失败、胜利时居中的提示面板。
void DrawCenterMessage(const char *title, const char *subtitle, Color titleColor)
{
    // 暂停、失败、胜利都用同一个居中面板。
    int boxWidth = 520; // 中央提示框宽度。
    int boxHeight = 170; // 中央提示框高度。
    int boxX = (SCREEN_WIDTH - boxWidth) / 2; // 提示框水平居中。
    int boxY = SCREEN_HEIGHT / 2 - 125; // 提示框略高于屏幕中心。

    DrawRectangle(boxX, boxY, boxWidth, boxHeight, (Color){0, 0, 0, 150});
    DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, (Color){220, 220, 220, 140});

    int titleSize = 58; // 主标题字号。
    int subtitleSize = 26; // 副标题字号。
    int titleWidth = MeasureText(title, titleSize); // 主标题宽度，用于居中。
    int subtitleWidth = MeasureText(subtitle, subtitleSize); // 副标题宽度，用于居中。

    DrawText(title, (SCREEN_WIDTH - titleWidth) / 2, boxY + 30, titleSize, titleColor);
    DrawText(subtitle, (SCREEN_WIDTH - subtitleWidth) / 2, boxY + 105, subtitleSize, LIGHTGRAY);
}
