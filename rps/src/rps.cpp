#include <rps/rps.hpp>
#include <rps/constants.hpp>

namespace proton
{
  void rps::create(name &host,name &challenger) {
//      require_auth(host);
      // check(challenger != host, "Challenger should not be the same as the host.");
      // Check if game already exists

      games existingHostGames(get_self(), get_self().value);

//      auto match_itr = existing_games.begin();
//
//      for (; match_itr != existing_games.end(); match_itr++) {
//         existing_games.erase(match_itr);
//      }

      auto itr = existingHostGames.find(host.value);
      if(itr != existingHostGames.end() && itr->winner != none  && itr->challenger == challenger){
         existingHostGames.modify(itr, itr->host, [](auto &g) {
            g.resetGame();
         });
      }else{
        check(itr == existingHostGames.end(), "Game already exists.");
        existingHostGames.emplace(host, [&](auto &g) {
            g.index = existingHostGames.available_primary_key();
            g.host = host;
            g.resetGame();
            g.challenger = challenger;
            g.created_at = current_time_point().sec_since_epoch();
        });
      }
  }

  void rps::restart(const name &challenger, const name &host, const name &by)
  {
      check(has_auth(by), "Only " + by.to_string() + "can restart the game.");

      // Check if game exists
      // auto itr = existing_games.find(challenger.value);

      games existingHostGames(get_self(), get_self().value);

      auto itr = existingHostGames.find(host.value);

//      auto itr = existing_games.require_find(game_id, "Game not found.");

      check(itr != existingHostGames.end(), "Game does not exist.");

      // Check if this game belongs to the action sender
      check(by == itr->host || by == itr->challenger, "This is not your game.");

      check( itr->winner != none, "Can't restart the game in the middle.");

      // Reset game
      existingHostGames.modify(itr, by, [](auto &g) {
          g.resetGame();
      });

      deftx(5);
  }


//  void rps::close(const name &challenger, const name &host, const name &by)
//  {
////      check(has_auth(host), "Only the host can close the game.");
//
////      require_auth(host);
//
//      require_auth(get_self()); //only this contract possible to call
//
//      // Check if game exists
//
//       games existingHostGames(get_self(), get_self().value);
//       auto itr = existingHostGames.find(host.value);
//
////      auto itr = existing_games.require_find(game_id, "Game not found.");
//
//      check(itr != existingHostGames.end(), "Game does not exist.");
//
//      check(by == itr->host || by == itr->challenger, "This is not your game.");
//
//      check_winner((*itr));
//
//      check( itr->winner == none, "Can't close the game in the middle.");
//
//      // Remove game
//      existingHostGames.erase(itr);
//  }

  void rps::unlockchoice(
    const name &player,
    const name &host,
    const name &challenger,
    const uint8_t& round_number,
    const std::string& choice,
    const std::string& password){

    games existingHostGames(get_self(), get_self().value);
    auto match_itr = existingHostGames.require_find(host.value,"Game does not exist.");

//    auto match_itr = existing_games.require_find(game_id, "Game does not exist.");

    // Check if this game hasn't ended yet
    check(match_itr->winner == none, "The game has ended.");

    check(match_itr->challenger.value == player.value || match_itr->host.value == player.value, "You are not playing this game.");

//    check(match_itr->round_number == round_number , "You are not playing this round.");

    check( match_itr->has_challenger_made_choice == 1 &&  match_itr->has_host_made_choice == 1 , "Both players should select the choice");

    std::string data_to_hash = password + ":" + choice;

    checksum256 choice_digest = eosio::sha256(data_to_hash.data(), data_to_hash.size());

//    std::string choice_string = to_hex(choice_digest);

    if(match_itr->challenger.value == player.value){
      check(match_itr->challenger_available == 1, "You didn't deposit anything.");
      check(match_itr->challenger_choice_hash == choice_digest , "Your choice is different from the original hash that you sent");
    }

    if(match_itr->host.value == player.value){
      check(match_itr->host_available == 1, "You didn't deposit anything.");
      check(match_itr->host_choice_hash == choice_digest , "Your choice is different from the original hash that you sent.");
    }

    existingHostGames.modify(match_itr,player, [&](auto& g) {

        if(match_itr->challenger.value == player.value){
            g.challenger_choice_password = password;
            g.challenger_choice = choice;

        } else if(match_itr->host.value == player.value){
            g.host_choice_password = password;
            g.host_choice = choice;
        }

        if( g.has_host_made_choice  == 1 && g.has_challenger_made_choice == 1 &&  g.challenger_choice != ""  && g.host_choice != "" ){

          string host_choice =    g.host_choice;
          string challenger_choice = g.challenger_choice;

          map<string ,map<string,uint8_t>> states = {
              { "R", {{"R",0},{"P",2},{"S",1}}},
              { "P",{{"P",0},{"R",1},{"S",2}}},
              { "S",{{"R",2},{"P",1},{"S",0}}}
            };

          int result = states[host_choice][challenger_choice];

          print(result);

          if(result == 0){
            // game is tie
          }else if(result == 1){
            // host is win
            g.host_win_count += 1;
          }else if(result == 2) {
            //challenge is win
            g.challenger_win_count += 1;
          }

          if(g.host_win_count > 1){
            g.winner = g.host;
            send_balance(g.host,g);
          }
          if(g.challenger_win_count > 1){
            g.winner = g.challenger;
            send_balance(g.challenger,g);
          }
        }
    });

  }

