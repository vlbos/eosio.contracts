/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <ibc/chain.hpp>
#include <ibc/merkle.hpp>
#include <string>

namespace eosio {

   using std::string;

   class [[eosio::contract("ibc")]] ibc : public contract {
      private:
         producer_schedule_singleton   _producer_schedule;
         producer_schedule_type        _producer_schedule_state;
         incremental_merkle_singleton  _incremental_merkle;
         incremental_merkle            _incremental_merkle_state;
         block_header_table            _block_header_table;

      public:
         ibc( name s, name code, datastream<const char*> ds );
         ~ibc();

         [[eosio::action]]
         void initchain( const std::vector<char>& init_block_header,
                         const producer_schedule_type& init_producer_schedule,
                         const incremental_merkle& init_incr_merkle );

         [[eosio::action]]
         void addheaders( const std::vector<char>& headers);


         [[eosio::action]]
         void ibctrxinfo(  uint64_t    transfer_seq,
                           uint32_t    block_time_slot,
                           capi_checksum256  trx_id,
                           name        from,
                           asset       quantity,
                           string      memo );

         [[eosio::action]]
         remoteibctrx( const uint32_t block_num,
                       const std::vector<char>& packed_trx,
                       const std::vector<capi_checksum256>& merkle_path);








         // for test
         [[eosio::action]]
         void header( const std::vector<char>& init_block_header);

         [[eosio::action]]
         void ps( const producer_schedule_type& params);

         [[eosio::action]]
         void merkle( const incremental_merkle& params);

         [[eosio::action]]
         void merkleadd( const digest_type& params);
   };

} /// namespace eosio
