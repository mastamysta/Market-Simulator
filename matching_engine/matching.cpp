#include <iostream>
#include <vector>
#include <tuple>

using quid = double;
using units = uint32_t;
using order_id = uint32_t;

// Given a set of limit buy/sell orders, implement a singlethreaded matching algorithm to match orders. It must:
//  Match orders in O(1)
//  Insert orders in O(1)
//  Be extremely quick

class matcher
{
public:
    
    auto place_limit_buy(quid price, units size) -> order_id
    {
        return 0;
    }

    auto place_limit_sell(quid price, units size) -> order_id
    {
        return 0;
    }

    auto cancel(order_id id) -> void
    {
        // not implemented for now
    }

private:

    order_id next_id = 0;

};

enum class order_type : uint8_t
{
    LIM_BUY = 0,
    LIM_SELL = 1,
    CANCEL = 2,
};

static const std::vector<std::tuple<order_type, quid, units>> orders =
{
    {order_type::LIM_BUY, 10, 100},
    {order_type::LIM_SELL, 20, 90}
}
;

int main()
{
    std::cout << "Hello world.\n";

    matcher my_matcher;

    for (const auto& order: orders)
    {
        switch (std::get<0>(order))
        {
        case order_type::LIM_BUY:

            my_matcher.place_limit_buy(std::get<1>(order), std::get<2>(order));

            break;
        
        case order_type::LIM_SELL:

            my_matcher.place_limit_sell(std::get<1>(order), std::get<2>(order));

            break;

        default:
            break;
        }
    }


    return 0;
}
