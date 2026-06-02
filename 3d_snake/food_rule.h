#pragma once

#include "raylib.h"

// food_rule.h 用来展示简单的面向对象设计。
// FoodRule 是父类，普通果子和金色果子是子类。
// 游戏通过父类引用调用函数时，会自动执行对应子类的版本，这就是多态。

// 食物规则父类。
class FoodRule
{
public:
    virtual ~FoodRule() {}

    // 函数作用：返回这种食物能增加多少分。
    virtual int GetScore() const = 0;

    // 函数作用：返回这种食物能让蛇增长几节。
    virtual int GetGrowCount() const = 0;

    // 函数作用：绘制这种食物。
    virtual void Draw(Vector3 position) const = 0;
};

// 普通红色食物规则。
class NormalFoodRule : public FoodRule
{
public:
    int GetScore() const override;
    int GetGrowCount() const override;
    void Draw(Vector3 position) const override;
};

// 金色超级果子规则。
class SuperFoodRule : public FoodRule
{
public:
    int GetScore() const override;
    int GetGrowCount() const override;
    void Draw(Vector3 position) const override;
};

// 函数作用：根据 isSuperFood 返回对应的食物规则。
const FoodRule &GetFoodRule(bool isSuperFood);
