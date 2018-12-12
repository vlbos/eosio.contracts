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
         chaindb                 _chaindb;
         prodsches               _prodsches;
         sections                _sections;
         global_singleton        _global;
         global_state            _gstate;


      public:
         ibc( name s, name code, datastream<const char*> ds );
         ~ibc();

         [[eosio::action]]
         void setglobal( global_state gs);

         [[eosio::action]]
         void chaininit( const std::vector<char>&     header,
                         const producer_schedule&     active_schedule,
                         const incremental_merkle&    blockroot_merkle );

         [[eosio::action]]
         void newsection( const std::vector<char>&    header,
                          const incremental_merkle&   blockroot_merkle );

         [[eosio::action]]
         void addheader( const std::vector<char>& header );

         [[eosio::action]]
         void addheaders( const std::vector<char>& headers );

         [[eosio::action]]
         void deltable( name talbe, uint32_t num = 0, bool reverse = false );






         // -- ibc transaction related --

         /*[[eosio::action]] no need to export to abi.json, for this action only called by ibc.token contract */
         void ibctrxinfo( const uint32_t          block_time_slot,
                          const capi_checksum256& trx_id,
                          const name              from,
                          const name              to,
                          const asset             quantity,
                          const string&           memo );

         [[eosio::action]]
         void srcibctrx( const uint64_t             seq,
                         const uint32_t             src_block_num,
                         const capi_checksum256&    src_trx_id, // redundant, but used for make logic and index info more easy
                         const std::vector<char>&   src_packed_trx,
                         const std::vector<capi_checksum256>& src_merkle_path,
                         const name                 relay );

         [[eosio::action]]
         void destibctrx( const uint32_t             dest_block_num,
                          const std::vector<char>&   dest_packed_trx,
                          const std::vector<capi_checksum256>& dest_merkle_path,
                          const name                 relay );

//         [[eosio::action]]
//         void setskiptrxs( const std::vector<capi_checksum256>& should_skiped_src_ibc_trxs );
//
//
//         void lockall();
//         void unlockall();
//         void tmplock();


         // -- just for test --
         [[eosio::action]]
         void packedtrx( const std::vector<char>& trx_receipt_header_data, const std::vector<char>& packed_trx_data);


//         [[eosio::action]]
//         void header( const std::vector<char>& init_block_header);
//
//         [[eosio::action]]
//         void ps( const producer_schedule_type& params);
//
//         [[eosio::action]]
//         void merkle( const incremental_merkle& params);
//
//         [[eosio::action]]
//         void merkleadd( const digest_type& params);

   private:

      struct [[eosio::table("ibctrxs"), eosio::contract("ibc")]] ibctrx_info {
         uint64_t             id;   // auto-increment
         uint32_t             block_time_slot;
         capi_checksum256     trx_id;
         name                 from;
         name                 to;
         asset                quantity;
         string               memo;
         capi_checksum256     dest_trx_id;
         uint64_t             state;   // 0 init, 1 success, 2 failed, 3 rollbacked

         uint64_t primary_key()const { return id; }
      };
      eosio::multi_index< "ibctrxs"_n, ibctrx_info >  _ibctrxs;

      struct [[eosio::table("rmtlcltrxs"), eosio::contract("ibc")]] remote_local_trx_info {
         uint64_t             id;   // auto-increment
         capi_checksum256     r_trx_id;
         name                 r_from;
         name                 r_to;
         asset                r_quantity;
         string               r_memo;
         capi_checksum256     l_trx_id;
         name                 l_from;
         name                 l_to;
         asset                l_quantity;
         string               l_memo;

         uint64_t primary_key()const { return id; }
      };
      eosio::multi_index< "rmtlcltrxs"_n, remote_local_trx_info >  _rmtlcltrxs;



      digest_type       bhs_sig_digest( const block_header_state& hs )const;
      capi_public_key   get_producer_capi_public_key( uint64_t table_id, name producer );
      void              assert_producer_signature( const digest_type& digest, const capi_signature& signature, const capi_public_key& pub_key );
      void              pushheader( const signed_block_header& header);
      uint32_t          get_active_schedule_id( const section_type& section );
      };

} /// namespace eosio
