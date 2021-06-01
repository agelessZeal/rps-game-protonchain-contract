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
    void rpswinprize(
       const name &winner,
       const name &rps,
       const uint64_t& game_index,
       const std::string&game_id,
       const name &challenger,
       const name &host
    );

    // Declare class method.
    [[eosio::action]]
    void rpsrefund(
       const uint64_t& game_index,
       const std::string&game_id,
       const name &challenger,
       const name &host
    );

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

    void transfer_to(const name& to, const extended_asset& balance, const string& memo);

    void add_balance (const name& account, const extended_asset& delta,const string& memo);

    void uint64_to_string( uint64_t value, std::string& result );

    string to_hex(const checksum256 &hashed);

    std::string random_choice(const rps_game &currentGame);

    void send_balance(const name & winner,const rps_game &current_game);

    bool is_digits(const std::string &str);

  };
}
