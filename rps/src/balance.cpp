#include <rps/rps.hpp>
#include <rps/constants.hpp>
#include <string>

namespace proton {
  void rps::add_balance (const name& account, const extended_asset& delta,const string& memo) {

      bool digit_check = is_digits(memo);
      check(digit_check,"The game id is wrong.");
      int index =  stoi(memo);

      rps_matches existingHostGames(get_self(), get_self().value);

      auto match_itr = existingHostGames.find(index);

      if(match_itr != existingHostGames.end() ){
          check( match_itr->winner == none,"This game is ended.");
          check( match_itr->challenger == none,"This game is full.");

          existingHostGames.modify(match_itr, get_self(), [&](auto &g) {
            g.challenger_bet = delta.quantity.amount;
            g.challenger = account;
          });

//          existing_games.modify(match_itr, get_self(), [&](auto &g) {
//            g.challenger_bet = delta.quantity.amount;
//            g.challenger = account;
//          });

      } else {
        existingHostGames.emplace(get_self(), [&](auto &g) {
            g.index = index;
            g.host = account;
            g.game_id = memo;
            g.challenger = none;
            g.winner = none;
            g.host_bet = delta.quantity.amount;
            g.challenger_bet = 0;
        });

//        existing_games.emplace(get_self(), [&](auto &g) {
//            g.index = index;
//            g.host = account;
//            g.game_id = memo;
//            g.challenger = none;
//            g.winner = none;
//            g.host_bet = delta.quantity.amount;
//            g.challenger_bet = 0;
//        });

      }
  }
}
