#include <rps/rps.hpp>
#include <rps/constants.hpp>

namespace proton
{
  void rps::create(name &host,name &challenger) {
//      require_auth(host);
      // check(challenger != host, "Challenger should not be the same as the host.");
      // Check if game already exists

      games existingHostGames(get_self(), host.value);

//      auto match_itr = existing_games.begin();
//
//      for (; match_itr != existing_games.end(); match_itr++) {
//         existing_games.erase(match_itr);
//      }

      auto itr = existingHostGames.find(host.value);
      check(itr == existingHostGames.end(), "Game already exists.");

      existingHostGames.emplace(host, [&](auto &g) {
          g.host = host;
          g.resetGame();
          g.challenger = challenger;
          g.created_at = current_time_point().sec_since_epoch();
      });
  }

  void rps::restart(const name &challenger, const name &host, const name &by)
  {
      check(has_auth(by), "Only " + by.to_string() + "can restart the game.");

      // Check if game exists
      // auto itr = existing_games.find(challenger.value);


     games existingHostGames(get_self(), host.value);
     auto itr = existingHostGames.find(host.value);

//      auto itr = existing_games.require_find(game_id, "Game not found.");

      check(itr != existingHostGames.end(), "Game does not exist.");

      // Check if this game belongs to the action sender
      check(by == itr->host || by == itr->challenger, "This is not your game.");

      check( itr->winner == none, "Can't close the game in the middle.");

      // Reset game
      existingHostGames.modify(itr, itr->host, [](auto &g) {
          g.resetGame();
      });

      deftx(5);
  }

  void rps::close(const name &challenger, const name &host, const name &by)
  {
//      check(has_auth(host), "Only the host can close the game.");

//      require_auth(host);

      require_auth(get_self()); //only this contract possible to call

      // Check if game exists

       games existingHostGames(get_self(), host.value);
       auto itr = existingHostGames.find(host.value);

//      auto itr = existing_games.require_find(game_id, "Game not found.");

      check(itr != existingHostGames.end(), "Game does not exist.");

      check(by == itr->host || by == itr->challenger, "This is not your game.");

      check_winner((*itr));

      check( itr->winner == none, "Can't close the game in the middle.");

      // Remove game
      existingHostGames.erase(itr);
  }

//   void rps::getstate( const name &host, const name &challenger){
//
//      games existingHostGames(get_self(), host.value);
//      auto itr = existingHostGames.find(host.value);
//      print('getstate');
//   }


//  void rps::join( const name &host, const name &challenger){
//
////    check(has_auth(challenger), "Only the host can join the game.");
//
////    require_auth(challenger);
//
//    // Get match
//
//    games existingHostGames(get_self(), host.value);
//    auto match_itr = existingHostGames.require_find(host.value,"Game not found.");
//
////    auto match_itr = existingHostGames.require_find(game_id, "Game not found.");
//
//    auto itr = existingHostGames.find(challenger.value);
//
//    check(itr == existingHostGames.end(), "You have already joined one game. Finish it");
//
//    check(match_itr->challenger == none , "This match is full now");
//
//    existingHostGames.modify(match_itr, match_itr->host, [&](auto& g) {
//      g.challenger = challenger;
//    });
//
//    deftx(5);
//  }

