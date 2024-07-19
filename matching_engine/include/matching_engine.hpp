#pragma once

#include <cmath>
#include <cstdint>
#include <vector>
#include <limits>
#include <map>
#include <set>

namespace matching_engine
{

static constexpr float tick_size = 0.05;

using pence = float; 
using units = uint16_t;
using order_id = uint64_t;
using time_priority = uint16_t;

enum class cancel_status : order_id
{
    SUCCESS = 0,
    ORDER_NOT_FOUND = 1
};

constexpr order_id
PLACE_ORDER_FAILED = UINT64_MAX,
PLACE_ORDER_FILLED_IMMEDIATELY = UINT64_MAX - 1;

struct order_data
{
    pence price;
    units size;
    time_priority priority;
};

int operator<=>(order_data lhs, order_data rhs)
{
    if (lhs.price == rhs.price && lhs.priority == rhs.priority)
        return 0; // Should never get here. Orders at same price should *always* get a different priority in time
    
    if (lhs.price < rhs.price || (lhs.price == rhs.price && lhs.priority < rhs.priority))
        return -1;
    
    return 1;
}

// Given a set of limit buy/sell orders, implement a singlethreaded matching algorithm to match orders. It must:
//  Match orders in O(1)
//  Insert orders in O(1)
//  Be extremely quick

class matching_engine
{
public:
    
    // Return UINT64_MAX for failure
    auto place_limit_buy(const pence price, const units size) -> order_id
    {
        if (std::fmod(price, 0.05))
            return PLACE_ORDER_FAILED;

        if (!fill_buy(price, size))
            return PLACE_ORDER_FILLED_IMMEDIATELY;

        order_data order = 
        {
            .price = price,
            .size = size,
            .priority = buy_price_count_table[price]
        };

        buy_price_count_table[price]++;

        return order_data_to_order_id(order);
    }

    // Return UINT64_MAX for failure
    auto place_limit_sell(const pence price, const units size) -> order_id
    {
        return PLACE_ORDER_FAILED;
    }

    auto cancel(const order_id id) -> cancel_status
    {
        return cancel_status::SUCCESS;
    }

private:

    // The system works by creating an (ordered) set of orders for each side of the market. A custom ordering is implemented
    // over the map - which enforces the price-time algorithm. A count of the number of orders at a given price-point is
    // maintained for each side of the market in *_price_count_table.
    // When attempting to fill an order, we need only inspect the .back() of the *_side() set to find the highest priority order
    // in the price-time ordering.
    // Order ID's are simply the raw order data serialized. We can hash this if we want to obfuscate later.
    std::map<float, time_priority> buy_price_count_table, sell_price_count_table;
    std::set<order_data> buy_side, sell_side;

    union float_uint32
    {
        float in;
        uint32_t out;
    };

    // Directly convert the data into an integer
    auto order_data_to_order_id(const order_data& order) -> order_id
    {
        // If these sizes aren't the same, this bitwise magic won't work!
        static_assert(sizeof(float) == sizeof(uint32_t));

        // Convert float to uint32_t so we can do bitmagic upon it.
        order_id price_bits = ((float_uint32)order.price).out << ((sizeof(order.size) + sizeof(order.priority)) * 8);
        order_id size_bits = (order.size << (sizeof(order.priority) * 8));
        order_id priority_bits = order.priority;


        // Convert float to uint32_t so we can do bitmagic upon it.
        order_id price_bits = ((float_uint32)order.price).out << ((sizeof(order.size) + sizeof(order.priority)) * 8);
        order_id size_bits = (order.size << (sizeof(order.priority) * 8));
        order_id priority_bits = order.priority;

        return 0 | price_bits | size_bits | priority_bits;
    }

    // Returns false if we succeed in immediately filling the order.
    auto fill_buy(const pence price, const units size) -> bool
    {
        

        return false;
    }

};

}
