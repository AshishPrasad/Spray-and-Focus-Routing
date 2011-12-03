#ifndef __sr_buffer_h__
#define __sr_buffer_h__

#include "sr_pkt.h"
#include "sr_rtable.h"
#include "sr_constants.h"
#include <common/agent.h>
#include <common/packet.h>
#include <trace/trace.h>
#include <common/timer-handler.h>
#include <tools/random.h>
#include "classifier/classifier-port.h"


///*
class BufferRecord;
class PacketRecord;

LIST_HEAD(sr_buffer_cache,BufferRecord);    //sr_buffer_cache type represents a list of BufferRecord
LIST_HEAD(packet_list_cache, PacketRecord); //packet_list_cache represents a list of Packet
//*/


class PacketRecord{
public:
    Packet* packet;
    bool marked_pkt;
    PacketRecord(Packet* p){
        packet = p;
        marked_pkt = false;
    }
    LIST_ENTRY(PacketRecord) pkt_entry;
};




class BufferRecord{
    friend class Sr_Neighbor;
    friend class Sr;
    nsaddr_t source_addr_;
    int source_seq_num_;
    int num_copy_recvd_;


public:
    /*Packet List*/
    packet_list_cache pktList;

    /*Visited neighbors list head*/
    sr_ncache vnList;

    LIST_ENTRY(BufferRecord)record;

    BufferRecord(){
        source_addr_ = UNDEFINED;
        source_seq_num_ = UNDEFINED;
        num_copy_recvd_ = 0;        
        LIST_INIT(&pktList);
        LIST_INIT(&vnList);
    }

    ~BufferRecord() {
    }

    PacketRecord* get_pkt_record(){
        return pktList.lh_first;
    }

    void set_source_addr(nsaddr_t src_addr){source_addr_ = src_addr;}
    nsaddr_t get_source_addr(){return source_addr_;}

    void add_packet(Packet*);
    void add_neighbor(Sr_Neighbor*);

    void set_source_seq_num(int src_seq_no){source_seq_num_ = src_seq_no;}
    int get_source_seq_num(){return source_seq_num_;}

    void increment_num_copy_recvd(){num_copy_recvd_++;}
    void decrement_num_copy_recvd(){num_copy_recvd_--;}
    int& get_num_copy_recvd(){return num_copy_recvd_;}

    void set_bufferRecord(Packet*, nsaddr_t, u_int8_t, Sr_Neighbor*);
    /* This function is to be called only on the originating message node. */
    void init_bufferRecord(Packet*, nsaddr_t, int);
};

#endif	/* sr_buffer_h */

