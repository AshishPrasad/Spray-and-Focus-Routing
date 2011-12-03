#ifndef __sr_pkt_h__
#define __sr_pkt_h__

#include <common/packet.h>
#include "sr_rtable.h"

#define HDR_SR_PKT(p) hdr_sr_pkt::access(p)

/*Constants which denote the sh_type of the packet.*/
#define SR_CONTROL  	0x01
#define SR_DATA  	0x02

struct hdr_sr_pkt{    
    nsaddr_t pkt_src_;          // Packet source
    nsaddr_t pkt_prev_hop_;     // Packet previous hop
    nsaddr_t pkt_next_hop_;     // Packet next hop
    nsaddr_t pkt_dest_;         // Packet destination
    u_int16_t pkt_len_;         // Packet length (in bytes)
    int pkt_seq_num_;           // Packet sequence number
    
    rtable_t* nb_table_;    // table of neighbor

    /*Classifies the packet as data packet or control packet*/
    u_int8_t  sh_type_;

    inline nsaddr_t& pkt_src() { return pkt_src_; }
    inline nsaddr_t& pkt_prev_hop() { return pkt_prev_hop_; }
    inline nsaddr_t& pkt_next_hop() { return pkt_next_hop_; }
    inline nsaddr_t& pkt_dest() { return pkt_dest_; }
    
    inline u_int16_t& pkt_len() { return pkt_len_; }
    inline int& pkt_seq_num() { return pkt_seq_num_; }
    inline rtable_t*& nb_table() { return nb_table_; }
    inline u_int8_t& sh_type() { return sh_type_; }

    static int offset_;
    inline static int& offset() { return offset_; }
    inline static hdr_sr_pkt* access(const Packet* p) {
        return (hdr_sr_pkt*)p->access(offset_);
    }

    /* returns the fixed size */
    inline u_int16_t sr_hdr_size() {
       u_int16_t sz = 0;
      
            sz = sizeof(nsaddr_t)		// pkt_src_
                 + sizeof(nsaddr_t)		// pkt_prev_hop_
                 + sizeof(nsaddr_t)		// pkt_next_hop_
                 + sizeof(nsaddr_t)		// pkt_dest_                 
                 + sizeof(u_int16_t) 	        // pkt_len_
                 + sizeof(int)                  // pkt_seq_num_
                 + sizeof(rtable_t*)            // nb_table_ pointer
                 + sizeof(u_int8_t);            // sh_type_
                 
       assert (sz >= 0);
       return sz;
  }

};

#endif