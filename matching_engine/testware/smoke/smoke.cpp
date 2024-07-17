#include <iostream>
#include <vector>
#include <tuple>

#include <gtest/gtest.h>

#include "matching_engine.hpp"

namespace matching_engine::smoke
{

enum class order_type : uint8_t
{
    LIM_BUY = 0,
    LIM_SELL = 1,
    CANCEL = 2,
};

static const std::vector<std::tuple<order_type, pence, units>> orders =
{
    {order_type::LIM_BUY, 10, 100},
    {order_type::LIM_SELL, 20, 90}
}
;

TEST(SmokeTest, InsertAndCancelOneBUYOrder) 
{
    matching_engine me;

    order_id id = me.place_limit_buy(100, 105);
    EXPECT_NE(id, PLACE_ORDER_FAILED);
    EXPECT_EQ(cancel_status::SUCCESS, me.cancel(id));
}

TEST(SmokeTest, InsertAndCancelOneSELLOrder) 
{
    matching_engine me;

    order_id id = me.place_limit_sell(100, 105);
    EXPECT_NE(id, PLACE_ORDER_FAILED);
    EXPECT_EQ(cancel_status::SUCCESS, me.cancel(id));
}

TEST(SmokeTest, AttemptInsertOffTick) 
{
    matching_engine me;

    order_id id = me.place_limit_buy(100.000001, 105);
    EXPECT_EQ(id, PLACE_ORDER_FAILED);

    id = me.place_limit_sell(100.000001, 105);
    EXPECT_EQ(id, PLACE_ORDER_FAILED);
}

// int main()
// {
//     std::cout << "Hello world.\n";

//     matching_engine my_matcher;

//     for (const auto& order: orders)
//     {
//         switch (std::get<0>(order))
//         {
//         case order_type::LIM_BUY:

//             my_matcher.place_limit_buy(std::get<1>(order), std::get<2>(order));

//             break;
        
//         case order_type::LIM_SELL:

//             my_matcher.place_limit_sell(std::get<1>(order), std::get<2>(order));

//             break;

//         default:
//             break;
//         }
//     }


//     return 0;
// }

}
