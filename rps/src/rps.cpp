#include <rps/rps.hpp>
#include <rps/constants.hpp>

namespace proton
{

    void rps::rpsrefund(
       const uint64_t& game_index,
       const std::string&game_id,
       const name &challenger,
       const name &host) {

       require_auth(get_self()); //only this contract possible to call

       rps_matches existingHostGames(get_self(), get_self().value);

       auto match_itr = existingHostGames.find(game_index);

       if(match_itr == existingHostGames.end()){
            bool digit_check = is_digits(game_id);
            check(digit_check,"The game id is wrong.");
            int index =  stoi(game_id);
            match_itr = existingHostGames.find(index);
       }

       uint32_t count = 0;

       auto ittr = existingHostGames.begin();

       for (; ittr != existingHostGames.end(); ittr++) {
         count ++;
       }

        if(match_itr == existingHostGames.end()){
            auto itr = existingHostGames.begin();

            for (; itr != existingHostGames.end(); itr++) {
              auto gm = *itr;
              if(gm.index == game_index){
                match_itr = itr;
                break;
              }
              if(gm.game_id == game_id){
                match_itr = itr;
                break;
              }
            }
        }

        if(match_itr == existingHostGames.end()){
            auto itr = existingHostGames.begin();

            for (; itr != existingHostGames.end(); itr++) {
              auto gm = *itr;
              if(gm.host == host && gm.challenger == challenger && gm.winner == none ){
                match_itr = itr;
                break;
              }
            }
        }


       uint32_t count2 = 0;

        if(count == 0){
             match_itr =  existing_games.find(game_index);
             auto itr = existing_games.begin();

             for (; itr != existing_games.end(); itr++) {
               count2 ++;
             }
        }

        std::string s = std::to_string( count );

        std::string s2 = std::to_string( count2 );

        std::string s1 ( "The table size is empty" );

//        check(count != 0,"The table size is empty");

       check(count != 0, s + s1 + s2 + s1  + game_id);

       check(match_itr != existingHostGames.end(),"Can't found your game with your index");

       if(match_itr->host_bet > 0){
           asset a = asset(match_itr->host_bet, symbol("XPR", 4));

           extended_asset award_asset = extended_asset(a,SYSTEM_TOKEN_CONTRACT);

           transfer_to(match_itr->host, award_asset, "refund");
       }

       if(match_itr->challenger_bet > 0){
           asset a = asset(match_itr->challenger_bet, symbol("XPR", 4));

           extended_asset award_asset = extended_asset(a,SYSTEM_TOKEN_CONTRACT);

           transfer_to(match_itr->challenger, award_asset, "refund");
       }

       existingHostGames.erase(match_itr);
    }

    void rps::rpswinprize(
       const name &winner,
       const name &rps,
       const uint64_t& game_index,
       const std::string& game_id,
       const name &challenger,
       const name &host){

        require_auth(get_self()); //only this contract possible to call

        rps_matches existingHostGames(get_self(), get_self().value);

        auto match_itr = existingHostGames.find(game_index);

        if(match_itr == existingHostGames.end()){
              bool digit_check = is_digits(game_id);
              check(digit_check,"The game id is wrong.");
              int index =  stoi(game_id);
              match_itr = existingHostGames.find(index);
        }

        uint32_t count = 0;

        auto ittr = existingHostGames.begin();

        for (; ittr != existingHostGames.end(); ittr++) {
          count ++;
        }

        if(match_itr == existingHostGames.end()){
            auto itr = existingHostGames.begin();

            for (; itr != existingHostGames.end(); itr++) {

              auto gm = *itr;
              if(gm.index == game_index){
                match_itr = itr;
                break;
              }
              if(gm.game_id == game_id){
                match_itr = itr;
                break;
              }
            }
        }

        if(match_itr == existingHostGames.end()){
            auto itr = existingHostGames.begin();

            for (; itr != existingHostGames.end(); itr++) {
              auto gm = *itr;
              if(gm.host == host && gm.challenger == challenger && gm.winner == none  &&  gm.host_bet > 0 && gm.challenger_bet > 0){
                match_itr = itr;
                break;
              }
            }
        }


        if(count == 0){
             match_itr =  existing_games.find(game_index);
             auto itr = existing_games.begin();

             for (; itr != existing_games.end(); itr++) {
               count ++;
             }
        }

        std::string s = std::to_string( count );

        std::string s1 ( "The table size is empty:" );

        check(count != 0,"The table size is empty");

//        check(false, s + s1 +  game_id);

        check(match_itr != existingHostGames.end(),"Can't found your game with your index");

        check(winner.value == match_itr->host.value || winner.value == match_itr->challenger.value, "The winner doesn't play in this game.");

        check( match_itr->host_bet > 0 && match_itr->challenger_bet > 0, "Both players don't deposit in this game.");

        uint64_t amount = (uint64_t)(( match_itr->host_bet + match_itr->challenger_bet)*0.99);

        asset a = asset(amount, symbol("XPR", 4));

        extended_asset award_asset = extended_asset(a,SYSTEM_TOKEN_CONTRACT);

        transfer_to(winner, award_asset, "winner");


        uint64_t fee_amount = (uint64_t)(( match_itr->host_bet + match_itr->challenger_bet)*0.01);

        asset fee_a = asset(fee_amount, symbol("XPR", 4));

        extended_asset fee_asset = extended_asset(fee_a,SYSTEM_TOKEN_CONTRACT);

        transfer_to(rps, fee_asset, "fee");

        existingHostGames.erase(match_itr);
    }

    void rps::uint64_to_string( uint64_t value, std::string& result ) {
        result.clear();
        result.reserve( 20 ); // max. 20 digits possible
        uint64_t q = value;
        do {
            result += "0123456789"[ q % 10 ];
            q /= 10;
        } while ( q );
        std::reverse( result.begin(), result.end() );
    }


   void rps::send_balance(const name & winner,const rps_game &current_game){

        uint64_t amount = (uint64_t)(( current_game.host_bet + current_game.challenger_bet)*0.90);

        asset a = asset(amount, symbol("XPR", 4));

        extended_asset award_asset = extended_asset(a,SYSTEM_TOKEN_CONTRACT);

        transfer_to(winner, award_asset, "winner");

   }

   bool rps::is_digits(const std::string &str) {
       return str.find_first_not_of("0123456789") == std::string::npos;
   }

} // namepsace contract
