/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/producer_schedule.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/singleton.hpp>

#include <string>

namespace eosio {

   using std::string;

   struct block_header {
      uint32_t                                  timestamp;
      name                                      producer;
      uint16_t                                  confirmed;
      capi_checksum256                          previous;
      capi_checksum256                          transaction_mroot;
      capi_checksum256                          action_mroot;
      uint32_t                                  schedule_version;
      std::optional<eosio::producer_schedule>   new_producers;
      extensions_type                           header_extensions;

      capi_checksum256     digest()const;
      capi_checksum256     id() const;
      uint32_t             block_num() const { return num_from_id(previous) + 1; }
      static uint32_t      num_from_id(const capi_checksum256& id);

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE(block_header, (timestamp)(producer)(confirmed)(previous)(transaction_mroot)(action_mroot)
            (schedule_version)(new_producers)(header_extensions))
   };

   struct signed_block_header : public block_header {
      capi_signature     producer_signature;

      EOSLIB_SERIALIZE_DERIVED( signed_block_header, block_header, (producer_signature) )
   };

   struct [[eosio::table("blockheaders"), eosio::contract("ibc")]] block_headers /*: public block_header*/ {
      uint64_t             block_num;
      capi_checksum256     block_id;
      block_header         header;
      capi_signature       producer_signature;

      uint64_t primary_key()const { return block_num; }
   };

   typedef eosio::multi_index< "blockheaders"_n, block_headers >  block_header_table;

   struct [[eosio::table("producers"), eosio::contract("ibc")]] producer_schedule_type {
      uint32_t                      version;
      std::vector<producer_key>     producers;

      public_key get_producer_key( name p )const {
         for( const auto& i : producers )
            if( i.producer_name == p )
               return i.block_signing_key;
         return public_key();
      }
   };
   typedef eosio::singleton< "producers"_n, producer_schedule_type >  producer_schedule_singleton;



} /// namespace eosio


