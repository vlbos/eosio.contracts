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
            _chaindb(_self, _self.value),
            _prodsches(_self, _self.value),
            _sections(_self, _self.value),
            _global(_self, _self.value)
   {
      _gstate = _global.exists() ? _global.get() : global_state{};
   }

   ibc::~ibc() {
      _global.set( _gstate, _self );
   }

   void ibc::setglobal( global_state gs){
      _gstate = gs;
   }

   /**
    * Notes
    * The chaininit block_header_state's pending_schedule version should equal to active_schedule version"
    */

   void ibc::chaininit( const std::vector<char>&      header_data,
                        const producer_schedule&      active_schedule,
                        const incremental_merkle&     blockroot_merkle) {
      const signed_block_header header = unpack<signed_block_header>(header_data);

      while ( _prodsches.begin() != _prodsches.end() ){ _prodsches.erase(_prodsches.begin()); }
      while ( _chaindb.begin() != _chaindb.end() ){ _chaindb.erase(_chaindb.begin()); }

      auto active_schedule_id = 1;
      _prodsches.emplace( _self, [&]( auto& r ) {
         r.id              = active_schedule_id;
         r.schedule        = active_schedule;
         r.schedule_hash   = get_checksum256(active_schedule);
      });

      auto block_signing_key = get_producer_capi_public_key( active_schedule_id, header.producer );
      auto header_block_num = header.block_num();

      block_header_state bhs;
      bhs.block_num             = header_block_num;
      bhs.block_id              = header.id();
      bhs.header                = header;
      bhs.active_schedule_id    = active_schedule_id;
      bhs.pending_schedule_id   = active_schedule_id;
      bhs.blockroot_merkle      = blockroot_merkle;
      bhs.block_signing_key     = block_signing_key;

      auto dg = bhs_sig_digest( bhs );
      assert_producer_signature( dg, header.producer_signature, block_signing_key );

      _chaindb.emplace( _self, [&]( auto& r ) {
         r = std::move( bhs );
      });

      section_type sct;
      sct.first              = header_block_num;
      sct.last               = header_block_num;
      sct.valid              = true;
      sct.add( header.producer, header_block_num );

      _sections.emplace( _self, [&]( auto& r ) {
         r = std::move( sct );
      });
   }

   /**
    * Notes
    * The newsection block_header_state should not have header.new_producers
    * and schedule_version not changed with valid last_section
    */
   void ibc::newsection(const std::vector<char>&   header_data,
                        const incremental_merkle&  blockroot_merkle){
      const signed_block_header header = unpack<signed_block_header>(header_data);
      eosio_assert( !header.new_producers, "section root header can not contain new_producers" );

      auto header_block_num = header.block_num();

      const auto& last_section = *(--_sections.end());
      eosio_assert( last_section.valid, "last_section is not valid" );
      eosio_assert( header_block_num > last_section.last + 1, "header_block_num should larger then last_section.last + 1" );

      auto active_schedule_id = get_active_schedule_id( last_section );
      auto version = _prodsches.get( active_schedule_id ).schedule.version;
      eosio_assert( header.schedule_version == version, "schedule_version not equal to previous one" );

      auto block_signing_key = get_producer_capi_public_key( active_schedule_id, header.producer );

      block_header_state bhs;
      bhs.block_num             = header_block_num;
      bhs.block_id              = header.id();
      bhs.header                = header;
      bhs.active_schedule_id    = active_schedule_id;
      bhs.pending_schedule_id   = active_schedule_id;
      bhs.blockroot_merkle      = blockroot_merkle;
      bhs.block_signing_key     = block_signing_key;

      auto dg = bhs_sig_digest( bhs );
      assert_producer_signature( dg, header.producer_signature, block_signing_key );

      _chaindb.emplace( _self, [&]( auto& r ) {
         r = std::move( bhs );
      });

      section_type sct;
      sct.first              = header_block_num;
      sct.last               = header_block_num;
      sct.valid              = false;
      sct.add( header.producer, header_block_num );

      _sections.emplace( _self, [&]( auto& r ) {
         r = std::move( sct );
      });
   }

   // 添加连续的bp是不同的，验证一个bp最多连续12个块
   // 设置sections 的 active_schedule_id
   void ibc::pushheader( const signed_block_header& header ){
      auto header_block_num = header.block_num();
      auto header_block_id = header.id();

      const auto& last_section_ptr = --_sections.end();
      auto last_section_first = last_section_ptr->first;
      auto last_section_last = last_section_ptr->last;
      eosio_assert( header_block_num > last_section_first, "new header number must larger then section root number" );
      eosio_assert( header_block_num <= last_section_last + 1, "unlinkable block" );

      // delete old branch
      if ( header_block_num < last_section_last + 1){
         auto result = _chaindb.get( header_block_num );
         eosio_assert( std::memcmp(header_block_id.hash, result.block_id.hash, 32) != 0, ("block repeated: " + std::to_string(header_block_num)).c_str()  );

         if ( header_block_num - last_section_first < _gstate.lib_depth ){
            _sections.modify( *last_section_ptr, same_payer, [&]( auto& r ) {
               r.valid = false;
            });
         }

         while ( ( --_chaindb.end() )->block_num != header_block_num - 1 ){
            _chaindb.erase( --_chaindb.end() );
         }
         print_f("-- block deleted: % to % --", last_section_last, header_block_num);
      }

      // verify new block
      auto last_hbs = *(--_chaindb.end());   // not use pointer or const, for it needs change
      eosio_assert(std::memcmp(last_hbs.block_id.hash, header.previous.hash, 32) == 0 , "unlinkable block" );

      block_header_state bhs;
      bhs.block_num           = header_block_num;
      bhs.block_id            = std::move(header_block_id);
      last_hbs.blockroot_merkle.append( last_hbs.block_id );
      bhs.blockroot_merkle = std::move(last_hbs.blockroot_merkle);

      if ( header.new_producers ){
         // 验证version + 1
         _sections.modify( *last_section_ptr, same_payer, [&]( auto& r ) {
            r.valid = false;
         });
         auto pending_schedule_id = _prodsches.available_primary_key();
         _prodsches.emplace( _self, [&]( auto& r ) {
            r.id              = pending_schedule_id;
            r.schedule        = *header.new_producers;
            r.schedule_hash   = get_checksum256( *header.new_producers );
         });

         bhs.pending_schedule_id = pending_schedule_id;
         bhs.active_schedule_id  = last_hbs.active_schedule_id;
      } else {
         if ( last_hbs.active_schedule_id == last_hbs.pending_schedule_id ){  //
            if ( header_block_num - last_section_first >= _gstate.lib_depth && !last_section_ptr->valid ){
               _sections.modify( *last_section_ptr, same_payer, [&]( auto& r ) {
                  r.valid = true;
               });
            }
            bhs.active_schedule_id  = last_hbs.active_schedule_id;
            bhs.pending_schedule_id = last_hbs.pending_schedule_id;
         } else { // producers replacement interval
            auto pending_schedule_version = _prodsches.get( last_hbs.pending_schedule_id ).schedule.version;
            if ( header.schedule_version == pending_schedule_version ){
               bhs.active_schedule_id  = last_hbs.pending_schedule_id;
               bhs.pending_schedule_id = last_hbs.pending_schedule_id;
            }
         }
      }

      bhs.header              = std::move(header);

      if ( bhs.header.producer == last_hbs.header.producer && bhs.active_schedule_id == last_hbs.active_schedule_id ){
         bhs.block_signing_key = std::move(last_hbs.block_signing_key);
      } else{
         bhs.block_signing_key = get_producer_capi_public_key( last_hbs.active_schedule_id, bhs.header.producer );
      }

      auto dg = bhs_sig_digest( bhs );
      assert_producer_signature( dg, bhs.header.producer_signature, bhs.block_signing_key);

      _chaindb.emplace( _self, [&]( auto& r ) {
         r = std::move(bhs);
      });

      _sections.modify( *last_section_ptr, same_payer, [&]( auto& r ) {
         r.last = header_block_num;
      });

      print_f("-- block add: % --", header_block_num);
   }


   void ibc::addheader( const std::vector<char>& header_data) {
      const signed_block_header header = unpack<signed_block_header>(header_data);
      pushheader(header);
   }

   void ibc::addheaders( const std::vector<char>& block_headers){
      std::vector<signed_block_header> headers = unpack<std::vector<signed_block_header>>(block_headers);
      print_f("total % blocks", headers.size());
      for ( const auto & header : headers ){
         print_f("pushing header: %", header.block_num());
         pushheader(header);
      }
   }

   void ibc::deltable( name talbe, uint32_t num, bool reverse ){
      // todo
   }








   // private methods
   void ibc::assert_producer_signature(const digest_type& digest,
                                       const capi_signature& signature,
                                       const capi_public_key& pub_key ){
      assert_recover_key( reinterpret_cast<const capi_checksum256*>( digest.hash ),
                          reinterpret_cast<const char*>( signature.data ), 66,
                          reinterpret_cast<const char*>( pub_key.data ), 34 );
   }

   capi_public_key ibc::get_producer_capi_public_key( uint64_t id, name producer ){
      auto it = _prodsches.find(id);
      eosio_assert( it != _prodsches.end(), "producer schedule id not found" );
      const producer_schedule& ps = it->schedule;
      for( auto pk : ps.producers){
         if( pk.producer_name == producer){
            capi_public_key cpk;
            eosio::datastream<char*> pubkey_ds( reinterpret_cast<char*>(cpk.data), sizeof(capi_signature) );
            pubkey_ds << pk.block_signing_key;
            return cpk;
         }
      }
      eosio_assert(false, "producer not found" );
      return capi_public_key(); //never excute, just used to suppress "no return" warning
   }

   digest_type ibc::bhs_sig_digest( const block_header_state& hs ) const {
      auto it = _prodsches.find( hs.pending_schedule_id );
      eosio_assert( it != _prodsches.end(), "internal error: block_header_state::sig_digest" );
      auto header_bmroot = get_checksum256( std::make_pair( hs.header.digest(), hs.blockroot_merkle.get_root() ));
      return get_checksum256( std::make_pair( header_bmroot, it->schedule_hash ));
   }

   uint32_t ibc::get_active_schedule_id( const section_type& section ){
      return _chaindb.get( section.last ).active_schedule_id;
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



} /// namespace eosio

