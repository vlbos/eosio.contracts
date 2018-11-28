/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */


#include <eosiolib/action.hpp>
#include <eosiolib/transaction.hpp>
#include <ibc.token/ibc.token.hpp>

#include "token.cpp"

namespace eosio {

   token::token( name receiver, name code, datastream<const char*> ds ):
   contract( receiver, code, ds )
   {

   }

   struct transfer_args{
      name    from;
      name    to;
      asset   quantity;
      string  memo;
   };

   void trim(string &s)
   {
      if( !s.empty() )
      {
         s.erase(0,s.find_first_not_of(" "));
         s.erase(s.find_last_not_of(" ") + 1);
      }
   }


   void token::regtoken( name contract, symbol sym, uint64_t min_quantity){

      require_auth( _self );
      eosio_assert( is_account( contract ), "contract account does not exist");

      accepted_assets_table acpt_assets(_self, contract.value);
      auto existing = acpt_assets.find( sym.raw() );
      eosio_assert( existing == acpt_assets.end(), ( contract.to_string() + ":" + sym.code().to_string() + " already exists" ).c_str() );


      acpt_assets.emplace( _self, [&]( auto& r ) {
         r.sym      = sym;
//         r.contract = contract;
      });
   }






   /*
    * memo format:
    * 1. ibc account [memo]
    * 2. transfer
    */
   struct ibctrxinfo{
      uint64_t             transfer_seq;  //递增
      uint32_t             block_num;
      capi_checksum256     trx_id;
      name                 from;
      asset                quantity;
      string               memo;
   };


   void token::transfer_notify( name from_token_contract, name from, name to, asset quantity, string memo ) {
      capi_checksum256 trx_id;
      std::vector<char> trx_bytes;
      trx_bytes.resize(transaction_size());
      read_transaction(trx_bytes.data(), transaction_size());


      ibctrxinfo info{1, 2, capi_checksum256(), "abc"_n, asset{100, symbol{"EOSS", 4}}, "memomomo"};
      action(permission_level{_self, "active"_n}, "eos222333ibc"_n, "ibctrxinfo"_n, info).send();
   }

//      printhex(trx_bytes.data(),trx_bytes.size());
//      print("====");
//      return;
//
//
//      if (to != _self) { return; }
//
//      auto memo_len = memo.length();
//      eosio_assert( memo_len > 4, "non suppoted action");
//

      // check code and quantity symbol

//      accepted_assets_table acpt_assets(self, contract.value)
//      auto existing = acpt_assets.find( sym.raw() );
//      eosio_assert( existing == acpt_assets.end(), ( contract.to_string() + ":" + sym.code().to_string() " already exists" ).c_str() );
//
//

//
//      if (memo.find("ibc ") == 0) {
//         auto m = memo.substr(4);
//         trim(m);
//         eosio_assert( !m.empty(), "--");
//
//         string name_str;
//         string ibc_memo;
//         if(m.find(" ") == std::string::npos ){
//            name_str = m;
//         } else{
//            name_str = m.substr(0, m.find(" "));
//            ibc_memo = m.substr(m.find(" "));
//         }
//
//         name ibc_to{ name_str };
//
//
//
////         action(permission_level{ _self, "active"_n }, _co.icp, N(sendaction), icp_sendaction{seq, send_action, expiration, receive_action}).send();
//
//
//
//      } else if (memo.find("transfer") == 0) {
//         return;
//      } else {
//         eosio_assert( false, "non suppoted action");
//      }
//   }
//
//
//





} /// namespace eosio


extern "C" {
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
      if( code == receiver ) {
         switch( action ) {
            EOSIO_DISPATCH_HELPER( eosio::token, (create)(issue)(transfer)(open)(close)(retire) )
         }
      }
      if (code != receiver && action == eosio::name("transfer").value) {
         auto args = eosio::unpack_action_data<eosio::transfer_args>();
         eosio::token thiscontract(eosio::name(receiver), eosio::name(code), eosio::datastream<const char*>(nullptr, 0));
         thiscontract.transfer_notify(eosio::name(code), args.from, args.to, args.quantity, args.memo);
      }
   }
}

