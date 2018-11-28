/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include <ibc/chain.hpp>
#include <ibc/ibc.hpp>
#include "merkle.cpp"
#include "chain.cpp"

namespace eosio {

   ibc::ibc( name s, name code, datastream<const char*> ds ) :contract(s,code,ds),
       _producer_schedule(_self, _self.value),
       _incremental_merkle(_self, _self.value),
       _block_header_table(_self, _self.value)
   {
      _producer_schedule_state = _producer_schedule.exists() ? _producer_schedule.get() : producer_schedule_type{};
      _incremental_merkle_state = _incremental_merkle.exists() ? _incremental_merkle.get() : incremental_merkle{};
   }

   ibc::~ibc() {
      _producer_schedule.set( _producer_schedule_state, _self );
      _incremental_merkle.set( _incremental_merkle_state, _self );
   }

   void ibc::initchain( const std::vector<char>& init_block_header,
                        const producer_schedule_type& init_producer_schedule,
                        const incremental_merkle& init_incr_merkle ){

      const signed_block_header header = unpack<signed_block_header>(init_block_header);

      auto existing = _block_header_table.find( header.block_num() );

      if( existing == _block_header_table.end() ){
         _block_header_table.emplace( _self, [&]( auto& r ) {
            r.block_num = header.block_num();
            r.block_id  = header.id();
            r.header    = header;
            r.producer_signature = header.producer_signature;
         });
      }

      _producer_schedule_state = init_producer_schedule;
      _incremental_merkle_state = init_incr_merkle;
   }

   void ibc::addheaders( const std::vector<char>& block_headers){
      std::vector<signed_block_header> headers = unpack<std::vector<signed_block_header>>(block_headers);


   }

   void ibctrxinfo(  uint64_t    transfer_seq,
                     uint32_t    block_num,
                     capi_checksum256  trx_id,
                     name        from,
                     asset       quantity,
                     string      memo ){
      print(transfer_seq);
      print(block_num);
      print(from.to_string());
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

      _producer_schedule_state = params;
      _producer_schedule.set( _producer_schedule_state, _self );
      print(_producer_schedule_state.version);
   }

   void ibc::merkle( const incremental_merkle& params){
      print(params.node_count);
      print("---------------++++-------+++++++++++++++++++./");
      _incremental_merkle_state = params;
   }

   void ibc::merkleadd( const digest_type& params){

      print("input=");
      printhex(&params.hash,32);

      _incremental_merkle_state.append(params);
      print("  root = ");
      capi_checksum256 root = _incremental_merkle_state.get_root();
      printhex(&(root.hash),32);
   }

} /// namespace eosio

EOSIO_DISPATCH( eosio::ibc, (initchain)(addheaders)(ps)(header)(merkle)(merkleadd) )
