#include <iostream>
#include <vector>
#include <tuple>

#include <gtest/gtest.h>


#define LOG_LEVEL_INFO

#include "matching_engine.hpp"

namespace matching_engine::smoke
{

enum class order_type : uint8_t
{
    LIM_BUY = 0,
    LIM_SELL = 1,
    CANCEL = 2,
};

static const std::vector<std::tuple<order_type, ticks, units>> orders =
{
    {order_type::LIM_BUY, 10, 100},
    {order_type::LIM_SELL, 20, 90}
}
;

TEST(SmokeTest, InsertAndCancelOneBUYOrder) 
{
    matching_engine me;

    order_id id = me.place_limit_buy(100, 1);
    EXPECT_NE(id, PLACE_ORDER_FAILED);
    EXPECT_EQ(cancel_status::SUCCESS, me.cancel(id));
}

TEST(SmokeTest, InsertAndCancelOneSELLOrder) 
{
    matching_engine me;

    order_id id = me.place_limit_sell(100, 1);
    EXPECT_NE(id, PLACE_ORDER_FAILED);
    EXPECT_EQ(cancel_status::SUCCESS, me.cancel(id));
}

TEST(SmokeTest, FillBestPreexistingSellPrice) 
{
    matching_engine me;

    order_id id0 = me.place_limit_sell(100, 1);
    EXPECT_NE(id0, PLACE_ORDER_FAILED);

    order_id id1 = me.place_limit_sell(80, 1);
    
    EXPECT_NE(id0, id1);

    order_id id2 = me.place_limit_buy(90, 50);
}


TEST(SmokeTest, FillMostRecentPreexistingSellAtSamePrice) 
{
    matching_engine me;

    order_id id0 = me.place_limit_sell(100, 1);
    EXPECT_NE(id0, PLACE_ORDER_FAILED);

    order_id id1 = me.place_limit_sell(100, 1);
    EXPECT_NE(id0, id1);

    order_id id2 = me.place_limit_buy(90, 50);
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
