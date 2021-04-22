#include <rps/rps.hpp>
#include <rps/constants.hpp>

namespace proton
{
  uint64_t rps::create(name &host) {
      require_auth(host);
      // check(challenger != host, "Challenger should not be the same as the host.");
      // Check if game already exists

      auto itr = existing_games.find(host.value);
      check(itr == existing_games.end(), "Game already exists.");
      uint64_t index = existing_games.available_primary_key();
      existing_games.emplace(host, [&](auto &g) {
          g.index = index;
          g.host = host;
          g.resetGame();
      });

      return index;
  }

  void rps::restart(const uint64_t& game_id,const name &challenger, const name &host, const name &by)
  {
      check(has_auth(by), "Only " + by.to_string() + "can restart the game.");

      // Check if game exists
      auto itr = existing_games.find(host.value);
      // auto itr = existing_games.require_find(game_id, "Game not found.");

      check(itr != existing_games.end(), "Game does not exist.");

      // Check if this game belongs to the action sender
      check(by == itr->host || by == itr->challenger, "This is not your game.");

      check( itr->winner == none, "Can't close the game in the middle.");

      // Reset game
      existing_games.modify(itr, itr->host, [](auto &g) {
          g.resetGame();
      });

      deftx(5);
  }

  void rps::close(const uint64_t& game_id,const name &challenger, const name &host, const name &by)
  {
//      check(has_auth(host), "Only the host can close the game.");

//      require_auth(host);

      require_auth(get_self()); //only this contract possible to call

      // Check if game exists
      auto itr = existing_games.require_find(host.value, "Game not found.");

      // check(itr != existing_games.end(), "Game does not exist.");

      check(by == itr->host || by == itr->challenger, "This is not your game.");

      check_winner((*itr));

      check( itr->winner == none, "Can't close the game in the middle.");

      // Remove game
      existing_games.erase(itr);
  }


  void rps::join(const uint64_t& game_id, const name &challenger,const name &host){

    check(has_auth(challenger), "Only the challenger can join the game.");

//    require_auth(challenger);

    // Get match
    auto match_itr = existing_games.require_find(host.value, "Host Game not found.");

    auto itr = existing_games.find(challenger.value);

    check(itr == existing_games.end(), "You have already joined one game. Finish it");

    eosio::check(match_itr->challenger != none , "This match is full now");

    existing_games.modify(match_itr, match_itr->host, [&](auto& g) {
      g.challenger = challenger;
    });

    deftx(5);
  }

  std::string rps::getstate(
      const uint64_t& game_id,
      const name &host,
      const name &by){

      // auto itr = existing_games.find(game_id);

      auto itr = existing_games.require_find(host.value, "Host Game not found.");

      // if(itr == existing_games.end()){
      //   itr = existing_games.find(host.value);
      // }

      // check(itr != existing_games.end(), "Game does not exist.");

      check(by == itr->host || by == itr->challenger, "This is not your game.");

      std::string result = "";

      result = +"*id-" + std::to_string(itr->index);

      result = +"*R-" + std::to_string(itr->round_number);

      result = +"*WL-" + std::to_string(itr->challenger_win_count);

      result = +"*WH-" + std::to_string(itr->host_win_count) + "*";

      result = +"pay_status:";
      if(itr->challenger_available){
        result = +"-1";
      }else{
        result = +"-0";
      }

      if(itr->host_available){
        result = +"-1";
      }else{
        result = +"-0";
      }

      result = +"*choice_status:";
      if(itr->has_challenger_made_choice){
        result = +"-1";
      }else{
        result = +"-0";
      }

      if(itr->has_host_made_choice){
        result = +"-1";
      }else{
        result = +"-0";
      }

      result = +"*choice_string:";

      if(itr->challenger_choice != ""){
        result = +"-1";
      }else{
        result = +"-0";
      }


      if(itr->host_choice != ""){
        result = +"1";
      }else{
        result = +"0";
      }

      result = +"*game_result:";

//      result = + std::to_string(itr->winner.to_string());

      if(itr->winner != none){
        result = +"-1";
      }else{
        result = +"-0";
      }
      return result;
  }