//  (ps)(header)(merkle)(merkleadd)

EOSIO_DISPATCH( eosio::ibc, (chaininit)(newsection)(addheader)(addheaders)(packedtrx)(ibctrxinfo)(remoteibctrx))

















//
//      void ibc::addheader2( const std::vector<char>& header_data){
//
//      capi_checksum256 header_digest;
//      sha256( header_data.data(), header_data.size(), &header_digest );
//
//      const signed_block_header header = unpack<signed_block_header>(header_data);
//
//      _chaindb.erase(--_chaindb.end());
//
//      auto last_hbs = *(--_chaindb.end());
//
//      auto header_block_num = header.block_num();
//      auto header_block_id = header.id();
//      eosio_assert(last_hbs.block_num + 1 == header_block_num, "can not linked block num" );
//
//
//      block_header_state bhs;
//      bhs.block_num = header_block_num;
//      bhs.block_id = std::move(header_block_id);
//      bhs.header = std::move(header);
//      bhs.active_schedule_id = last_hbs.active_schedule_id;
//      bhs.pending_schedule_id = last_hbs.pending_schedule_id;
//
//      last_hbs.blockroot_merkle.append( last_hbs.block_id );
//      bhs.blockroot_merkle = std::move(last_hbs.blockroot_merkle);
//
//      if ( bhs.header.producer == last_hbs.header.producer ){
//         bhs.block_signing_key = last_hbs.block_signing_key;
//      } else{
//         bhs.block_signing_key = get_producer_capi_public_key( last_hbs.active_schedule_id, header.producer );
//      }
//
//      _chaindb.emplace( _self, [&]( auto& r ) {
//         r=bhs;
//      });
//
//      auto dg = bhs_sig_digest( bhs);
//      assert_producer_signature( dg, bhs.header.producer_signature, bhs.block_signing_key);
//
//
//   }
//
//   void ibc::addheaders( const std::vector<char>& block_headers){
//      std::vector<signed_block_header> headers = unpack<std::vector<signed_block_header>>(block_headers);
//
//
//   }
//













