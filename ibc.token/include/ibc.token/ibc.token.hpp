/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/singleton.hpp>
#include <string>


namespace eosio {
   using std::string;

   struct [[eosio::table, eosio::contract("ibc.token")]] accepted_asset {
      symbol   sym;

      uint64_t  primary_key()const { return sym.raw(); }
   };

   typedef eosio::multi_index< "acptassets"_n, accepted_asset> accepted_assets_table;


   struct [[eosio::table, eosio::contract("ibc.token")]] globals {
      globals(){}

      name  ibc_contract;
   };
   typedef eosio::singleton< "global"_n, globals > globals_singleton;



   class [[eosio::contract("ibc.token")]] token : public contract {
      private:


      public:
         token( name receiver, name code, datastream<const char*> ds );


         [[eosio::action]]
         void regtoken( name contract, symbol sym, uint64_t min_quantity);

         void transfer_notify( name code, name from, name to, asset quantity, string memo );





         [[eosio::action]]
         void create( name   issuer,
                      asset  maximum_supply);

         [[eosio::action]]
         void issue( name to, asset quantity, string memo );

         [[eosio::action]]
         void retire( asset quantity, string memo );

         [[eosio::action]]
         void transfer( name    from,
                        name    to,
                        asset   quantity,
                        string  memo );

         [[eosio::action]]
         void open( name owner, const symbol& symbol, name ram_payer );

         [[eosio::action]]
         void close( name owner, const symbol& symbol );

         static asset get_supply( name token_contract_account, symbol_code sym_code )
         {
            stats statstable( token_contract_account, sym_code.raw() );
            const auto& st = statstable.get( sym_code.raw() );
            return st.supply;
         }

         static asset get_balance( name token_contract_account, name owner, symbol_code sym_code )
         {
            accounts accountstable( token_contract_account, owner.value );
            const auto& ac = accountstable.get( sym_code.raw() );
            return ac.balance;
         }

      private:




         struct [[eosio::table]] account {
            asset    balance;

            uint64_t primary_key()const { return balance.symbol.code().raw(); }
         };

         struct [[eosio::table]] currency_stats {
            asset    supply;
            asset    max_supply;
            name     issuer;

            uint64_t primary_key()const { return supply.symbol.code().raw(); }
         };

         typedef eosio::multi_index< "accounts"_n, account > accounts;
         typedef eosio::multi_index< "stat"_n, currency_stats > stats;

         void sub_balance( name owner, asset value );
         void add_balance( name owner, asset value, name ram_payer );


   };

} /// namespace eosio
