#pragma once

#include <cstdint>
#include <vector>
#include <limits>
#include <memory>
#include <map>

namespace matching_engine
{

static constexpr float tick_size = 0.05;

using pence = double; 
using units = uint32_t;
using order_id = uint64_t;

enum class cancel_status
{
    SUCCESS = 0,
    ORDER_NOT_FOUND = 1
};

constexpr order_id PLACE_ORDER_FAILED = UINT64_MAX;

struct order_data
{
    pence price;
    units size;
    order_id id;

    std::unique_ptr<order_data> next;
};

/// This structure holds the head and tail of a list of orders at the same price.
/// It only occupies keys of the price map at which there are open orders.
struct price_anchor
{
    std::unique_ptr<order_data> head, tail;
};

// Given a set of limit buy/sell orders, implement a singlethreaded matching algorithm to match orders. It must:
//  Match orders in O(1)
//  Insert orders in O(1)
//  Be extremely quick

class matching_engine
{
public:
    
    // Return UINT64_MAX for failure
    auto place_limit_buy(pence price, units size) -> order_id
    {
        return PLACE_ORDER_FAILED;
    }

    // Return UINT64_MAX for failure
    auto place_limit_sell(pence price, units size) -> order_id
    {
        return PLACE_ORDER_FAILED;
    }

    auto cancel(order_id id) -> cancel_status
    {
        return cancel_status::SUCCESS;
    }

private:

    order_id next_id = 0;

    std::map<pence, price_anchor> book;

};

}
