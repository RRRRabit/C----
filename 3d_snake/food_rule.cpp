#include "food_rule.h"
#include "constants.h"

// 食物规则：分数、增长、绘制。

int NormalFoodRule::GetScore() const
{
    return SCORE_PER_NORMAL_FOOD;
}

int NormalFoodRule::GetGrowCount() const
{
    return NORMAL_FOOD_GROW;
}

void NormalFoodRule::Draw(Vector3 position) const
{
    DrawSphere(position, CUBE_SIZE * 0.40f, RED);
}

int SuperFoodRule::GetScore() const
{
    return SCORE_PER_SUPER_FOOD;
}

int SuperFoodRule::GetGrowCount() const
{
    return SUPER_FOOD_GROW;
}

void SuperFoodRule::Draw(Vector3 position) const
{
    DrawSphere(position, CUBE_SIZE * 0.52f, GOLD);
    DrawSphereWires(position, CUBE_SIZE * 0.64f, 10, 10, ORANGE);
}

// 根据类型选择食物规则。
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
