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

//    // Declare class method.
//    [[eosio::action]]
//    void create(name &host,name &challenger);

//    // Declare class method.
//    [[eosio::action]]
//    void restart(const name &challenger, const name &host, const name &by);

//    // Declare class method.
//    [[eosio::action]]
//    void checkround(const name &challenger, const name &host);

//    // Declare class method.
//    [[eosio::action]]
//    void close(
//      const name &challenger,
//      const name &host,
//      const name &by);

    // Declare class method.
    // [[eosio::action]]
    // void startround( const name &challenger, const name &host, const name &by);


//    // Declare class method.
//    [[eosio::action]]
//    void makechoice(
//       const name &player,
//       const name &host,
//       const name &challenger,
//       const uint8_t& round_number,
//       const eosio::checksum256& choice_digest);

//    // Declare class method.
//    [[eosio::action]]
//    void unlockchoice(
//       const name &player,
//       const name &host,
//       const name &challenger,
//       const uint8_t& round_number,
//       const std::string& choice,
//       const std::string& password);

//     // Declare class method.
//    [[eosio::action]]
//    void ticker();

    // Declare class method.
    [[eosio::action]]
    void checkwinner(
       const name &winner,
       const uint64_t& game_index,
       const std::string&game_id
    );

    // Declare class method.
    [[eosio::action]]
    void refund(
       const uint64_t& game_index,
       const std::string&game_id
    );

//    ACTION withdraw   ( const name& account,
//                        const extended_asset& balance,
//                        const std::string& game);

    // This function will be called when the contract is notified of incoming or outgoing transfer actions from any contract
    [[eosio::on_notify("*::transfer")]]
    void ontransfer   ( const name& from,
                        const name& to,
                        const asset& quantity,
                        const string& memo );


    // Action wrappers
    using transfer_action     = action_wrapper<"transfer"_n,     &rps::ontransfer>;

    // Declare game data structure.
    struct [[eosio::table]] rps_game
    {

        uint64_t index;
        std::string game_id;
        name challenger = none;
        name host = none;
        name winner = none;

        uint64_t host_bet;
        uint64_t challenger_bet;

        uint64_t primary_key() const { return index; };

        EOSLIB_SERIALIZE( rps_game, (index)(game_id)(challenger)(host)(winner)(host_bet)(challenger_bet));
    };


        // Define the games type which uses the game data structure.
    typedef multi_index<"games"_n, rps_game>  rps_matches;

    rps_matches existing_games;

  private:

//    uint32_t checkgames();
//
//    void deftx(uint64_t delay );

    void transfer_to(const name& to, const extended_asset& balance, const string& memo);

    void add_balance (const name& account, const extended_asset& delta,const string& memo);

    void uint64_to_string( uint64_t value, std::string& result );

    string to_hex(const checksum256 &hashed);

    std::string random_choice(const rps_game &currentGame);

    void send_balance(const name & winner,const rps_game &current_game);

    bool is_digits(const std::string &str);

    static inline eosio::checksum256 hash_key(eosio::public_key key) {
      eosio::ecc_public_key public_key_data = std::get<0>(key);
      return eosio::sha256(public_key_data.data(), public_key_data.size());
    }
  };
}
