

namespace eosio{

   struct transfer_args{
      name    from;
      name    to;
      asset   quantity;
      string  memo;
   };

   void trim(string &s) {
      if( !s.empty() ) {
         s.erase(0,s.find_first_not_of(" "));
         s.erase(s.find_last_not_of(" ") + 1);
      }
   }

   capi_checksum256 get_trx_id(){
      capi_checksum256 trx_id;
      std::vector<char> trx_bytes;
      size_t trx_size = transaction_size();
      trx_bytes.resize(trx_size);
      read_transaction(trx_bytes.data(), trx_size);
      sha256( trx_bytes.data(), trx_size, &trx_id );
      return trx_id;
   }

   uint32_t get_block_time_slot(){
      return  ( current_time() / 1000 - 946684800000 ) / 500;
   }


}



