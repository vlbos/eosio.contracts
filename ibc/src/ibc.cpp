/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include <ibc/chain.hpp>
#include <ibc/ibc.hpp>
#include <ibc/types.hpp>
#include "merkle.cpp"
#include "chain.cpp"

namespace eosio {

   ibc::ibc( name s, name code, datastream<const char*> ds ) :contract(s,code,ds),
            _prodsches(_self, _self.value),
            _chaindb(_self, _self.value),
            _forkdb(_self, _self.value)
   {

   }

   ibc::~ibc() {

   }

   void ibc::forkdbinit( const std::vector<char>& init_block_header,
                        const producer_schedule& init_active_schedule,
                        const incremental_merkle& init_blockroot_merkle ){

      const signed_block_header header = unpack<signed_block_header>(init_block_header);

      auto schedule_id = _prodsches.available_primary_key();
      auto schedule_hash = get_checksum256(init_active_schedule);

      _prodsches.emplace( _self, [&]( auto& r ) {
         r.id              = schedule_id;
         r.schedule        = init_active_schedule;
         r.schedule_hash   = schedule_hash;
      });

      _forkdb.emplace( _self, [&]( auto& r ) {
         r.block_num = header.block_num();
         r.block_id  = header.id();
         r.header    = header;
         r.blockroot_merkle = init_blockroot_merkle;
//         r.pending_schedule_id = ;
         r.active_schedule_id = schedule_id;
//         r.confirm_count = ;
      });


   }































   void ibc::initchain( const std::vector<char>& init_block_header,
                        const producer_schedule_type& init_producer_schedule,
                        const incremental_merkle& init_incr_merkle ){

      const signed_block_header header = unpack<signed_block_header>(init_block_header);

//      auto existing = _block_header_table.find( header.block_num() );
//
//      if( existing == _block_header_table.end() ){
//         _block_header_table.emplace( _self, [&]( auto& r ) {
//            r.block_num = header.block_num();
//            r.block_id  = header.id();
//            r.header    = header;
//            r.producer_signature = header.producer_signature;
//         });
//      }
//
//      _producer_schedule_state = init_producer_schedule;
//      _incremental_merkle_state = init_incr_merkle;
   }

   void ibc::addheaders( const std::vector<char>& block_headers){
      std::vector<signed_block_header> headers = unpack<std::vector<signed_block_header>>(block_headers);


   }

   void ibc::ibctrxinfo(   uint64_t    transfer_seq,
                           uint32_t    block_time_slot,
                           capi_checksum256  trx_id,
                           name        from,
                           asset       quantity,
                           string      memo ){
      print(transfer_seq);print("===");
      print(block_time_slot);print("===");
      print(from.to_string());print("===");
      printhex(trx_id.hash,32);print("===");
   }

   void ibc::remoteibctrx( const uint32_t block_num,
                 const std::vector<char>& packed_trx,
                 const std::vector<capi_checksum256>& merkle_path){


   }

   void ibc::packedtrx(const std::vector<char>& trx_receipt_header_data, const std::vector<char>& packed_trx_data){
      const transaction_receipt_header trx_receipt_header = unpack<transaction_receipt_header>( trx_receipt_header_data );
      const packed_transaction packed_trx = unpack<packed_transaction>( packed_trx_data );

      transaction_receipt trx_rcpt{trx_receipt_header};
      trx_rcpt.trx = packed_trx;

      digest_type dg = trx_rcpt.digest();

      printhex(dg.hash,32);


//      auto dg = packed_trx.packed_digest();

//      printhex(dg.hash,32);


//      datastream<size_t> ps;
//      ps << trx_receipt_header.status << trx_receipt_header.cpu_usage_us << trx_receipt_header.net_usage_words;
//
//
//      const transaction trx = unpack<transaction>(packed_trx.packed_trx);
//
//      print(int(trxraw.ref_block_num));

      print("-2-");
   }




// for test

//   for(auto& h:headers)
//         print(h.block_num());

//   void ibc::addheaders( const std::vector<char>& block_headers){

//      printhex(block_headers.data(),block_headers.size());
//      std::vector<signed_block_header> headers = unpack<std::vector<signed_block_header>>(block_headers);
//      std::vector<signed_block_header> headers = unpack<std::vector<signed_block_header>>(block_headers.data(),block_headers.size());

//      std::vector<signed_block_header> headers;
//      datastream<const char*> ds(block_headers.data(),block_headers.size());
//
//      unsigned_int s;
//      ds >> s; print(s.value);
//      headers.resize(s.value);
//      ds >> headers[0];
//      print("---");print(ds.tellp());print("---");print(ds.remaining());print("---");
////      ds >> headers[1];
//
//      print(headers[0].block_num());
//      print(headers[1].block_num());
//      for( auto& i : headers )
//         ds >> i;

//
//
//      ds >> headers;

//
//      for(auto & h : headers){
//         print(h.block_num());print("//");
//      }

//   }


   void ibc::header( const std::vector<char>& init_block_header){

      datastream<const char*> ds(init_block_header.data(),init_block_header.size());
      signed_block_header header;
      ds >> header;
      print("---");print(ds.tellp());print("---");print(ds.remaining());print("---");



      printhex(init_block_header.data()+ds.tellp(),init_block_header.size()-ds.tellp());



      capi_checksum256 r = header.id();
      print("--id--");printhex(&(r.hash),32);


      print("--num--");print(header.block_num());

      r = header.digest();
      print("--dig--");printhex(&(r.hash),32);

      capi_signature rr = header.producer_signature;
      print("--sig--");printhex(&(rr.data), sizeof(capi_checksum256));

//
//      const signed_block_header header = unpack<signed_block_header>(init_block_header);
//
//
//
//
//
//
//
//
//
//      auto existing = _block_header_table.find( header.block_num() );
//
//      if( existing == _block_header_table.end() ){
//         _block_header_table.emplace( _self, [&]( auto& r ) {
//            r.block_num = header.block_num();
//            r.block_id  = header.id();
//            r.header    = header;
//            r.producer_signature = header.producer_signature;
//         });
//      }
//
//      return;
//
//      capi_checksum256 r = header.id();
//      print("--id--");printhex(&(r.hash),32);
//
//
//      print("--num--");print(header.block_num());
//
//      r = header.digest();
//      print("--dig--");printhex(&(r.hash),32);
//
//      r = header.digest();
//      print("--dig--");printhex(&(r.hash),32);
//
//      r = header.producer_signature;
//      print("--producer sig--");printhex(&(r.hash),32);

   }


   void ibc::ps( const producer_schedule_type& params){

      print("0000aa");

//      _producer_schedule_state = params;
//      _producer_schedule.set( _producer_schedule_state, _self );
//      print(_producer_schedule_state.version);
   }

   void ibc::merkle( const incremental_merkle& params){
      print(params.node_count);
      print("---------------++++-------+++++++++++++++++++./");
//      _incremental_merkle_state = params;
   }

   void ibc::merkleadd( const digest_type& params){

//      print("input=");
//      printhex(&params.hash,32);
//
//      _incremental_merkle_state.append(params);
//      print("  root = ");
//      capi_checksum256 root = _incremental_merkle_state.get_root();
//      printhex(&(root.hash),32);
   }

} /// namespace eosio

EOSIO_DISPATCH( eosio::ibc, (packedtrx)(forkdbinit)(initchain)(ibctrxinfo)(remoteibctrx)(addheaders)(ps)(header)(merkle)(merkleadd) )
