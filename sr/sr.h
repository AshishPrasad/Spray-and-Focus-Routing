#ifndef __sr_h__
#define __sr_h__

#include "sr_pkt.h"
#include "sr_rtable.h"
#include "sr_buffer.h"
#include "sr_constants.h"
#include <common/agent.h>
#include <common/packet.h>
#include <trace/trace.h>
#include <common/timer-handler.h>
#include <tools/random.h>
#include "classifier/classifier-port.h"
#include "classifier/classifier.h"
#include "trace/cmu-trace.h"

class Sr;

/*
class BufferRecord;
class PacketRecord;

LIST_HEAD(sr_buffer_cache,BufferRecord);    //sr_buffer_cache type represents a list of BufferRecord
LIST_HEAD(packet_list_cache, PacketRecord); //packet_list_cache represents a list of Packet
*/



/* Timers */

/*Timer to delete rotten entries in the sr_rtable rtable_*/
class Sr_Table_Timer : public TimerHandler {
public:
    Sr_Table_Timer(Sr* agent) : TimerHandler(){
        agent_ = agent;
    }
protected:
    Sr* agent_;
    virtual void expire(Event* e);
};



class Sr_Ctrl_PktTimer : public TimerHandler {
public:
    Sr_Ctrl_PktTimer(Sr* agent) : TimerHandler(){
        agent_ = agent;
    }
protected:
    Sr* agent_;
    virtual void expire(Event* e);
};


class Sr_Data_PktTimer : public TimerHandler {
public:
    Sr_Data_PktTimer(Sr* agent) : TimerHandler(){
        agent_ = agent;
    }
protected:
    Sr* agent_;
    virtual void expire(Event* e);
};




/* Agent */

class Sr: public Agent {

  /* Friends */
  friend class Sr_Table_Timer;
  friend class Sr_Ctrl_PktTimer;
  friend class Sr_Data_PktTimer;
  friend class Sr_Neighbor;
  
  /* Private members */
  nsaddr_t          ra_addr_;//This is set in the constructor of this class.
  sr_rtable         rtable_;
  int               accessible_var_;
  int               seq_num_;

protected:

    PortClassifier*     dmux_;          // For passing packets up to agents.
    Trace*              logtarget_;     // For logging.
    Sr_Ctrl_PktTimer     ctrl_pkt_timer_;     // Timer for sending control packets.
    Sr_Data_PktTimer     data_pkt_timer_;     // Timer for sending control packets.
    Sr_Table_Timer      table_timer_;         

    /*List to store the buffer entries.*/
    sr_buffer_cache bufferList;

    nsaddr_t&    ra_addr()       { return ra_addr_; }
    int    seq_num()             { return seq_num_ ;}

    void increment_seq_num()     { seq_num_ = seq_num_ + 1;}
    
    inline int&         accessible_var(){ return accessible_var_; }

    nsaddr_t forward_data_spray(Packet*);
    nsaddr_t forward_data_focus(Packet*);
    void recv_sr_pkt(Packet*);
    void recv_sr_control(Packet*);
    void recv_sr_data(Packet*);
    void send_sr_ctrl_pkt();
    void send_sr_data_pkt();
    void sr_clean_table();
    void print_buffer(Trace* out);
    
    void cache_in_buffer(Packet*);
    
    void reset_sr_ctrl_pkt_timer();
    void reset_sr_data_pkt_timer();
    void reset_sr_table_timer();

    bool vn_list_lookup(nsaddr_t, nsaddr_t, int);
    
    BufferRecord* buffer_lookup(nsaddr_t, int);

    /*Update the routing table based on the current values of time of encounter.*/
    void update_rtable(rtable_t*, nsaddr_t);  

public:
    Sr(nsaddr_t);
    int command(int, const char*const*);
    void recv(Packet*, Handler*);

};


#endif /* __sr_h__ */