  void rps::makechoice(
      const name & player,
      const name &host,
      const name &challenger,
      const uint8_t& round_number,
      const checksum256& choice_digest){

      // check(has_auth(player), "The next move should be made by " + by.to_string());
          // Get match

      games existingHostGames(get_self(), get_self().value);
      auto match_itr = existingHostGames.require_find(host.value,"Game does not exist.");
//      auto match_itr = existing_games.require_find(game_id, "Game does not exist.");

      // Check if this game hasn't ended yet
      check(match_itr->winner == none, "The game has ended.");

      check(match_itr->challenger.value == player.value || match_itr->host.value == player.value, "You are not playing this game.");

//      check(match_itr->round_number == round_number , "You are not playing this round.");

      if(match_itr->challenger.value == player.value){
        check( match_itr->challenger_available == 1, "You didn't deposit anything.");
        check( match_itr->has_challenger_made_choice == 0 , "You have already selected your choice.");
      }

      if(match_itr->host.value == player.value){
        check(match_itr->host_available == 1, "You didn't deposit anything.");
        check( match_itr->has_host_made_choice == 0 , "You have already selected your choice.");
      }

//      string choice_string =  to_hex(choice_digest);

      if(match_itr->challenger.value == player.value){
        existingHostGames.modify(match_itr, player, [&](auto& g) {
          g.challenger_choice_hash = choice_digest;
          g.has_challenger_made_choice = 1;
        });
      } else if(match_itr->host.value == player.value){
        existingHostGames.modify(match_itr, player, [&](auto& g) {
          g.host_choice_hash = choice_digest ;
          g.has_host_made_choice = 1;
        });
      }
  }


  void rps::checkround( const name &challenger, const name &host){

//    check(has_auth(host), "Only the host can close the game.");

     print("checkround\n");

//    require_auth(get_self()); //only this contract possible to call

    games existingHostGames(get_self(), get_self().value);
    auto match_itr = existingHostGames.require_find(host.value,"Game does not exist.");

    if(match_itr->winner != none){
        // Remove game
         existingHostGames.erase(match_itr);
         check(false, "This game has winner and remove it in table.");
         return;
    }

    check(match_itr->winner == none, "This game is not end.");

    check(match_itr->challenger.value == challenger.value && match_itr->host.value == host.value, "Different game selected.");

    check(match_itr->host_available == 1 , "The host doesn't deposit anything");

    check(match_itr->challenger_available == 1, "The challenger doesn't deposit anything");

    auto current_time = current_time_point().sec_since_epoch();

    check( match_itr->start_at > 0 && current_time >= match_itr->start_at + gametimeout,"This game is not overtime yet" );

    check( match_itr->has_host_made_choice + match_itr->has_challenger_made_choice > 0,"Two players don't choose the move at all." );

    std::string  choice =  random_choice(*match_itr);

    existingHostGames.modify(match_itr, _self, [&](auto& g) {

        if( match_itr->has_host_made_choice == 0  && match_itr->has_challenger_made_choice  == 1 && match_itr->host_choice == ""  && match_itr->challenger_choice != "" ){
          g.host_choice = choice;
          g.has_host_made_choice = 1;
        } else if(match_itr->has_host_made_choice == 1 && match_itr->has_challenger_made_choice == 0  && match_itr->challenger_choice == "" && match_itr->host_choice != ""){
          g.challenger_choice = choice;
          g.has_challenger_made_choice = 1;
        }

        if( g.has_host_made_choice  == 1 && g.has_challenger_made_choice == 1 &&  g.challenger_choice != ""  && g.host_choice != "" ){

          string host_choice =    g.host_choice;
          string challenger_choice = g.challenger_choice;

          map<string ,map<string,uint8_t>> states = {
              { "R", {{"R",0},{"P",2},{"S",1}}},
              { "P",{{"P",0},{"R",1},{"S",2}}},
              { "S",{{"R",2},{"P",1},{"S",0}}}
            };

          int result = states[host_choice][challenger_choice];

          print(result);

          if(result == 0){
            // game is tie
          }else if(result == 1){
            // host is win
            g.host_win_count += 1;
          }else if(result == 2) {
            //challenge is win
            g.challenger_win_count += 1;
          }

          if(g.host_win_count > 1){
            g.winner = g.host;
            send_balance(g.host,g);
          }
          if(g.challenger_win_count > 1){
            g.winner = g.challenger;
            send_balance(g.challenger,g);
          }
        }
    });
  }


