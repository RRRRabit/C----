#pragma once

// ===== 窗口 =====
const int SCREEN_WIDTH = 2560;
const int SCREEN_HEIGHT = 1440;

// ===== 地图和关卡 =====
const int DEFAULT_GRID_SIZE = 15; // 非法关卡时使用的默认地图边长。
const int LEVEL_COUNT = 4;

// ===== 菜单设置范围 =====
const int MIN_SPEED_LEVEL = 1;
const int MAX_SPEED_LEVEL = 5;
const int MIN_DIFFICULTY_LEVEL = 1;
const int MAX_DIFFICULTY_LEVEL = 5;

// ===== 分数和成长 =====
const int SCORE_PER_NORMAL_FOOD = 10; // 普通食物加分。
const int SCORE_PER_SUPER_FOOD = 50; // 金色超级果子加分。
const int NORMAL_FOOD_GROW = 1; // 普通食物增长节数。
const int SUPER_FOOD_GROW = 5; // 金色超级果子增长节数。
const int SPEED_UP_SCORE = 50; // 每 50 分提升一次速度显示等级。
const int SUPER_FOOD_CHANCE = 20; // 金色超级果子概率，20 表示 20%。

// ===== 文件路径 =====
const char *const HIGH_SCORE_FILE = "highscore.txt";
const char *const MENU_BACKGROUND_FILE = "assets/menu_background.png";

// ===== 3D 绘制 =====
const float CUBE_SIZE = 1.0f;

// ===== 简单特效 =====
const float EAT_EFFECT_TIME = 0.35f; // 吃食物爆炸效果持续时间，单位秒。
const float SCORE_FLASH_TIME = 0.25f; // 分数变黄持续时间，单位秒。

// ===== 疯狂模式 =====
const float BOOST_TIME = 8.0f; // 疯狂模式加速持续时间，单位秒。
const float RED_FLASH_TIME = 1.0f; // 加速前红光预警时间，单位秒。
const float GREEN_FLASH_TIME = 1.0f; // 加速后绿光恢复提示时间，单位秒。
const float BOOST_SPEED_RATE = 1.6f; // 疯狂模式加速倍率，1.6 表示速度变成 1.6 倍。
const float BLUE_FLASH_TIME = 1.5f; // 蓝方块从预警变实体的时间，单位秒。
const int MAX_CRAZY_BLOCKS = 8; // 蓝色障碍方块最大数量。

// ===== Loading 过渡 =====
const float LOADING_HOLD_TIME = 0.4f; // Loading 完全黑屏保持时间。
const float LOADING_FADE_OUT_TIME = 0.15f; // Loading 渐隐时间。
const float LOADING_TIME = LOADING_HOLD_TIME + LOADING_FADE_OUT_TIME; // Loading 总时长。

// ===== 主菜单布局 =====
const int MENU_WIDTH = 800;
const int MENU_HEIGHT = 750;
const int MENU_Y = 150;
