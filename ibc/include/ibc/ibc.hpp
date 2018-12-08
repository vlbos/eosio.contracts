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
         prodsches      _prodsches;
         chaindb        _chaindb;

      public:
         ibc( name s, name code, datastream<const char*> ds );
         ~ibc();

//         [[eosio::action]]
//         void chaininit(const std::vector<char>& init_block_header,
//                      const producer_schedule& init_active_schedule,
//                      const incremental_merkle& init_blockroot_merkle );

         [[eosio::action]]
         void chaininit( const std::vector<char>&      header,
                         const producer_schedule&      pending_schedule,
                         const producer_schedule&      active_schedule,
                         const incremental_merkle&     blockroot_merkle,
                         const std::vector<uint8_t>&   confirm_count);

         [[eosio::action]]
         void addheader( const std::vector<char>& header);


         [[eosio::action]]
         void addheaders( const std::vector<char>& headers);


         // ---















         [[eosio::action]]
         void initchain( const std::vector<char>& init_block_header,
                         const producer_schedule_type& init_producer_schedule,
                         const incremental_merkle& init_incr_merkle );




         [[eosio::action]]
         void ibctrxinfo(  uint64_t    transfer_seq,
                           uint32_t    block_time_slot,
                           capi_checksum256  trx_id,
                           name        from,
                           asset       quantity,
                           string      memo );

         [[eosio::action]]
         void remoteibctrx( const uint32_t block_num,
                       const std::vector<char>& packed_trx,
                       const std::vector<capi_checksum256>& merkle_path);












         // for test


      [[eosio::action]]
      void packedtrx( const std::vector<char>& trx_receipt_header_data, const std::vector<char>& packed_trx_data);


      [[eosio::action]]
         void header( const std::vector<char>& init_block_header);

         [[eosio::action]]
         void ps( const producer_schedule_type& params);

         [[eosio::action]]
         void merkle( const incremental_merkle& params);

         [[eosio::action]]
         void merkleadd( const digest_type& params);

   private:
      digest_type       bhs_sig_digest(block_header_state hs)const;
      capi_public_key   get_produer_capi_public_key(uint64_t table_id, name producer);
      void              assert_producer_signature(const digest_type& digest, const capi_signature& signature, const capi_public_key& pub_key);

      void bhs_sign( block_header_state hs );
   };

} /// namespace eosio
