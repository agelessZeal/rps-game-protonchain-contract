#pragma once

// Standard.
#include <vector>
#include <map>
#include <string>

// EOS
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>
#include <eosio/transaction.hpp>
#include <eosio/system.hpp>
#include <eosio/print.hpp>


// Local
#include "constants.hpp"

using namespace eosio;
using namespace std;


namespace proton {
  CONTRACT rps : public contract {
  public:
    using contract::contract;

    rps( name receiver, name code, datastream<const char*> ds )
      : contract(receiver, code, ds),
      existing_games(receiver, receiver.value) {}

    static constexpr name none = "none"_n;
    static constexpr name draw = "draw"_n;
    static constexpr uint8_t gametimeout = 5;

    // Declare class method.
    [[eosio::action]]
    void create(name &host,name &challenger);

    // Declare class method.
    [[eosio::action]]
    void restart(const name &challenger, const name &host, const name &by);

    // Declare class method.
    [[eosio::action]]
    void checkround(const name &challenger, const name &host);

//    // Declare class method.
//    [[eosio::action]]
//    void close(
//      const name &challenger,
//      const name &host,
//      const name &by);

    // Declare class method.
    [[eosio::action]]
    void startround( const name &challenger, const name &host, const name &by);


//    // Declare class method.
//    [[eosio::action]]
//    void join(
//      const name &host,
//      const name &challenger);


    // Declare class method.
    [[eosio::action]]
    void makechoice(
       const name &player,
       const uint8_t& round_number,
       const eosio::checksum256& choice_digest);

    // Declare class method.
    [[eosio::action]]
    void unlockchoice(
       const name &player,
       const uint8_t& round_number,
       const std::string& choice,
       const std::string& password);

           // Declare class method.
    [[eosio::action]]
    void ticker();

    // This function will be called when the contract is notified of incoming or outgoing transfer actions from any contract
    [[eosio::on_notify("*::transfer")]]
    void ontransfer   ( const name& from,
                        const name& to,
                        const asset& quantity,
                        const string& memo );

    // Action wrappers
    using transfer_action     = action_wrapper<"transfer"_n,     &rps::ontransfer>;



              // Declare game data structure.
        TABLE game
        {

            uint64_t index;

            name challenger = none;
            name host = none;
            name winner = none; // = none/ draw/ name of host/ name of challenger

            uint8_t round_number;
            uint8_t host_win_count;
            uint8_t challenger_win_count;

            uint64_t host_bet;
            uint64_t challenger_bet;

            uint64_t start_at;
            uint64_t created_at;

            bool host_available;
            bool challenger_available;

            bool has_host_made_choice;
            bool has_challenger_made_choice;



            std::string host_choice_hash;
            std::string host_choice_password;
            std::string host_choice;

            std::string challenger_choice_hash;
            std::string challenger_choice_password;
            std::string challenger_choice;

            // Reset game
            void resetGame()
            {
                winner = "none"_n;
                round_number = 1;
                challenger_win_count = 0;
                host_win_count = 0;

                has_host_made_choice =  false;
                has_challenger_made_choice = false;
                host_available = false;
                challenger_available = false;
                host_choice_password = "";
                host_choice= "";
                challenger_choice_hash =  "";
                host_choice_hash =  "";

                challenger_choice_password = "";
                challenger_choice = "";

                host_bet = 0;
                challenger_bet = 0;

                start_at = 0;
            }

                    // Reset game
            void newRound()
            {
                has_host_made_choice =  false;
                has_challenger_made_choice = false;
                round_number += 1;
                host_choice_password = "";
                host_choice="";

                challenger_choice_password = "";
                challenger_choice = "";

                challenger_choice_hash =  "";
                host_choice_hash =  "";

//               challenger_choice_hash =  eosio::sha256("0000000000000000", 16);
//               host_choice_hash =  eosio::sha256("0000000000000000", 16);

                start_at = eosio::current_time_point().sec_since_epoch();
            }

            uint64_t primary_key() const { return host.value; };
    //        uint64_t get_secondary() const { return host.value; };
            uint64_t get_third() const { return challenger.value; };

            EOSLIB_SERIALIZE( game, (start_at)(host)(challenger)(winner)(host_bet)(challenger_bet)(host_win_count)(challenger_win_count)(host_choice)(challenger_choice)(host_choice_password)(challenger_choice_password)(challenger_choice_hash)(host_choice_hash))
        };

            // Define the games type which uses the game data structure.
        typedef multi_index<"games"_n, game,
         indexed_by<"host"_n, const_mem_fun< game, uint64_t, &game::primary_key>>,
         indexed_by<"challenger"_n,const_mem_fun<game,uint64_t, &game::get_third>>
        >  games;

        games existing_games;

  private:

    uint32_t checkgames();

    void deftx(uint64_t delay );

    void transfer_to(const name& to, const extended_asset& balance, const string& memo);

    void add_balance (const name& account, const extended_asset& delta);

//    checksum256 decode_checksum(string hex);
    string to_hex(const checksum256 &hashed);


//    string SHA256toHEX(capi_checksum256 sha256);
//    string conv2HEX(char* hasha, uint32_t ssize);
//    capi_checksum256 HEX2SHA256(string hexstr);
//    uint8_t convFromHEX(char ch);
//    size_t  convFromHEX(string hexstr, char* res, size_t res_len);

    std::string random_choice(const game &currentGame);

    name check_winner(const game &currentGame);

    static inline eosio::checksum256 hash_key(eosio::public_key key) {
      eosio::ecc_public_key public_key_data = std::get<0>(key);
      return eosio::sha256(public_key_data.data(), public_key_data.size());
    }
  };
}