  void rps::startround(const name &challenger, const name &host, const name &by){

//     check(has_auth(by), "Only " + by.to_string() + "can start round.");

//    require_auth(get_self()); //only this contract possible to call

     games existingHostGames(get_self(), get_self().value);

     auto itr = existingHostGames.require_find(host.value,"Game does not exist.");

     if(itr->winner != none){
     // Remove game
       existingHostGames.erase(itr);
       return;
     }

     check(itr->winner == none, "This game is end.");

     // Check if this game belongs to the action sender
     check(by == itr->host || by == itr->challenger, "This is not your game.");

     check(itr->host_available == 1, "The host doesn't deposit anything");

     check(itr->challenger_available == 1, "The challenger doesn't deposit anything");

     check(itr->host_choice != "" && itr->challenger_choice != "","This current round is not ended");

     existingHostGames.modify(itr, by, [&](auto& g) {
       g.newRound();
     });
  }

   void rps::send_balance(const name & winner,const game &current_game){

        uint64_t amount = (uint64_t)(( current_game.host_bet + current_game.challenger_bet)*0.98);

        asset a;
        a.set_amount(amount);

        extended_asset award_asset(a,SYSTEM_TOKEN_CONTRACT);

        transfer_to(winner, award_asset, "winner prize");

        print("deposit the winner result\n");

   }

   void rps::ticker() {
      //only this contract can call this action
//      require_auth(get_self());

      print("ticker\n");

      //check games timeouts
      uint32_t games_count = checkgames();
      if (games_count > 0){
          deftx(0);
        }
     }


    void rps::deftx(uint64_t delay ) {

      if (delay<1)
        delay = gametimeout;
      //send new one


      auto current_time = current_time_point().sec_since_epoch();

      transaction tickerTX{};
      tickerTX.actions.emplace_back( action({get_self(), "active"_n}, _self, "ticker"_n, current_time) );
      tickerTX.delay_sec = delay;
      tickerTX.expiration = time_point_sec(current_time + 5);
      uint128_t sender_id = (uint128_t(current_time) << 64) | current_time -5;
      tickerTX.send(sender_id, _self);
    }


    uint32_t rps::checkgames(){

      games existingHostGames(get_self(), get_self().value);

      auto itr = existingHostGames.begin();
      uint32_t count = 0;
      print("checkgames\n");
      print(gametimeout);

      for (; itr != existingHostGames.end(); itr++) {
        auto gm = *itr;

        int timeout = current_time_point().sec_since_epoch() - gm.start_at;
        auto current_time = eosio::current_time_point().sec_since_epoch();

        int created_timeout = current_time - gm.created_at;

        if(created_timeout > 3600){
          existingHostGames.erase(itr);
        }

        print("timeout\n");
        print(gm.start_at);
        print(timeout);

        if( gm.start_at != 0){
          if(timeout > gametimeout){
            checkround( gm.challenger, gm.host);
          } else {
            count++;
          }
        }
      }
      return count;
    }

   std::string rps::random_choice(const game &current_game){

      checksum256 result = eosio::sha256((char *)&current_game, sizeof(current_game)*2);

      int choice = ((result.get_array()[0] + result.get_array()[2]  + result.get_array()[0] )% 3);

      print("random_choice\n");
      print(choice);

      if(choice == 0){
        return "R";
      }else if(choice == 1){
        return "P";
      }else if(choice == 2){
        return "C";
      }
      return "R";
   }

   std::string rps::to_hex(const checksum256 &hashed) {
   		// Construct variables
   		string result;
   		const char *hex_chars = "0123456789abcdef";
   		const auto bytes = hashed.extract_as_byte_array();
   		// Iterate hash and build result
   		for (uint32_t i = 0; i < bytes.size(); ++i) {
   			(result += hex_chars[(bytes.at(i) >> 4)]) += hex_chars[(bytes.at(i) & 0x0f)];
   		}
   		// Return string
   		return result;
   }

} // namepsace contract
