/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosiolib/transaction.hpp>
#include <eosiolib/varint.hpp>
#include <any>

namespace eosio {

   typedef capi_checksum256   digest_type;
   typedef capi_checksum256   block_id_type;
   typedef capi_checksum256   transaction_id_type;
   typedef capi_signature     signature_type;

   template<typename T>
   void push(T&){}

   template<typename Stream, typename T, typename ... Types>
   void push(Stream &s, T arg, Types ... args){
      s << arg;
      push(s, args...);
   }

   template<class ... Types> capi_checksum256 get_checksum256( Types ... args ){
      datastream <size_t> ps;
      push(ps, args...);
      size_t size = ps.tellp();

      std::vector<char> result;
      result.resize(size);

      datastream<char *> ds(result.data(), result.size());
      push(ds, args...);
      capi_checksum256 digest;
      sha256(result.data(), result.size(), &digest);
      return digest;
   }

   struct packed_transaction {
      enum compression_type : uint8_t {
         none = 0,
         zlib = 1,
      };

      std::vector<signature_type>   signatures;
      compression_type              compression;
      std::vector<char>             packed_context_free_data;
      std::vector<char>             packed_trx;

      digest_type packed_digest() const {
         auto digest = get_checksum256( signatures, packed_context_free_data );
         return get_checksum256( compression, packed_trx, digest );
      }

//   time_point_sec     expiration()const;
//   transaction_id_type id()const;
//   transaction_id_type get_uncached_id()const;
//   bytes              get_raw_transaction()const;
//   vector<bytes>      get_context_free_data()const;
//   transaction        get_transaction()const;
//   signed_transaction get_signed_transaction()const;
   };


   struct transaction_receipt_header {
      enum status_enum : uint8_t {
         executed = 0,      ///< succeed, no error handler executed
         soft_fail = 1,     ///< objectively failed (not executed), error handler executed
         hard_fail = 2,     ///< objectively failed and error handler objectively failed thus no state change
         delayed = 3,       ///< transaction delayed/deferred/scheduled for future execution
         expired = 4        ///< transaction expired and storage space refuned to user
      };

      status_enum    status;
      uint32_t       cpu_usage_us;
      uint8_t        net_usage_words;
   };

//
//   struct transaction_receipt : public transaction_receipt_header {
//
//
//      packed_transaction trx;
//
//
//      digest_type digest() const {
//         datastream<size_t> ps;
//         ps << status << cpu_usage_us << net_usage_words << std::get<packed_transaction>(trx).packed_digest();
//         size_t size = ps.tellp();
//
//         std::vector<char> result;
//         result.resize(size);
//         datastream<char *> ds(result.data(),result.size());
//         ds << status;
//         ds << cpu_usage_us;
//         ds << net_usage_words;
//         ds << std::get<packed_transaction>(trx).packed_digest();
//
//         capi_checksum256 digest;
//
//         sha256(result.data(), result.size(), &digest);
//         return digest;
//      }
//   };



}

























