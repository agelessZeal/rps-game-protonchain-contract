#include <rps/rps.hpp>

namespace proton
{
  void rps::create(const name &challenger, name &host) {
      require_auth(host);
      check(challenger != host, "Challenger should not be the same as the host.");

      // Check if game already exists

      auto itr = existingHostGames.find(host.value);
      check(itr == existingHostGames.end(), "Game already exists.");

      existingHostGames.emplace(host, [&](auto &g) {
          g.index = existingHostGames.available_primary_key();
          g.challenger = challenger;
          g.host = host;
          g.resetGame();
      });
  }

  void rps::restart(const name &challenger, const name &host, const name &by)
  {
      check(has_auth(by), "Only " + by.to_string() + "can restart the game.");

      // Check if game exists
      auto itr = existingHostGames.find(challenger.value);
      check(itr != existingHostGames.end(), "Game does not exist.");

      // Check if this game belongs to the action sender
      check(by == itr->host || by == itr->challenger, "This is not your game.");

      // Reset game
      existingHostGames.modify(itr, itr->host, [](auto &g) {
          g.resetGame();
      });
  }

  void rps::close(const name &challenger, const name &host)
  {
      check(has_auth(host), "Only the host can close the game.");

      require_auth(host);

      // Check if game exists
      auto itr = existingHostGames.find(challenger.value);
      check(itr != existingHostGames.end(), "Game does not exist.");

      // Remove game
      existingHostGames.erase(itr);
  }

  bool rps::isEmptyCell(const uint8_t &cell)
  {
      return cell == 0;
  }

  bool rps::isValidMove(const uint16_t &row, const uint16_t &column, const std::vector<uint8_t> &board)
  {
      uint32_t movementLocation = row * game::boardWidth + column;
      bool isValid = movementLocation < board.size() && isEmptyCell(board[movementLocation]);
      return isValid;
  }

  void join(const uint64_t& game_id, const name &challenger){

    require_auth(get_self());
    // Get match
    auto match_itr = existingHostGames.require_find(game_id, "Game not found.");

    eosio::check(match_itr->challenger != none , "This match is full now");

    existingHostGames.modify(match_itr, get_self(), [&](auto& g) {
      g.challenger = challenger;
    });

  }

  void makeChoice(
      const uint64_t& game_id,
      const name & player, 
      const uint8_t& round_number,
      const std::string& choice,
      const eosio::public_key& key,
      const eosio::signature& signature){



      require_auth(get_self());

      // check(has_auth(player), "The next move should be made by " + by.to_string());
          // Get match
      auto match_itr = existingHostGames.require_find(game_id, "Game does not exist.");

      // Check if this game hasn't ended yet
      check(match_itr->winner == none, "The game has ended.");
      
      eosio::check(match_itr->challenger.value == player.value || match_itr->host.value == player.value, "You are not playing this game.");

      eosio::check(match_itr->round_number == round_number , "You are not playing this round.");

      if(match_itr->challenger.value == player.value){
        eosio::check( match_itr->choices_of_challenger.size() + 1 == round_number , "You are not playing this round.");
        eosio::check( match_itr->has_challenger_made_choice == false , "You have already selected your choice.");
      }

      if(match_itr->host.value == player.value){
        eosio::check( match_itr->choices_of_host.size() + 1 == round_number , "You are not playing this round.");
        eosio::check( match_itr->has_host_made_choice == false , "You have already selected your choice.");
      }

      // Check that the choice, key and signature are valid
      std::string data_to_hash = std::to_string(game_id) + ":" + choice;
      eosio::checksum256 choice_digest = eosio::sha256(data_to_hash.data(), data_to_hash.size());
      eosio::public_key recovered_key = eosio::recover_key(choice_digest, signature);
      eosio::check(recovered_key == key, "Invalid signature or key provided.");

      if(match_itr->challenger.value == player.value){
        existingHostGames.modify(match_itr, get_self(), [&](auto& g) {
          g.choices_of_challenger.push(choice);
          g.has_challenger_made_choice = true;
          g.challenger_key = key;
          g.winner = checkWinner(g)
        });
      }else if(match_itr->host.value == player.value){
        existingHostGames.modify(match_itr, get_self(), [&](auto& g) {
          g.choices_of_host.push(choice);
          g.has_host_made_choice = true;
          g.host_key = key;
          g.winner = checkWinner(g)
        });
      }
  }


  name rps::checkWinner(const game &currentGame)
  {

      bool isFullRound = false;
      
      if( currentGame.has_host_made_choice  && currentGame.has_challenger_made_choice &&  currentGame.choices_of_host.size()  == currentGame.choices_of_challenger.size() && currentGame.choices_of_host.size() >  0 ){

        isFullRound = currentGame.choices_of_host.size() > 12;

        string host_choice =    currentGame.choices_of_host.back();
        string challenger_choice =    currentGame.choices_of_challenger.back();
        int result = states[host_choice][challenger_choice];

        if(result == 0){
          // game is tie

        }else if(result == 1){
          // host is win
          currentGame.host_win_count += 1;
        }else if(result == 2) {
          //challenge is win
          currentGame.challenger_win_count += 1;
        }

        if(currentGame.host_win_count > 1){
          return currentGame.host;
        }
        
        if(currentGame.challenger_win_count > 1){
            return currentGame.challenge;
        }
      }

      // Draw if the board is full, otherwise the winner is not determined yet
      return isFullRound ? draw : none;
   }

} // namepsace contract