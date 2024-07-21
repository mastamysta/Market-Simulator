#pragma once

#include <cmath>
#include <cstdint>
#include <vector>
#include <limits>
#include <map>
#include <set>
#include <array>
#include <concepts>
#include <functional>
#include <unordered_map>

#include "logging.hpp"

#define MODULE_NAME "MATCHING"

namespace matching_engine
{

static constexpr float tick_size = 0.05;

using ticks = uint32_t; 
using units = uint16_t;
using order_id = uint64_t;
using time_priority = uint16_t;
using participant_id = uint16_t;
using order_executed_callback = std::function<bool(participant_id, order_id, units)>;

static constexpr order_id INVALID_ID = UINT64_MAX - 2;
static constexpr order_id PLACE_ORDER_FAILED = UINT64_MAX - 1;
static constexpr order_id PLACE_ORDER_FILLED_IMMEDIATELY = UINT64_MAX;

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
    requires std::same_as<decltype(v.agent_id), participant_id>;
    requires std::same_as<decltype(v.id), order_id>;

    requires std::same_as<decltype(v.can_fill(p)), bool>;
};

struct order_data_sell
{
    ticks price;
    units size;
    time_priority priority;
    participant_id agent_id;
    order_id id;

    inline bool can_fill(const ticks buy_price) { return buy_price >= price;  }
};

struct order_data_buy
{
    ticks price;
    units size;
    time_priority priority;
    participant_id agent_id;
    order_id id;

    inline bool can_fill(const ticks sell_price) { return price >= sell_price;  }
};

int operator<=>(const order_data_sell& lhs, const order_data_sell& rhs)
{
    if (lhs.price == rhs.price && lhs.priority == rhs.priority)
        return 0; // We get here when attempting to delete an order. Priorities must be unique!

    if (lhs.price < rhs.price || (lhs.price == rhs.price && lhs.priority < rhs.priority))
        return -1;
    
    return 1;
}

int operator<=>(const order_data_buy& lhs, const order_data_buy& rhs)
{
    if (lhs.price == rhs.price && lhs.priority == rhs.priority)
        return 0; // We get here when attempting to delete an order. Priorities must be unique!
    
    if (lhs.price > rhs.price || (lhs.price == rhs.price && lhs.priority < rhs.priority))
        return -1;
    
    return 1;
}

template <IOrderData order_data>
struct market_side
{
    std::set<order_data> book = { };
    std::unordered_map<order_id, order_data> id_map = { };

    auto insert_order(const order_data& order)
    {
        book.insert(order);
        id_map[order.id] = order;
    }

    auto remove_order(const order_data& order) -> void
    {
        id_map.erase(order.id);
        book.erase(order);
    }
};

// Given a set of limit buy/sell orders, implement a singlethreaded matching algorithm to match orders. It must:
//  Match orders in O(1)
//  Insert orders in O(1)
//  Be extremely quick

class matching_engine
{
public:

    auto register_order_executed_callback(order_executed_callback cb) -> void
    {
        exec_cb = cb;
    }
    
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
            .priority = buy_price_count_table[price],
            .id = id_cnt
        };

        buy_side.insert_order(order);

        buy_price_count_table[price]++;
        id_cnt++;

        return order.id;
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
            .priority = sell_price_count_table[price],
            .id = id_cnt
        };

        sell_side.insert_order(order);

        sell_price_count_table[price]++;
        id_cnt++;

        return order.id;
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
    market_side<order_data_buy> buy_side;
    market_side<order_data_sell> sell_side;

    order_id id_cnt = 0;

    order_executed_callback exec_cb;

    // Notify a counterparty to a partial or complete sale of an order
    template <IOrderData order_data>
    auto notify(order_data order, units size) -> void
    {
        if (!exec_cb)
        {
            ERROR("Attempted to notify of order completion, but no notification callback has been registered.\n");
            return;
        }

        exec_cb(order.agent_id, order.id, size);
    }

    // Returns false if we succeed in immediately filling the order.
    template <IOrderData order_data>
    auto fill(market_side<order_data>& side, const ticks price, const units size, units& leftover_size) -> bool
    {
        if (!side.book.size())
            return true;

        // Get iterator and value of best
        auto best_it = side.book.begin();
        auto best = *best_it;
        leftover_size = size;

        while (best.can_fill(price))
        {
            INFO("Best price: %lu worst price: %lu.\n",
                            best.price,
                            (*(side.book.rbegin())).price);

            INFO("Best price: %lu best time: %hu second price: %lu second time: %hu\n", 
                                                        best.price, 
                                                        best.priority, 
                                                        (*(std::next(side.book.begin()))).price, 
                                                        (*(std::next(side.book.begin()))).priority);

            if (best.size < leftover_size)
            {
                notify(best, best.size);

                side.remove_order(best);
                leftover_size -= best.size;
            }
            else if (best.size > leftover_size)
            {
                notify(best, leftover_size);

                side.remove_order(best);
                best.size -= size;
                side.insert_order(best); // TODO: It would be good to ammend an order rather than doing a full removal and reinsert.
                return false; // No point reevaluating the conditions, we've successfully bought all we can!
            }
            else // best.size == leftover_size
            {
                notify(best, best.size);

                side.remove_order(best);
                return false;
            }

            best = *(side.book.begin());
        }

        return true;
    }
};

}