  void rps::unlockchoice(
    const uint64_t& game_id,
    const name &player,
    const name &host,
    const uint8_t& round_number,
    const std::string& choice,
    const std::string& password){

    auto match_itr = existing_games.require_find(host.value, "Game does not exist.");

    // Check if this game hasn't ended yet
    check(match_itr->winner == none, "The game has ended.");

    eosio::check(match_itr->challenger.value == player.value || match_itr->host.value == player.value, "You are not playing this game.");

    eosio::check(match_itr->round_number == round_number , "You are not playing this round.");

    eosio::check( match_itr->has_challenger_made_choice &&  match_itr->has_host_made_choice , "Both player should select the choice");

    std::string data_to_hash = password + ":" + choice;

    eosio::checksum256 choice_digest = eosio::sha256(data_to_hash.data(), data_to_hash.size());


    if(match_itr->challenger.value == player.value){
      check(match_itr->challenger_available, "You didn't deposit anything.");
      check(match_itr->challenger_choice_hash == choice_digest , "Your choice is different from the original has that you sent");
    }

    if(match_itr->host.value == player.value){
      check(match_itr->host_available, "You didn't deposit anything.");
      check(match_itr->host_choice_hash == choice_digest , "Your choice is different from the original has that you sent.");
    }

    if(match_itr->challenger.value == player.value){
      existing_games.modify(match_itr,match_itr->host, [&](auto& g) {
        g.challenger_choice_hash = choice_digest;
        g.challenger_choice_password = password;
        g.challenger_choice = choice;
        g.winner = check_winner(g);
      });
    }else if(match_itr->host.value == player.value){
      existing_games.modify(match_itr, match_itr->host, [&](auto& g) {
        g.host_choice_hash =choice_digest ;
        g.host_choice_password = password;
        g.host_choice = choice;
        g.winner = check_winner(g);
      });
    }
  }

  void rps::makechoice(
      const uint64_t& game_id,
      const name & player,
      const name &host,
      const uint8_t& round_number,
      const eosio::checksum256& choice_digest){


      // check(has_auth(player), "The next move should be made by " + by.to_string());
          // Get match
      auto match_itr = existing_games.require_find(host.value, "Game does not exist.");

      // Check if this game hasn't ended yet
      check(match_itr->winner == none, "The game has ended.");

      eosio::check(match_itr->challenger.value == player.value || match_itr->host.value == player.value, "You are not playing this game.");

      eosio::check(match_itr->round_number == round_number , "You are not playing this round.");

      if(match_itr->challenger.value == player.value){
        check(match_itr->challenger_available, "You didn't deposit anything.");
        eosio::check( match_itr->has_challenger_made_choice == false , "You have already selected your choice.");
      }

      if(match_itr->host.value == player.value){
        check(match_itr->host_available, "You didn't deposit anything.");
        eosio::check( match_itr->has_host_made_choice == false , "You have already selected your choice.");
      }

      if(match_itr->challenger.value == player.value){
        existing_games.modify(match_itr,match_itr->host, [&](auto& g) {
          g.challenger_choice_hash = choice_digest;
          g.has_challenger_made_choice = true;
        });
      } else if(match_itr->host.value == player.value){
        existing_games.modify(match_itr, match_itr->host, [&](auto& g) {
          g.host_choice_hash = choice_digest ;
          g.has_host_made_choice = true;
        });
      }
  }


  void rps::checkround( const uint64_t& game_id,const name &challenger, const name &host){

//    check(has_auth(host), "Only the host can close the game.");

    eosio::print("checkround\n");
    

    require_auth(get_self()); //only this contract possible to call

    // Get match
    auto match_itr = existing_games.require_find(host.value, "Game does not exist.");

    if(match_itr->winner != none){
    // Remove game
      existing_games.erase(match_itr);
      return;
    }
    check(match_itr->winner == none, "This game is end.");

    check(match_itr->challenger.value == challenger.value && match_itr->host.value == host.value, "Different game selected.");

    check(match_itr->host_available && match_itr->challenger_available, "The player don't pay anything");

    auto current_time = eosio::current_time_point().sec_since_epoch();

    check( match_itr->start_at > 0 && current_time >= match_itr->start_at + gametimeout,"This game is not overtime yet" );

    if( match_itr->has_host_made_choice == false  && match_itr->has_challenger_made_choice  && match_itr->host_choice == ""  && match_itr->challenger_choice != "" ){

        std::string  choice =  random_choice(*match_itr);

        existing_games.modify(match_itr, match_itr->host, [&](auto& g) {
          g.host_choice = choice;
          g.has_challenger_made_choice = true;
          g.winner = check_winner(g);
        });


    } else if( match_itr->has_host_made_choice  && match_itr->has_challenger_made_choice == false  && match_itr->challenger_choice == "" && match_itr->host_choice != ""){

        std::string  choice =  random_choice(*match_itr);
        existing_games.modify(match_itr, match_itr->host, [&](auto& g) {
          g.challenger_choice = choice;
          g.has_host_made_choice = true;
          g.winner = check_winner(g);
        });

    } else {
     name winner = check_winner(*match_itr);
    }
  }


