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


// Local
#include "constants.hpp"

using namespace eosio;
using namespace std;


namespace proton {
  CONTRACT rps : public contract {
  public:
    using contract::contract;

    rps( name receiver, name code, datastream<const char*> ds )
      : contract(receiver, code, ds) {}

    static constexpr name none = "none"_n;
    static constexpr name draw = "draw"_n;

    static map<string ,map<string,uint8_t>> state = {
        { "R", {{"R",0},{"P",2},{"S",1}}},
        { "P",{{"P",0},{"R",1},{"S",2}}},
        { "S",{{"R",2},{"P",1},{"S",0}}}
      };

        // Declare game data structure.
    struct [[eosio::table]] game
    {

        uint64_t index;
        uint8_t round_number;
        uint8_t host_win_count;
        uint8_t challenger_win_count;

        uint8_t stake;

        const uint64_t& starts_at,

        vector<string> choices_of_host;
        vector<string> choices_of_challenger; 

        bool has_host_made_choice;
        bool has_challenger_made_choice;

        eosio::public_key host_key;
        eosio::checksum256 host_txid;

        eosio::public_key challenger_key;
        eosio::checksum256 challenger_txid;

        name challenger;
        name host;
        name winner = none; // = none/ draw/ name of host/ name of challenger


        // Reset game
        void resetGame()
        {
            winner = "none"_n;
            round_number = 0; 
            challenger_win_count = 0;
            host_win_count = 0;

            choices_of_host.clear();
            choices_of_challenger.clear();
            has_host_made_choice =  false;
            has_challenger_made_choice = false;
        }

                // Reset game
        void newRound()
        {
            winner = "none"_n;
            has_host_made_choice =  false;
            has_challenger_made_choice = false;
            round_number += 1;
        }


        auto primary_key() const { return index; }
        EOSLIB_SERIALIZE( game, (index)(starts_at)(stake)(challenger)(host)(winner)(challenger_win_count)(host_win_count)(choices_of_host)(choices_of_challenger)(host_key)(host_txid)(challenger_key)(challenger_txid))
    };

        // Define the games type which uses the game data structure. 
    typedef eosio::multi_index<"games"_n, game> games;

    // Declare class method.
    [[eosio::action]]
    void create(const name &challenger, name &host);
    
    // Declare class method.
    [[eosio::action]]
    void restart(const name &challenger, const name &host, const name &by);

    // Declare class method.
    [[eosio::action]]
    void close(      
      const uint64_t& game_id,
      const name &challenger, 
      const name &host);
    
    // Declare class method.
    [[eosio::action]]   
    void join(
      const uint64_t& game_id,
      const name &player);


    // Declare class method.
    [[eosio::action]]   
    void makeChoice(
       const uint64_t& game_id,
       const name &player, 
       const std::string& choice,
       const eosio::public_key& key,
       const eosio::signature& signature);

    // This function will be called when the contract is notified of incoming or outgoing transfer actions from any contract
    [[eosio::on_notify("*::transfer")]]
    void ontransfer   ( const name& from,
                        const name& to,
                        const asset& quantity,
                        const string& memo );


    // Action wrappers
    using transfer_action     = action_wrapper<"transfer"_n,     &rps::ontransfer>;

  private:
    name getWinner(const game &currentGame);

    bool isEmptyCell(const uint8_t &cell);
    bool isValidMove(const uint16_t &row, const uint16_t &column, const std::vector<uint8_t> &board);

    
    static inline eosio::checksum256 hash_key(eosio::public_key key) {
      eosio::ecc_public_key public_key_data = std::get<0>(key);
      return eosio::sha256(public_key_data.data(), public_key_data.size());
    }

    static inline eosio::checksum256 get_txid () {
      auto size = eosio::transaction_size();
      char buf[size];
      uint32_t read = eosio::read_transaction(buf, size);
      eosio::check( size == read, "read_transaction failed");
      return eosio::sha256(buf, size);
    }

  };
}