

typedef capi_checksum256   digest_type;
typedef capi_checksum256   block_id_type;
typedef capi_checksum256   transaction_id_type;
typedef capi_signature     signature_type;

struct packed_transaction {
   enum compression_type : uint8_t {
      none = 0,
      zlib = 1,
   };

   std::vector<signature_type>   signatures;
   compression_type              compression;
   std::vector<char>             packed_context_free_data;
   std::vector<char>             packed_trx;

   digest_type packed_digest()const;

//   time_point_sec     expiration()const;
//   transaction_id_type id()const;
//   transaction_id_type get_uncached_id()const;
//   bytes              get_raw_transaction()const;
//   vector<bytes>      get_context_free_data()const;
//   transaction        get_transaction()const;
//   signed_transaction get_signed_transaction()const;

private:
   mutable std::optional<transaction>           unpacked_trx; // <-- intermediate buffer used to retrieve values
   void local_unpack()const;
};


struct transaction_receipt_header {
   enum status_enum : uint8_t {
      executed  = 0, ///< succeed, no error handler executed
      soft_fail = 1, ///< objectively failed (not executed), error handler executed
      hard_fail = 2, ///< objectively failed and error handler objectively failed thus no state change
      delayed   = 3, ///< transaction delayed/deferred/scheduled for future execution
      expired   = 4  ///< transaction expired and storage space refuned to user
   };
   
   status_enum    status;
   uint32_t       cpu_usage_us;
   uint32_t       net_usage_words;
};

struct transaction_receipt : public transaction_receipt_header {
   
   packed_transaction trx;

   digest_type digest()const {
      datastream<char*> ds( result.data(), result.size() );
      ds << status;
      ds << cpu_usage_us;
      ds << net_usage_words;
      ds << trx.packed_digest();

      capi_checksum256 digest;

      sha256( trx_bytes.data(), trx_size, &digest );
      return digest;



      pack( enc, status );
      pack( enc, cpu_usage_us );
      pack( enc, net_usage_words );
      pack( enc, trx.packed_digest() );
      return enc.result();
   }
};






























