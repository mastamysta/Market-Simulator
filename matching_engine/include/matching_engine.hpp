#pragma once

#include <cmath>
#include <cstdint>
#include <vector>
#include <limits>
#include <map>
#include <set>
#include <array>
#include <concepts>

#include <iostream>

#include "logging.hpp"

#define MODULE_NAME "MATCHING"

namespace matching_engine
{

static constexpr float tick_size = 0.05;

using ticks = uint32_t; 
using units = uint16_t;
using order_id = std::array<char, 128>;
using time_priority = uint16_t;

static constexpr order_id PLACE_ORDER_FAILED = []()
{
    order_id ret = { 0 };
    ret[127] = 0xFF;
    return ret;
}();

static constexpr order_id PLACE_ORDER_FILLED_IMMEDIATELY = []()
{
    order_id ret = { 0 };
    ret[127] = 0xEE;
    return ret;
}();

enum class cancel_status : uint32_t
{
    SUCCESS = 0,
    ORDER_NOT_FOUND = 1
};

template <typename T>
concept IOrderData = requires(T v, ticks p)
{
    requires std::same_as<decltype(v.price), ticks>;
    requires std::same_as<decltype(v.size), units>;
    requires std::same_as<decltype(v.priority), time_priority>;

    requires std::same_as<decltype(v.can_fill(p)), bool>;
};

struct order_data_sell
{
    ticks price;
    units size;
    time_priority priority;

    inline bool can_fill(const ticks buy_price) { return buy_price >= price;  }
};

struct order_data_buy
{
    ticks price;
    units size;
    time_priority priority;

    inline bool can_fill(const ticks sell_price) { return price >= sell_price;  }
};

int operator<=>(const order_data_sell& lhs, const order_data_sell& rhs)
{
    if (lhs.price == rhs.price && lhs.priority == rhs.priority)
    {
        ERROR("Two sell orders' could not be sorted.\n");
        return 0; // Should never get here. Orders at same price should *always* get a different priority in time
    }

    if (lhs.price < rhs.price || (lhs.price == rhs.price && lhs.priority < rhs.priority))
        return -1;
    
    return 1;
}

int operator<=>(const order_data_buy& lhs, const order_data_buy& rhs)
{
    if (lhs.price == rhs.price && lhs.priority == rhs.priority)
    {
        ERROR("Two buy orders' could not be sorted.\n");
        return 0; // Should never get here. Orders at same price should *always* get a different priority in time
    }
    
    if (lhs.price > rhs.price || (lhs.price == rhs.price && lhs.priority < rhs.priority))
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
    auto place_limit_buy(const ticks price, const units size) -> order_id
    {
        if (!size)
            return PLACE_ORDER_FAILED;

        units leftover_size;

        if (!fill(sell_side, price, size, leftover_size))
            return PLACE_ORDER_FILLED_IMMEDIATELY;

        order_data_buy order = 
        {
            .price = price,
            .size = leftover_size,
            .priority = buy_price_count_table[price]
        };

        buy_price_count_table[price]++;

        buy_side.insert(order);        

        return order_data_to_order_id(order);
    }

    // Return UINT64_MAX for failure
    auto place_limit_sell(const ticks price, const units size) -> order_id
    {
        if (!size)
            return PLACE_ORDER_FAILED;
        
        units leftover_size;

        if (!fill(buy_side, price, size, leftover_size))
            return PLACE_ORDER_FILLED_IMMEDIATELY;

        order_data_sell order = 
        {
            .price = price,
            .size = leftover_size,
            .priority = sell_price_count_table[price]
        };

        sell_price_count_table[price]++;

        sell_side.insert(order);

        return order_data_to_order_id(order);
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
    std::set<order_data_buy> buy_side;
    std::set<order_data_sell> sell_side;

    template <IOrderData order_data>
    union order_u
    {
        order_data data;
        order_id id;
    };

    // Directly convert the data into an integer
    template <IOrderData order_data>
    auto order_data_to_order_id(const order_data& order) -> order_id
    {
        order_u o = { .data = order };
        return o.id;
    }

    // Directly convert order id to order data
    // template <IOrderData order_data>
    // auto order_id_to_order_data(const order_id id) -> order_data
    // {
    //     order_u o = { .id = id };
    //     return o.data;
    // }

    // Returns false if we succeed in immediately filling the order.
    template <IOrderData order_data>
    auto fill(std::set<order_data>& side, const ticks price, const units size, units& leftover_size) -> bool
    {
        if (!side.size())
            return true;

        order_data best;
        leftover_size = size;

        while (best.can_fill(price))
        {
            best = *(side.begin());

            INFO("Best price: %lu worst price: %lu.\n",
                            best.price,
                            (*(side.rbegin())).price);

            INFO("Best price: %lu best time: %hu second price: %lu second time: %hu", 
                                                        best.price, 
                                                        best.priority, 
                                                        (*(std::next(side.begin()))).price, 
                                                        (*(std::next(side.begin()))).priority);

            if (best.size < leftover_size)
            {
                // TODO: Notify the seller of full sale
                side.erase(side.begin());
                leftover_size -= best.size;
            }
            else if (best.size > leftover_size)
            {
                // TODO: Notify seller of partial sale? How does this work IRL?
                side.erase(side.begin());
                best.size -= size;
                side.insert(best); // TODO: New order needs new ID, and we need to supply that back to the seller - or map old ID to new ID.
                return false; // No point reevaluating the conditions, we've successfully bought all we can!
            }
            else // best.size == leftover_size
            {
                // TODO: Notify the seller of full sale
                side.erase(side.begin());
                return false;
            }
        }

        return true;
    }
};

}
