#include "food_rule.h"
#include "constants.h"

// food_rule.cpp 只负责食物自己的规则。
// 普通果子和金色果子的分数、增长、绘制都放在这里。

// 函数作用：返回普通食物的分数。
int NormalFoodRule::GetScore() const
{
    return SCORE_PER_NORMAL_FOOD;
}

// 函数作用：返回普通食物的增长节数。
int NormalFoodRule::GetGrowCount() const
{
    return NORMAL_FOOD_GROW;
}

// 函数作用：绘制普通红色食物。
void NormalFoodRule::Draw(Vector3 position) const
{
    DrawSphere(position, CUBE_SIZE * 0.40f, RED);
}

// 函数作用：返回金色超级果子的分数。
int SuperFoodRule::GetScore() const
{
    return SCORE_PER_SUPER_FOOD;
}

// 函数作用：返回金色超级果子的增长节数。
int SuperFoodRule::GetGrowCount() const
{
    return SUPER_FOOD_GROW;
}

// 函数作用：绘制金色超级果子。
void SuperFoodRule::Draw(Vector3 position) const
{
    DrawSphere(position, CUBE_SIZE * 0.52f, GOLD);
    DrawSphereWires(position, CUBE_SIZE * 0.64f, 10, 10, ORANGE);
}

// 函数作用：根据 isSuperFood 返回对应的食物规则。
const FoodRule &GetFoodRule(bool isSuperFood)
{
    static NormalFoodRule normalFoodRule;
    static SuperFoodRule superFoodRule;

    if (isSuperFood)
    {
        return superFoodRule;
    }

    return normalFoodRule;
}
