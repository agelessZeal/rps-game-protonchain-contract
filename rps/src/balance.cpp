#include <rps/rps.hpp>
#include <rps/constants.hpp>
namespace proton {
  void rps::add_balance (const name& account, const extended_asset& delta) {


    // Get match

      games existingHostGames(get_self(), get_self().value);
//      auto match_itr = existingHostGames.require_find(account.value,"Game does not exist.");

      auto match_itr = existingHostGames.begin();
      auto itr = existingHostGames.begin();

      for (; itr != existingHostGames.end(); itr++) {
          if(itr->winner == none){
            if(itr->host == account.value || itr->challenger == account.value){
                match_itr = itr;
                break;
            }
          }
      }

      check(match_itr != existingHostGames.begin(),'Your game is not found' )

//    auto match_itr = existing_games.require_find(account.value, "Game not found.");

    if(match_itr->host.value == account.value){
      if(match_itr->challenger_bet != 0){
        check(delta.quantity.amount >= match_itr->challenger_bet, "balance must be the same");
      }
      existingHostGames.modify(match_itr, match_itr->host, [&](auto& g) {
        g.host_bet = delta.quantity.amount;
        g.host_available = 1;

        if( g.challenger_available == 1 ){
          g.start_at = eosio::current_time_point().sec_since_epoch();
        }

      });
    } else if(match_itr->challenger.value == account.value){

      if(match_itr->host_bet != 0){
         check(delta.quantity.amount >= match_itr->host_bet, "balance must be the same");
      }

      existingHostGames.modify(match_itr, match_itr->host, [&](auto& g) {
        g.challenger_bet = delta.quantity.amount;
        g.challenger_available = 1;
        if( g.host_available == 1 ){
          g.start_at = eosio::current_time_point().sec_since_epoch();
        }
      });
    }
  }
}
