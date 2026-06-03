#pragma once

#include "raylib.h"

// 面向对象食物规则：父类 + 子类 + 多态。

// 食物规则父类。
class FoodRule
{
public:
    virtual ~FoodRule() {}

    virtual int GetScore() const = 0;

    virtual int GetGrowCount() const = 0;

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

// 根据类型选择食物规则。
const FoodRule &GetFoodRule(bool isSuperFood);
