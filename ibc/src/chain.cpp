/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <ibc/types.hpp>

namespace eosio {

   inline uint64_t endian_reverse_u64( uint64_t x )
   {
      return (((x >> 0x38) & 0xFF)        )
             | (((x >> 0x30) & 0xFF) << 0x08)
             | (((x >> 0x28) & 0xFF) << 0x10)
             | (((x >> 0x20) & 0xFF) << 0x18)
             | (((x >> 0x18) & 0xFF) << 0x20)
             | (((x >> 0x10) & 0xFF) << 0x28)
             | (((x >> 0x08) & 0xFF) << 0x30)
             | (((x        ) & 0xFF) << 0x38)
         ;
   }

   inline uint32_t endian_reverse_u32( uint32_t x )
   {
      return (((x >> 0x18) & 0xFF)        )
             | (((x >> 0x10) & 0xFF) << 0x08)
             | (((x >> 0x08) & 0xFF) << 0x10)
             | (((x        ) & 0xFF) << 0x18)
         ;
   }

   digest_type block_header::digest()const
   {
      std::vector<char> buf = pack(*this);
      ::capi_checksum256 hash;
      ::sha256( reinterpret_cast<char*>(buf.data()), buf.size(), &hash );
      return hash;
   }

   uint32_t block_header::num_from_id(const block_id_type& id)
   {
      return endian_reverse_u32(*(uint64_t*)(id.hash));
   }

   block_id_type block_header::id()const
   {
      union {
         block_id_type result;
         uint64_t hash64[4];
      }u;

      u.result = digest();
      u.hash64[0] &= 0xffffffff00000000;
      u.hash64[0] += endian_reverse_u32(block_num());
      return u.result;
   }

#define BIGNUM  2000
#define MAXSPAN 4
   void section_type::add( name pro, uint32_t num, const producer_schedule& sch ){
      if ( producers.empty() ){
         eosio_assert( block_nums.empty(), "producers not consistent with block_nums" );
         eosio_assert( pro.value != 0 && num != 0, "invalid parameters" );
         producers.push_back( pro );
         block_nums.push_back( num );
         return;
      }

      if( pro == producers.back() ){
         return;
      } else {
         eosio_assert( sch.producers.size() > 15, "less then 15 producers" );
         int index_last = BIGNUM;
         int index_this = BIGNUM;
         int i = 0;
         for ( const auto& pk : sch.producers ){
            if ( pk.producer_name == producers.back() ){
               index_last = i;
            }
            if ( pk.producer_name == pro ){
               index_this = i;
            }
            ++i;
         }
         if ( index_this > index_last ){
            eosio_assert( index_this - index_last <= MAXSPAN, "exceed max span" );
         } else {
            eosio_assert( index_last - index_this >= sch.producers.size() - MAXSPAN, "exceed max span" );
         }
      }

      if( producers.size() > 21 ){
         producers.erase( producers.begin() );
         block_nums.erase( block_nums.begin() );
      }

      eosio_assert( num <= block_nums.back() + 12 , "one producer can not produce more then 12 blocks continue" );



      int size = producers.size();
      int i = size >= 15 ? 15 : size;
      i -= 1;
      while ( i >= 0 ){
         eosio_assert( pro != producers[i] , "producer can not repeat within last 15 producers" );
         --i;
      }

      producers.push_back( pro );
      block_nums.push_back( num );
   }

   void section_type::clear_from( uint32_t num ){
      int pos = 0;
      eosio_assert( first < num && num <= last , "invalid number" );

      while ( num <= block_nums.back() ){
         producers.pop_back();
         block_nums.pop_back();
      }
   }










} /// namespace eosio