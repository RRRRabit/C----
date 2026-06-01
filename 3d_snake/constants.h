#pragma once

// ===== 窗口 =====
const int SCREEN_WIDTH = 2560;
const int SCREEN_HEIGHT = 1440;

// ===== 地图和关卡 =====
const int DEFAULT_GRID_SIZE = 15;
const int LEVEL_COUNT = 4;

// ===== 菜单设置范围 =====
const int MIN_SPEED_LEVEL = 1;
const int MAX_SPEED_LEVEL = 5;
const int MIN_DIFFICULTY_LEVEL = 1;
const int MAX_DIFFICULTY_LEVEL = 5;

// ===== 分数和成长 =====
const int SCORE_PER_NORMAL_FOOD = 10;
const int SCORE_PER_SUPER_FOOD = 50;
const int NORMAL_FOOD_GROW = 1;
const int SUPER_FOOD_GROW = 5;
const int SPEED_UP_SCORE = 50;
const int SUPER_FOOD_CHANCE = 20;

// ===== 文件路径 =====
const char *const HIGH_SCORE_FILE = "highscore.txt";
const char *const MENU_BACKGROUND_FILE = "assets/menu_background.png";

// ===== 3D 绘制 =====
const float CUBE_SIZE = 1.0f;

// ===== 简单特效 =====
const float EAT_EFFECT_TIME = 0.35f;
const float SCORE_FLASH_TIME = 0.25f;

// ===== 疯狂模式 =====
const float BOOST_TIME = 8.0f;
const float RED_FLASH_TIME = 1.0f;
const float GREEN_FLASH_TIME = 1.0f;
const float BOOST_SPEED_RATE = 1.6f;
const float BLUE_FLASH_TIME = 1.5f;
const int MAX_CRAZY_BLOCKS = 8;

// ===== Loading 过渡 =====
const float LOADING_HOLD_TIME = 0.4f;
const float LOADING_FADE_OUT_TIME = 0.15f;
const float LOADING_TIME = LOADING_HOLD_TIME + LOADING_FADE_OUT_TIME;

// ===== 主菜单布局 =====
const int MENU_WIDTH = 800;
const int MENU_HEIGHT = 750;
const int MENU_Y = 150;