  void rps::unlockchoice(
    const name &player,
    const uint8_t& round_number,
    const std::string& choice,
    const std::string& password){


    games existingHostGames(get_self(), player.value);
    auto match_itr = existingHostGames.require_find(player.value,"Game does not exist.");

//    auto match_itr = existing_games.require_find(game_id, "Game does not exist.");

    // Check if this game hasn't ended yet
    check(match_itr->winner == none, "The game has ended.");

    check(match_itr->challenger.value == player.value || match_itr->host.value == player.value, "You are not playing this game.");

    check(match_itr->round_number == round_number , "You are not playing this round.");

    check( match_itr->has_challenger_made_choice &&  match_itr->has_host_made_choice , "Both player should select the choice");

    std::string data_to_hash = password + ":" + choice;

    checksum256 choice_digest = sha256(data_to_hash.data(), data_to_hash.size());

    std::string choice_string = to_hex(choice_digest);


    if(match_itr->challenger.value == player.value){
      check(match_itr->challenger_available, "You didn't deposit anything.");
      check(match_itr->challenger_choice_hash == choice_string , "Your choice is different from the original has that you sent");
    }

    if(match_itr->host.value == player.value){
      check(match_itr->host_available, "You didn't deposit anything.");
      check(match_itr->host_choice_hash == choice_string , "Your choice is different from the original has that you sent.");
    }

    if(match_itr->challenger.value == player.value){
      existingHostGames.modify(match_itr,match_itr->host, [&](auto& g) {
//        g.challenger_choice_hash = choice_digest;
        g.challenger_choice_password = password;
        g.challenger_choice = choice;
        g.winner = check_winner(g);
      });
    }else if(match_itr->host.value == player.value){
      existingHostGames.modify(match_itr, match_itr->host, [&](auto& g) {
//        g.host_choice_hash =choice_digest ;
        g.host_choice_password = password;
        g.host_choice = choice;
        g.winner = check_winner(g);
      });
    }
  }

  void rps::makechoice(
      const name & player,
      const uint8_t& round_number,
      const checksum256& choice_digest){


      // check(has_auth(player), "The next move should be made by " + by.to_string());
          // Get match

      games existingHostGames(get_self(), player.value);
      auto match_itr = existingHostGames.require_find(player.value,"Game does not exist.");
//      auto match_itr = existing_games.require_find(game_id, "Game does not exist.");

      // Check if this game hasn't ended yet
      check(match_itr->winner == none, "The game has ended.");

      check(match_itr->challenger.value == player.value || match_itr->host.value == player.value, "You are not playing this game.");

      check(match_itr->round_number == round_number , "You are not playing this round.");

      if(match_itr->challenger.value == player.value){
        check( match_itr->challenger_available, "You didn't deposit anything.");
        check( match_itr->has_challenger_made_choice == false , "You have already selected your choice.");
      }

      if(match_itr->host.value == player.value){
        check(match_itr->host_available, "You didn't deposit anything.");
        check( match_itr->has_host_made_choice == false , "You have already selected your choice.");
      }

      string choice_string =  to_hex(choice_digest);

      if(match_itr->challenger.value == player.value){
        existingHostGames.modify(match_itr,match_itr->host, [&](auto& g) {
          g.challenger_choice_hash = choice_string;
          g.has_challenger_made_choice = true;
        });
      } else if(match_itr->host.value == player.value){
        existingHostGames.modify(match_itr, match_itr->host, [&](auto& g) {
          g.host_choice_hash = choice_string ;
          g.has_host_made_choice = true;
        });
      }
  }


  void rps::checkround( const name &challenger, const name &host){

//    check(has_auth(host), "Only the host can close the game.");

     print("checkround\n");

//    require_auth(get_self()); //only this contract possible to call


      games existingHostGames(get_self(), host.value);
      auto match_itr = existingHostGames.require_find(host.value,"Game does not exist.");

    // Get match
//    auto match_itr = existing_games.require_find(game_id, "Game does not exist.");

    if(match_itr->winner != none){
    // Remove game
      existingHostGames.erase(match_itr);
      return;
    }
    check(match_itr->winner == none, "This game is end.");

    check(match_itr->challenger.value == challenger.value && match_itr->host.value == host.value, "Different game selected.");

    check(match_itr->host_available && match_itr->challenger_available, "The player don't pay anything");

    auto current_time = current_time_point().sec_since_epoch();

    check( match_itr->start_at > 0 && current_time >= match_itr->start_at + gametimeout,"This game is not overtime yet" );

    if( match_itr->has_host_made_choice == false  && match_itr->has_challenger_made_choice  && match_itr->host_choice == ""  && match_itr->challenger_choice != "" ){

        std::string  choice =  random_choice(*match_itr);

        existingHostGames.modify(match_itr, match_itr->host, [&](auto& g) {
          g.host_choice = choice;
          g.has_challenger_made_choice = true;
          g.winner = check_winner(g);
        });


    } else if( match_itr->has_host_made_choice  && match_itr->has_challenger_made_choice == false  && match_itr->challenger_choice == "" && match_itr->host_choice != ""){

        std::string  choice =  random_choice(*match_itr);
        existingHostGames.modify(match_itr, match_itr->host, [&](auto& g) {
          g.challenger_choice = choice;
          g.has_host_made_choice = true;
          g.winner = check_winner(g);
        });

    } else {
     name winner = check_winner(*match_itr);
    }
  }


