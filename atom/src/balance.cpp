#include <atom/atom.hpp>

namespace proton {
  void rps::add_balance (const name& account, const extended_asset& delta) {


    // Get match
    auto match_itr = existingHostGames.require_find(account.value, "Game not found.");

    if(match_itr->host === account){
      existingHostGames.modify(match_itr, get_self(), [&](auto& g) {
        g.price = delta.quantity.amount;
        g.host_available = true;
        g.starts_at = eosio::current_time_point().sec_since_epoch();
      });
    }
    else if(match_itr->challenger === account){

      check(delta.quantity.amount >= match_itr->price, "balance must be the same");

      existingHostGames.modify(match_itr, get_self(), [&](auto& g) {
        g.price = delta.quantity.amount;
        g.challenger_available = true;
        g.starts_at = eosio::current_time_point().sec_since_epoch();
      });
    }
  }
}