//
//
//   void ibc::initchain( const std::vector<char>& init_block_header,
//                        const producer_schedule_type& init_producer_schedule,
//                        const incremental_merkle& init_incr_merkle ){
//
//      const signed_block_header header = unpack<signed_block_header>(init_block_header);

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
//   }



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


//   void ibc::header( const std::vector<char>& init_block_header){
//
//      datastream<const char*> ds(init_block_header.data(),init_block_header.size());
//      signed_block_header header;
//      ds >> header;
//      print("---");print(ds.tellp());print("---");print(ds.remaining());print("---");
//
//
//
//      printhex(init_block_header.data()+ds.tellp(),init_block_header.size()-ds.tellp());
//
//
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
//      capi_signature rr = header.producer_signature;
//      print("--sig--");printhex(&(rr.data), sizeof(capi_checksum256));

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

//   }


//   void ibc::ps( const producer_schedule_type& params){
//
//      print("0000aa");

//      _producer_schedule_state = params;
//      _producer_schedule.set( _producer_schedule_state, _self );
//      print(_producer_schedule_state.version);
//   }

//   void ibc::merkle( const incremental_merkle& params){
//      print(params._node_count);
//      print("---------------++++-------+++++++++++++++++++./");
//      _incremental_merkle_state = params;
//   }
//
//   void ibc::merkleadd( const digest_type& params){

//      print("input=");
//      printhex(&params.hash,32);
//
//      _incremental_merkle_state.append(params);
//      print("  root = ");
//      capi_checksum256 root = _incremental_merkle_state.get_root();
//      printhex(&(root.hash),32);
//   }