  void rps::startround( const uint64_t& game_id,const name &challenger, const name &host, const name &by){

    check(has_auth(by), "Only " + by.to_string() + "can restart the game.");

    require_auth(get_self()); //only this contract possible to call

    // Check if game exists
    // auto itr = existing_games.find(challenger.value);
    auto itr = existing_games.require_find(host.value, "Game not found.");

    check(itr != existing_games.end(), "Game does not exist.");

    // Check if this game belongs to the action sender
    check(by == itr->host || by == itr->challenger, "This is not your game.");

    eosio::check(itr->host_available && itr->challenger_available, "The player don't pay anything");

    existing_games.modify(itr, itr->host, [&](auto& g) {
      g.newRound();
    });
  }

  name rps::check_winner(const Game &current_game)
  {
      if( current_game.has_host_made_choice  && current_game.has_challenger_made_choice &&  current_game.challenger_choice != ""  && current_game.host_choice != "" ){

        string host_choice =    current_game.host_choice;
        string challenger_choice = current_game.challenger_choice;

        map<string ,map<string,uint8_t>> states = {
            { "R", {{"R",0},{"P",2},{"S",1}}},
            { "P",{{"P",0},{"R",1},{"S",2}}},
            { "S",{{"R",2},{"P",1},{"S",0}}}
          };

        eosio::print("check_winner\n");

        int result = states[host_choice][challenger_choice];

        eosio::print(result);

        if(result == 0){
          // game is tie

        }else if(result == 1){
          // host is win

              // Get match
          auto match_itr = existing_games.require_find(current_game.index, "Game does not exist.");

          existing_games.modify(match_itr, match_itr->host, [&](auto& g) {
            g.host_win_count += 1;
          });

        }else if(result == 2) {
          //challenge is win

            auto match_itr = existing_games.require_find(current_game.index, "Game does not exist.");

            existing_games.modify(match_itr, match_itr->host, [&](auto& g) {
              g.challenger_win_count += 1;
            });
        }

        uint64_t amount = (uint64_t)(( current_game.host_bet + current_game.challenger_bet)*0.99);

        asset a;
        a.set_amount(amount);

        extended_asset award_asset(a,SYSTEM_TOKEN_CONTRACT);

        eosio::print("deposit the winner result\n");

        if(current_game.host_win_count > 1){
          // unint

          transfer_to(current_game.host, award_asset, "winner prize");

          return current_game.host;
        }

        if(current_game.challenger_win_count > 1){


          transfer_to(current_game.challenger, award_asset, "winner prize");

          return current_game.challenger;
        }

        auto match_itr = existing_games.require_find(current_game.index, "Game does not exist.");

        existing_games.modify(match_itr, match_itr->host, [&](auto& g) {
                    g.newRound();
        });
      }
      // Draw if the board is full, otherwise the winner is not determined yet
      return  none;
   }

   void rps::ticker() {
      //only this contract can call this action
      require_auth(get_self());

      eosio::print("ticker\n");

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

      auto itr = existing_games.begin();
      uint32_t count = 0;
      eosio::print("checkgames\n");
      eosio::print(gametimeout);

      for (; itr != existing_games.end(); itr++) {
        auto gm = *itr;

        int timeout = eosio::current_time_point().sec_since_epoch() - gm.start_at;

        eosio::print("timeout\n");
        eosio::print(gm.start_at);
        eosio::print(timeout);

        if( gm.start_at != 0){
          if(timeout > gametimeout){
            checkround( gm.index, gm.challenger, gm.host);
          }else{
            count++;
          }
        }
      }
      return count;
    }

   std::string rps::random_choice(const Game &current_game){

      eosio::checksum256 result = eosio::sha256((char *)&current_game, sizeof(current_game)*2);

      int choice = ((result.get_array()[0] + result.get_array()[2]  + result.get_array()[0] )% 3);

      eosio::print("random_choice\n");
      eosio::print(choice);

      if(choice == 0){
        return "R";
      }else if(choice == 1){
        return "P";
      }else if(choice == 2){
        return "C";
      }
      return "R";
   }
} // namepsace contract
