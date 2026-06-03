#include "draw_ui.h"
#include "assets.h"
#include "constants.h"
#include "settings.h"

// 绘制资源加载失败提示。
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

// 计算关卡按钮位置。
Rectangle GetLevelButtonRect(int index)
{
    // index 从 0 开始，所以 index 0 对应 Level 1。
    int buttonWidth = 155;
    int buttonHeight = 65;
    int buttonGap = 25;

    int totalWidth =
        buttonWidth * LEVEL_COUNT + buttonGap * (LEVEL_COUNT - 1);

    int startX = GetMenuX() + (MENU_WIDTH - totalWidth) / 2;
    int y = MENU_Y + 190;

    return {
        (float)(startX + index * (buttonWidth + buttonGap)),
        (float)y,
        (float)buttonWidth,
        (float)buttonHeight
    };
}

// 速度减号按钮位置。
Rectangle GetSpeedMinusButtonRect()
{
    return {
        (float)(GetMenuX() + 110),
        (float)(MENU_Y + 400),
        65.0f,
        50.0f};
}

// 速度加号按钮位置。
Rectangle GetSpeedPlusButtonRect()
{
    return {
        (float)(GetMenuX() + MENU_WIDTH - 175),
        (float)(MENU_Y + 400),
        65.0f,
        50.0f};
}

// 难度减号按钮位置。
Rectangle GetDifficultyMinusButtonRect()
{
    return {
        (float)(GetMenuX() + 110),
        (float)(MENU_Y + 550),
        65.0f,
        50.0f};
}

// 难度加号按钮位置。
Rectangle GetDifficultyPlusButtonRect()
{
    return {
        (float)(GetMenuX() + MENU_WIDTH - 175),
        (float)(MENU_Y + 550),
        65.0f,
        50.0f};
}

// 疯狂模式按钮位置。
Rectangle GetCrazyModeButtonRect()
{
    int buttonWidth = 260;
    int buttonHeight = 55;

    return {
        (float)(GetMenuX() + (MENU_WIDTH - buttonWidth) / 2),
        (float)(MENU_Y + 615),
        (float)buttonWidth,
        (float)buttonHeight};
}

// START 按钮位置。
Rectangle GetStartButtonRect()
{
    int buttonWidth = 240;
    int buttonHeight = 70;

    return {
        (float)(GetMenuX() + (MENU_WIDTH - buttonWidth) / 2),
        (float)(MENU_Y + 680),
        (float)buttonWidth,
        (float)buttonHeight};
}

// 判断鼠标是否点击按钮。
bool IsButtonClicked(Rectangle button)
{
    bool mouseOnButton = CheckCollisionPointRec(GetMousePosition(), button);
    bool leftMousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    return mouseOnButton && leftMousePressed;
}

// 绘制通用菜单按钮。
void DrawButton(Rectangle button, const char *text, bool selected)
{
    bool mouseOver = CheckCollisionPointRec(GetMousePosition(), button);

    Color fillColor = (Color){45, 55, 70, 230};
    Color lineColor = (Color){170, 180, 195, 220};
    Color textColor = LIGHTGRAY;

    if (selected)
    {
        fillColor = (Color){60, 120, 80, 240};
        lineColor = (Color){230, 230, 160, 255};
        textColor = WHITE;
    }
    else if (mouseOver)
    {
        fillColor = (Color){65, 75, 92, 240};
        textColor = WHITE;
    }

    DrawRectangleRec(button, fillColor);

    DrawRectangleLines(
        (int)button.x,
        (int)button.y,
        (int)button.width,
        (int)button.height,
        lineColor);

    int fontSize = 22;
    int textWidth = MeasureText(text, fontSize);
    int textX = (int)(button.x + (button.width - textWidth) / 2);
    int textY = (int)(button.y + (button.height - fontSize) / 2);

    DrawText(text, textX, textY, fontSize, textColor);
}