  void rps::startround(const name &challenger, const name &host, const name &by){

    check(has_auth(by), "Only " + by.to_string() + "can restart the game.");

//    require_auth(get_self()); //only this contract possible to call

     games existingHostGames(get_self(), host.value);
     auto itr = existingHostGames.require_find(host.value,"Game does not exist.");

    // Check if game exists
    // auto itr = existing_games.find(challenger.value);
//    auto itr = existing_games.require_find(game_id, "Game not found.");

//    check(itr != existing_games.end(), "Game does not exist.");

    // Check if this game belongs to the action sender
    check(by == itr->host || by == itr->challenger, "This is not your game.");

    check(itr->host_available && itr->challenger_available, "The player don't pay anything");

    existingHostGames.modify(itr, itr->host, [&](auto& g) {
      g.newRound();
    });
  }

  name rps::check_winner(const game &current_game)
  {
      if( current_game.has_host_made_choice  && current_game.has_challenger_made_choice &&  current_game.challenger_choice != ""  && current_game.host_choice != "" ){

        string host_choice =    current_game.host_choice;
        string challenger_choice = current_game.challenger_choice;

        map<string ,map<string,uint8_t>> states = {
            { "R", {{"R",0},{"P",2},{"S",1}}},
            { "P",{{"P",0},{"R",1},{"S",2}}},
            { "S",{{"R",2},{"P",1},{"S",0}}}
          };

        print("check_winner\n");

        int result = states[host_choice][challenger_choice];

        print(result);

        if(result == 0){
          // game is tie

        }else if(result == 1){
          // host is win


           games existingHostGames(get_self(), current_game.host.value);
           auto match_itr = existingHostGames.require_find( current_game.host.value,"Game does not exist.");
              // Get match
//          auto match_itr = existing_games.require_find(current_game.index, "Game does not exist.");

          existingHostGames.modify(match_itr, match_itr->host, [&](auto& g) {
            g.host_win_count += 1;
          });

        }else if(result == 2) {
          //challenge is win

           games existingHostGames(get_self(), current_game.host.value);
           auto match_itr = existingHostGames.require_find( current_game.host.value,"Game does not exist.");

//            auto match_itr = existing_games.require_find(current_game.index, "Game does not exist.");

            existingHostGames.modify(match_itr, match_itr->host, [&](auto& g) {
              g.challenger_win_count += 1;
            });
        }

        uint64_t amount = (uint64_t)(( current_game.host_bet + current_game.challenger_bet)*0.99);

        asset a;
        a.set_amount(amount);

        extended_asset award_asset(a,SYSTEM_TOKEN_CONTRACT);

        print("deposit the winner result\n");

        if(current_game.host_win_count > 1){
          // unint

          transfer_to(current_game.host, award_asset, "winner prize");

          return current_game.host;
        }

        if(current_game.challenger_win_count > 1){


          transfer_to(current_game.challenger, award_asset, "winner prize");

          return current_game.challenger;
        }

        games existingHostGames(get_self(), current_game.host.value);
        auto match_itr = existingHostGames.require_find( current_game.host.value,"Game does not exist.");

//        auto match_itr = existing_games.require_find(current_game.index, "Game does not exist.");

        existingHostGames.modify(match_itr, match_itr->host, [&](auto& g) {
                    g.newRound();
        });
      }
      // Draw if the board is full, otherwise the winner is not determined yet
      return  none;
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

        int created_timeout = current_time_point().sec_since_epoch() - gm.created_at;

        if(created_timeout > 3600){
          existingHostGames.erase(itr);
        }

        print("timeout\n");
        print(gm.start_at);
        print(timeout);

        if( gm.start_at != 0){
          if(timeout > gametimeout){
            checkround( gm.challenger, gm.host);
          }else{
            count++;
          }
        }
      }
      return count;
    }

   std::string rps::random_choice(const game &current_game){

      checksum256 result = sha256((char *)&current_game, sizeof(current_game)*2);

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


//   checksum256 decode_checksum(string hex)
//   {
//       checksum256 buffer;
//
//       int index = 0;
//       while (index < hex.length()) {
//           char c = hex[index];
//
//           int value = 0;
//           if (c >= '0' && c <= '9')
//               value = (c - '0');
//           else if (c >= 'A' && c <= 'F')
//               value = (10 + (c - 'A'));
//           else if (c >= 'a' && c <= 'f')
//               value = (10 + (c - 'a'));
//
//           buffer.hash[(index / 2)] += value << (((index + 1) % 2) * 4);
//           index++;
//       }
//
//       return buffer;
//   }

//   /* Convert a SHA256 sum (given as a string) to checksum256 object */
//   bool hex2bin(const std::string& in, checksum256 &out)
//   {
//       if (in.size() != 64)
//           return false;
//
//       std::string hex{"0123456789abcdef"};
//       for (int i = 0; i < 32; i++) {
//           auto d1 = hex.find(in[2*i]);
//           auto d2 = hex.find(in[2*i+1]);
//           if (d1 == std::string::npos || d2 == std::string::npos)
//               return false;
//
//           // checksum256 is composed of little endian int128_t
//           reinterpret_cast<char *>(out.data())[i/16*16 + 15-(i%16)] = (d1 << 4)  + d2;
//       }
//       return true;
//   }

   //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
   //------------------------------
   //TOOLS for SHA256 convertation
   //----------------------------
//   string rps::SHA256toHEX(capi_checksum256 sha256) {
//   	return conv2HEX((char*)sha256.hash, sizeof(sha256.hash));
//   }
//
//   string rps::conv2HEX(char* hasha, uint32_t ssize) {
//   	std::string res;
//   	const char* hex = "0123456789abcdef";
//
//   	uint8_t* conv = (uint8_t*)hasha;
//   	for (uint32_t i = 0; i < ssize; ++i)
//   		(res += hex[(conv[i] >> 4)]) += hex[(conv[i] & 0x0f)];
//   	return res;
//   }
//
//   capi_checksum256 rps::HEX2SHA256(string hexstr) {
//   	check(hexstr.length() == 64, "invalid sha256");
//
//   	capi_checksum256 cs;
//   	convFromHEX(hexstr, (char*)cs.hash, sizeof(cs.hash));
//   	return cs;
//   }
//
//   uint8_t rps::convFromHEX(char ch) {
//   	if (ch >= '0' && ch <= '9') return ch - '0';
//   	if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
//   	if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
//
//   	check(false, "Wrong hex symbol");
//   	return 0;
//   }
//
//   size_t rps::convFromHEX(string hexstr, char* res, size_t res_len) {
//   	auto itr = hexstr.begin();
//
//   	uint8_t* rpos = (uint8_t*)res;
//   	uint8_t* rend = rpos + res_len;
//
//   	while (itr != hexstr.end() && rend != rpos) {
//   		*rpos = convFromHEX((char)(*itr)) << 4;
//   		++itr;
//   		if (itr != hexstr.end()) {
//   			*rpos |= convFromHEX((char)(*itr));
//   			++itr;
//   		}
//   		++rpos;
//   	}
//   	return rpos - (uint8_t*)res;
//   }

} // namepsace contract
