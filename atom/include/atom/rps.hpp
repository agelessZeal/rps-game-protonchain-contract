#pragma once

// Standard.
#include <vector>

// EOS
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>
#include <eosio/transaction.hpp>


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

    static map<std:string ,map<std:string,uint8_t>> state = {
        { "R", {{"R",0},{"P",2},{"S",1}}},
        { "P",{{"P",0},{"R",1},{"S",2}}},
        { "S",{{"R",2},{"P",1},{"S",0}}}
      };


    // states['R']['R'] = 0;
    // states['R']['P'] = 2;
    // states['R']['S'] = 1;
    // states['P']['R'] = 1;
    // states['P']['P'] = 0;
    // states['P']['S'] = 2;
    // states['S']['R'] = 2;
    // states['S']['P'] = 1;
    // states['S']['S'] = 0;


        // Declare game data structure.
    struct [[eosio::table]] game
    {

        game() : board(boardWidth * boardHeight, 0){}

        static constexpr uint16_t boardWidth = 3;
        static constexpr uint16_t boardHeight = boardWidth;
        
        game() : board(boardWidth * boardHeight, 0){}

        name challenger;
        name host;
        name turn;              // = account name of host/ challenger
        name winner = none; // = none/ draw/ name of host/ name of challenger

        std::vector<uint8_t> board;

        // Initialize board with empty cell
        void initializeBoard()
        {
            board.assign(boardWidth * boardHeight, 0);
        }

        // Reset game
        void resetGame()
        {
            initializeBoard();
            turn = host;
            winner = "none"_n;
        }

        auto primary_key() const { return challenger.value; }
        EOSLIB_SERIALIZE( game, (challenger)(host)(turn)(winner)(board))
    };



    // This function will be called when the contract is notified of incoming or outgoing transfer actions from any contract
    [[eosio::on_notify("*::transfer")]]
    void ontransfer   ( const name& from,
                        const name& to,
                        const asset& quantity,
                        const string& memo );

    void delegatebw   ( const name& from,
                        const name& receiver,
                        const asset& stake_net_quantity,
                        const asset& stake_cpu_quantity,
                        bool transfer );
    void undelegatebw ( const name& from,
                        const name& receiver,
                        const asset& unstake_net_quantity,
                        const asset& unstake_cpu_quantity );
    void buyrambytes  ( const name& payer,
                        const name& receiver,
                        uint32_t bytes );

    // Action wrappers
    using withdraw_action     = action_wrapper<"withdraw"_n,     &rps::withdraw>;
    using transfer_action     = action_wrapper<"transfer"_n,     &rps::ontransfer>;
    using delegatebw_action   = action_wrapper<"delegatebw"_n,   &rps::delegatebw>;
    using undelegatebw_action = action_wrapper<"undelegatebw"_n, &rps::undelegatebw>;
    using buyrambytes_action  = action_wrapper<"buyrambytes"_n,  &rps::buyrambytes>;
    using planreceipt_action  = action_wrapper<"planreceipt"_n,  &rps::planreceipt>;

    // Initialize tables from tables.hpp
    account_table _accounts;
    plan_table _plans;
    subscription_table _subscriptions;

  private:
    void add_balance (const name& account, const extended_asset& delta);
    void substract_balance (const name& account, const extended_asset& delta);
    void end_subscription(const Subscription& subscription);
    void transfer_to(const name& to, const extended_asset& balance, const string& memo);
  };
}