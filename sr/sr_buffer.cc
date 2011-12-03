#include "sr_buffer.h"

void BufferRecord::add_packet(Packet* p){
    PacketRecord* pkt_record = new PacketRecord(p);
    LIST_INSERT_HEAD(&pktList, pkt_record, pkt_entry);
}


void BufferRecord::add_neighbor(Sr_Neighbor* nb){
    LIST_INSERT_HEAD(&vnList, nb, nb_link);
}

void BufferRecord::set_bufferRecord(Packet* p, nsaddr_t src_addr, u_int8_t src_seq_no, Sr_Neighbor* nb){
    PacketRecord* pkt_record = new PacketRecord(p);
    LIST_INSERT_HEAD(&pktList, pkt_record, pkt_entry);
    source_addr_ = src_addr;
    source_seq_num_ = src_seq_no;
    num_copy_recvd_ += 1;        
    LIST_INSERT_HEAD(&vnList, nb, nb_link);
}

/* This function is to be called only on the originating message node. */
void BufferRecord::init_bufferRecord(Packet* p, nsaddr_t src_addr, int src_seq_no){
    PacketRecord* pkt_record;

    source_addr_ = src_addr;
    source_seq_num_ = src_seq_no;
    num_copy_recvd_ = SR_L;

    int i = 0;
    for(i = 0; i < SR_L; i++)
    {
        pkt_record = new PacketRecord(p->copy());
        LIST_INSERT_HEAD(&pktList, pkt_record, pkt_entry);
    }    
}