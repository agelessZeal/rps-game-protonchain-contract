#include <rps/rps.hpp>
#include <rps/constants.hpp>
#include <string>

namespace proton {
  void rps::add_balance (const name& account, const extended_asset& delta,const string& memo) {

      bool digit_check = is_digits(memo);
      check(digit_check,"The game id is wrong.");

//      uint64_t  index = _atoi64(memo.c_str());
//      uint64_t value;
//      stringstream  iss(memo);
//      iss >> value;
      int index =  stoi(memo);

    // Get match
//      name host = name(memo);

      games existingHostGames(get_self(), get_self().value);
//      auto match_itr = existingHostGames.require_find(account.value,"Game does not exist.");

//      auto match_itr = existingHostGames.begin();

      auto match_itr = existingHostGames.find(index);

      if(match_itr != existingHostGames.end() ){
          check( match_itr->winner == none,"This game is ended.");
          check( match_itr->challenger == none,"This game is full.");

          existingHostGames.modify(match_itr, get_self(), [&](auto &g) {
            g.challenger_bet = delta.quantity.amount;
            g.challenger = account;
          });

//          if(match_itr->winner != none  && match_itr->challenger == none){
//               existingHostGames.modify(match_itr, get_self(), [&](auto &g) {
//                  g.challenger_bet = delta.quantity.amount;
//                  g.challenger = account;
//               });
//          }

      } else {
        existingHostGames.emplace(get_self(), [&](auto &g) {
            g.index = index;
            g.host = account;
            g.game_id = memo;
            g.challenger = none;
            g.winner = none;
            g.host_bet = delta.quantity.amount;
        });
      }

//
//       if(match_itr == existingHostGames.end()){
//
////         auto idx = existingHostGames.get_index<"secid"_n>();
////         // iterate through secondary index
////         for ( auto itr = idx.begin(); itr != idx.end(); itr++ ) {
////           // print each row's values
//// //          eosio::print_f("Game Table : {%, %, %}\n", itr->host, itr->challenger, itr->winner);
////
////             if(itr->winner == none  && itr->challenger == account){
////                 match_itr = existingHostGames.find(itr->host.value);
////                 break;
////             }
////         }
//       }else{
//
//
//       }
//
//      check(match_itr != existingHostGames.end(),"Your game is not found");
//
//  //    auto match_itr = existing_games.require_find(account.value, "Game not found.");
//
//      if(match_itr->host.value == account.value){
//        if(match_itr->challenger_bet != 0){
//          check(delta.quantity.amount >= match_itr->challenger_bet, "Your host balance should not less than other's");
//        }
//        existingHostGames.modify(match_itr, get_self(), [&](auto& g) {
//          g.host_bet = delta.quantity.amount;
//          g.host_available = 1;
//
//          if( g.challenger_available == 1 ){
//            g.start_at = eosio::current_time_point().sec_since_epoch();
//          }
//        });
//      } else if(match_itr->challenger.value == account.value){
//
//          if(match_itr->host_bet != 0){
//           check(delta.quantity.amount >= match_itr->host_bet, "Your challenger balance should not less than other's");
//          }
//
//          existingHostGames.modify(match_itr, get_self(), [&](auto& g) {
//            g.challenger_bet = delta.quantity.amount;
//            g.challenger_available = 1;
//            if( g.host_available == 1 ){
//              g.start_at = eosio::current_time_point().sec_since_epoch();
//            }
//          });
//      }
  }
}
