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
#include "merkle.hpp"

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


      // ---- tables ----
   struct [[eosio::table("chaindb"), eosio::contract("ibc")]] block_header_type {
      uint64_t                   block_num;
      block_id_type              block_id;
      signed_block_header        header;
      uint32_t                   active_schedule_id;
      uint32_t                   pending_schedule_id;
      incremental_merkle         blockroot_merkle;
      std::vector<uint8_t>       confirm_count;

      uint64_t primary_key()const { return block_num; }
   };
   typedef eosio::multi_index< "chaindb"_n, block_header_type >  chaindb;


   struct [[eosio::table("prodsches"), eosio::contract("ibc")]] producer_schedule_ {
      uint64_t                      id;
      producer_schedule             schedule;
      digest_type                   schedule_hash;

      uint64_t primary_key()const { return id; }
      public_key get_producer_key( name p )const {
         for( const auto& i : schedule.producers )
            if( i.producer_name == p )
               return i.block_signing_key;
         return public_key();
      }
   };
   typedef eosio::multi_index< "prodsches"_n, producer_schedule_ >  prodsches;


} /// namespace eosio


