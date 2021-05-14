#include <rps/rps.hpp>
#include <rps/constants.hpp>

namespace proton {
  void rps::ontransfer (const name& from, const name& to, const asset& quantity, const string& memo) {
    // Skip if outgoing
    if (from == get_self()) {
      return;
    }

    // Skip if deposit memo
    if (memo == "deposit") {
      return;
    }

    // Skip if winner memo
    if (memo == "winner") {
      return;
    }

    // Skip if refund memo
    if (memo == "refund") {
      return;
    }

    // Skip if deposit from system accounts
    if (from == "eosio.stake"_n || from == "eosio.ram"_n || from == "eosio"_n) {
      return;
    }

    // Validate transfer
    check(to == get_self(), "Invalid Deposit");

    // Deposit
    name token_contract = get_first_receiver();
    check(token_contract == SYSTEM_TOKEN_CONTRACT, "only eosio.token tokens are supported");

    // Add balance
    auto balance_to_add = extended_asset(quantity, token_contract);

    add_balance(from,balance_to_add,memo);

  }

//  void rps::withdraw (const name& account, const extended_asset& balance) {
//    require_auth(get_self());
//
////    substract_balance(account, balance);
//    transfer_to(account, balance, "winner");
//  }



  void rps::transfer_to(const name& to, const extended_asset& balance, const string& memo) {
    transfer_action t_action( balance.contract, {get_self(), "active"_n} );
    t_action.send(get_self(), to, balance.quantity, memo);
  }

}