// 绘制主菜单界面。
void DrawLevelSelectUI(const GameContext &context)
{
    DrawBackgroundImage(context);

    int menuX = GetMenuX();

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

    const char *title = "SNAKE MENU";
    int titleSize = 72;
    int titleWidth = MeasureText(title, titleSize);

    DrawText(title, (SCREEN_WIDTH - titleWidth) / 2, MENU_Y + 50, titleSize, YELLOW);

    const char *mapLabel = "Choose Map";
    int mapLabelSize = 32;
    int mapLabelWidth = MeasureText(mapLabel, mapLabelSize);

    DrawText(mapLabel, (SCREEN_WIDTH - mapLabelWidth) / 2, MENU_Y + 140, mapLabelSize, WHITE);

    for (int i = 0; i < LEVEL_COUNT; i++)
    {
        // 关卡按钮和地图大小文字一起画。
        Rectangle button = GetLevelButtonRect(i);

        std::string buttonText = "Level " + std::to_string(i + 1);

        DrawButton(
            button,
            buttonText.c_str(),
            context.settings.currentLevel == i + 1);

        int mapSize = GetMapSizeByLevel(i + 1);

        std::string mapText =
            std::to_string(mapSize) +
            " x " +
            std::to_string(mapSize);

        int mapTextWidth = MeasureText(mapText.c_str(), 22);

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

    std::string speedText = "Level " + std::to_string(context.settings.speedLevel);
    int speedTextWidth = MeasureText(speedText.c_str(), 34);

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

    std::string crazyText;

    if (context.settings.crazyMode)
    {
        crazyText = "Crazy Mode: ON";
    }
    else
    {
        crazyText = "Crazy Mode: OFF";
    }

    DrawButton(GetCrazyModeButtonRect(), crazyText.c_str(), context.settings.crazyMode);
    DrawButton(GetStartButtonRect(), "START", false);
    DrawAssetWarning(context);
}

// 根据状态绘制菜单、分数和提示。
void DrawUI(const GameContext &context)
{
    if (context.game.status == LevelSelect)
    {
        DrawLevelSelectUI(context);
        return;
    }

    std::string scoreText = "Score: " + std::to_string(context.game.score);

    Color scoreColor = WHITE;

    // 吃到食物后，分数短暂变黄。
    if (context.effects.scoreFlashTimer > 0.0f)
    {
        scoreColor = YELLOW;
    }

    int scoreSize = 42;
    int scoreWidth = MeasureText(scoreText.c_str(), scoreSize);

    DrawText(scoreText.c_str(), (SCREEN_WIDTH - scoreWidth) / 2, 25, scoreSize, scoreColor);

    std::string highScoreText = "High Score: " + std::to_string(context.highScore);

    std::string targetText;

    // 疯狂模式没有目标分数，所以显示 Infinite。
    if (context.settings.crazyMode)
    {
        targetText = "Target: Infinite";
    }
    else
    {
        targetText = "Target: " + std::to_string(context.settings.targetScore);
    }

    std::string levelText =
        "Level: " +
        GetLevelName(context) +
        " | Map: " +
        std::to_string(context.settings.gridSize) +
        " x " +
        std::to_string(context.settings.gridSize);

    std::string modeText;

    if (context.settings.crazyMode)
    {
        modeText = "Mode: Crazy";
    }
    else
    {
        modeText = "Mode: Normal";
    }

    if (context.crazy.boostTimer > 0.0f)
    {
        modeText += " | Boost";
    }
    else if (context.crazy.redFlashTimer > 0.0f)
    {
        modeText += " | Warning";
    }
    else if (context.crazy.greenFlashTimer > 0.0f)
    {
        modeText += " | Recover";
    }

    std::string infoText =
        highScoreText +
        "   |   " +
        targetText +
        "   |   " +
        levelText +
        "   |   " +
        modeText;

    int infoSize = 24;
    int infoWidth = MeasureText(infoText.c_str(), infoSize);

    DrawText(infoText.c_str(), (SCREEN_WIDTH - infoWidth) / 2, 78, infoSize, LIGHTGRAY);

    std::string speedText =
        "Speed Level: " +
        std::to_string(context.game.score / SPEED_UP_SCORE + 1);

    int speedWidth = MeasureText(speedText.c_str(), infoSize);

    DrawText(speedText.c_str(), (SCREEN_WIDTH - speedWidth) / 2, 110, infoSize, LIGHTGRAY);

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
        int fontSize = 25;
        int textWidth = MeasureText(message, fontSize);

        DrawText(
            message,
            (SCREEN_WIDTH - textWidth) / 2,
            SCREEN_HEIGHT / 2 + 200,
            fontSize,
            (Color){200, 200, 200, 180});
    }

    DrawAssetWarning(context);
}

// 绘制居中提示面板。
void DrawCenterMessage(const char *title, const char *subtitle, Color titleColor)
{
    int boxWidth = 520;
    int boxHeight = 170;
    int boxX = (SCREEN_WIDTH - boxWidth) / 2;
    int boxY = SCREEN_HEIGHT / 2 - 125;

    DrawRectangle(boxX, boxY, boxWidth, boxHeight, (Color){0, 0, 0, 150});
    DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, (Color){220, 220, 220, 140});

    int titleSize = 58;
    int subtitleSize = 26;
    int titleWidth = MeasureText(title, titleSize);
    int subtitleWidth = MeasureText(subtitle, subtitleSize);

    DrawText(title, (SCREEN_WIDTH - titleWidth) / 2, boxY + 30, titleSize, titleColor);
    DrawText(subtitle, (SCREEN_WIDTH - subtitleWidth) / 2, boxY + 105, subtitleSize, LIGHTGRAY);
}
