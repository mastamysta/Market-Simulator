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

    auto id = me.place_limit_buy(100, 1);
    EXPECT_NE(id, PLACE_ORDER_FAILED);
    EXPECT_EQ(cancel_status::SUCCESS, me.cancel(id));
}

TEST(SmokeTest, InsertAndCancelOneSELLOrder) 
{
    matching_engine me;

    auto id = me.place_limit_sell(100, 1);
    EXPECT_NE(id, PLACE_ORDER_FAILED);
    EXPECT_EQ(cancel_status::SUCCESS, me.cancel(id));
}

TEST(SmokeTest, BuyOrderIDNotEqual)
{
    matching_engine me;

    auto id0 = me.place_limit_buy(100, 1);
    auto id1 = me.place_limit_buy(100, 1);
    EXPECT_NE(id0, id1);

    EXPECT_EQ(cancel_status::SUCCESS, me.cancel(id0));
    EXPECT_EQ(cancel_status::SUCCESS, me.cancel(id1));
    auto id2 = me.place_limit_buy(100, 1);

    // New orders cannot have the same ID as old cancelled orders.
    EXPECT_NE(id2, id0);
    EXPECT_NE(id2, id1);
}

TEST(SmokeTest, SellOrderIDNotEqual)
{
    matching_engine me;

    auto id0 = me.place_limit_sell(100, 1);
    auto id1 = me.place_limit_sell(100, 1);
    EXPECT_NE(id0, id1);

    EXPECT_EQ(cancel_status::SUCCESS, me.cancel(id0));
    EXPECT_EQ(cancel_status::SUCCESS, me.cancel(id1));
    auto id2 = me.place_limit_sell(100, 1);

    // New orders cannot have the same ID as old cancelled orders.
    EXPECT_NE(id2, id0);
    EXPECT_NE(id2, id1);
}

TEST(SmokeTest, FillBestPreexistingSellPrice) 
{
    matching_engine me;

    auto id1 = INVALID_ID;
    auto was_notified = false;

    auto notify = [&](participant_id pid, order_id id, units size)
    {
        EXPECT_EQ(id1, id);
        was_notified = true;
        return false;
    };

    me.register_order_executed_callback(notify);

    auto id0 = me.place_limit_sell(100, 1);
    id1 = me.place_limit_sell(80, 1);

    EXPECT_EQ(me.place_limit_buy(90, 50), PLACE_ORDER_FILLED_IMMEDIATELY);
    EXPECT_EQ(was_notified, true);
}


TEST(SmokeTest, FillEarliestPreexistingSellAtSamePrice) 
{
    matching_engine me;

    auto id0 = INVALID_ID;
    auto was_notified = false;

    auto notify = [&](participant_id pid, order_id id, units size)
    {
        EXPECT_EQ(id0, id);
        was_notified = true;
        return false;
    };

    me.register_order_executed_callback(notify);

    id0 = me.place_limit_sell(100, 1);
    auto id1 = me.place_limit_sell(100, 1);

    auto id2 = me.place_limit_buy(100, 1);
    EXPECT_EQ(was_notified, true);
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
