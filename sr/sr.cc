#include "sr.h"
#include "sr_rtable.h"
#include "aomdv/aomdv_rtable.h"
#include "common/ip.h"
#include "routing/rttable.h"
#include <iostream>

int hdr_sr_pkt::offset_;
static class SrHeaderClass : public PacketHeaderClass {
public:
        SrHeaderClass() : PacketHeaderClass("PacketHeader/Sr",
                                              sizeof(hdr_sr_pkt)) {
	  bind_offset(&hdr_sr_pkt::offset_);
	} 
} class_rtProtoSr_hdr;


static class SrClass : public TclClass {
public:
        SrClass() : TclClass("Agent/Sr") {}
        TclObject* create(int argc, const char*const* argv) {
          assert(argc == 5);
      	  return (new Sr((nsaddr_t) Address::instance().str2addr(argv[4])));
        }
} class_rtProtoSr;



/* Function to be executed at regular intervals for cleaning rotten entries from
 * the routing table of this node.
 * 
 * Interval length: IEXPIRE
 */
void Sr_Table_Timer::expire(Event* e){    
    agent_->sr_clean_table();
    agent_->reset_sr_table_timer();
}

void Sr::sr_clean_table(){
    for (rtable_t::iterator it = rtable_.rt_.begin(); it != rtable_.rt_.end(); it++) {
        if((*it).second.time_expire <= CURRENT_TIME)
            rtable_.rm_entry((*it).first);
    }
}


/* Function to be executed at regular intervals for broadcasting control packets
 * at regular intervals from this node.
 * 
 * Interval length: ICTRL
 */
void Sr_Ctrl_PktTimer::expire(Event* e){

    agent_->send_sr_ctrl_pkt();
    agent_->reset_sr_ctrl_pkt_timer();
}

/* Function to be executed at regular intervals for routing data packets collected
 * in the buffer of this node to their respective destinations.
 *
 * Interval length: IDATA
 */
void Sr_Data_PktTimer::expire(Event* e){    
    
    agent_->send_sr_data_pkt();
    agent_->reset_sr_data_pkt_timer();
}

/*This constructor is invoked once during node creation*/

Sr::Sr(nsaddr_t id) : Agent(PT_SR), data_pkt_timer_(this), ctrl_pkt_timer_(this),table_timer_(this) {
 bind_bool("accessible_var_", &accessible_var_);
 ra_addr_ = id;
 LIST_INIT(&bufferList);
 seq_num_ = 0;
}

/*Reset the timers for next occurrence*/
void Sr::reset_sr_ctrl_pkt_timer() {
    ctrl_pkt_timer_.resched((double)ICTRL);
}

void Sr::reset_sr_data_pkt_timer() {
    data_pkt_timer_.resched((double)IDATA);
}

void Sr::reset_sr_table_timer() {
    /*IEXPIRE:  abuse of notation*/
    table_timer_.resched((double)IEXPIRE);
}

/* Function to search the bufferList at this node and find the message identified by
 * the root IP address        = root_src and the
 * the root sequence number   = root_seq
 */
BufferRecord* Sr:: buffer_lookup(nsaddr_t root_src,int root_seq){
    BufferRecord *bf = bufferList.lh_first;
    for(; bf; bf = bf->record.le_next) {
        if((bf->get_source_addr() == root_src) & (bf->get_source_seq_num() == root_seq)){
            return bf;
        }
    }
    return NULL;
}

/* Function to search the vnList attached with the BufferRecord identified by the
 * the root IP address        = root_src and the
 * the root sequence number   = root_seq
 *
 * the function returns true if nb_address is already present in this vnList
 */
bool Sr::vn_list_lookup(nsaddr_t nb_address, nsaddr_t root_src, int root_seq){
    BufferRecord *bf = buffer_lookup(root_src, root_seq);

    if(bf){
        Sr_Neighbor *nb = bf->vnList.lh_first;
        for(; nb; nb = nb->nb_link.le_next){
            if (nb->nb_addr == nb_address)
                return false;
        }
        return true;
    }
    else{
        return false;
    }
}


/*
 * Updates the routing table associated with this node based on the latest values
 * of time of encounter.
 */
void Sr::update_rtable(rtable_t* nbtable, nsaddr_t nb_ip){

    /*Neighbor's table*/
    rtable_t nb_table = *nbtable;

    /* Traverse over the neighbor's table. Here table is actually the map in the neighbor's table */
    for (rtable_t::iterator nb_it = nb_table.begin(); nb_it != nb_table.end(); nb_it++) {
        nsaddr_t id = (*nb_it).first;

        /* If this entry corresponds to the current node, do nothing
         * because a node does not maintain information about itself
         */
        if(id == ra_addr())
            continue;

        /*Find the ip of this entry in the current node's table*/
        bool exists = rtable_.lookup_entry(id);



        /*If this ip is not stored previously in the current node's table, insert it*/
        if (!exists){
            rtable_.add_entry(id,nb_ip,CURRENT_TIME,CURRENT_TIME, CURRENT_TIME + IEXPIRE);
        }
        else{
            /*t_elapsed between successive encounters of 'nb' with 'id'*/
            double t_nb_id = (*nb_it).second.time_elapsed;

            /*t_elapsed between successive encounters of 'this' node with 'nb'*/
            double t_this_nb = rtable_.rt_[nb_ip].time_elapsed;

            /*Existing value of t_elapsed between successive encounters of 'this' node with 'id'*/
            double t_this_id = rtable_.rt_[id].time_elapsed;

            //CRUX OF THE PROTOCOL
            if( t_this_id > t_nb_id + t_this_nb + IDATA){
                rtable_.rt_[id].time_elapsed = t_nb_id + t_this_nb + IDATA;
                rtable_.rt_[id].time_of_encounter = UNDEFINED;
                rtable_.rt_[id].time_expire = CURRENT_TIME + IEXPIRE;

                /*Now the next hop to destination 'id' is 'nb'*/
                rtable_.rt_[id].next_hop = nb_ip;
            }
        }
    }
}



int Sr::command(int argc, const char*const* argv) {    
    if (argc == 2) {
        if (strcasecmp(argv[1],"start") == 0) {
            ctrl_pkt_timer_.resched(0.0);
            data_pkt_timer_.resched(0.0);
            table_timer_.resched(0.0);
            return TCL_OK;
        }

        else if (strcasecmp(argv[1],"print_buffer") == 0) {
            if (logtarget_ != 0) {
                sprintf(logtarget_->pt_->buffer(), "P %f _%d_ Buffer Table",
                        CURRENT_TIME,
                        ra_addr());
                logtarget_->pt_->dump();
                print_buffer(logtarget_);
            }
            else {
                fprintf(stdout, "%f _%d_ If you want to print this routing table "
                    "you must create a trace file in your tcl script",
                    CURRENT_TIME,
                    ra_addr());
            }
            return TCL_OK;            
        }
        
        else if (strcasecmp(argv[1],"print_rtable") == 0) {
            if (logtarget_ != 0) {
                sprintf(logtarget_->pt_->buffer(), "P %f _%d_ Routing Table",
                        CURRENT_TIME,
                        ra_addr());
                logtarget_->pt_->dump();
                rtable_.print(logtarget_);
            }
            else {
                fprintf(stdout, "%f _%d_ If you want to print this routing table "
                    "you must create a trace file in your tcl script",
                    CURRENT_TIME,
                    ra_addr());
            }
            return TCL_OK;
        }         
      }
    
      else if (argc == 3) {        
        // Obtains corresponding dmux to carry packets to upper layers
        if (strcmp(argv[1], "port-dmux") == 0) {
            dmux_ = (PortClassifier*)TclObject::lookup(argv[2]);
            if (dmux_ == 0) {
                fprintf(stderr, "%s: %s lookup of %s failed\n",
                    __FILE__,
                    argv[1],
                    argv[2]);
                return TCL_ERROR;
            }
            return TCL_OK;
        }
        // Obtains corresponding tracer
        else if (strcmp(argv[1], "log-target") == 0 ||
            strcmp(argv[1], "tracetarget") == 0) {
            logtarget_ = (Trace*)TclObject::lookup(argv[2]);
            if (logtarget_ == 0)
                return TCL_ERROR;
            return TCL_OK;
        }
    }
   // Pass the command to the base class
   return Agent::command(argc, argv);
}


/* Receive */
void Sr::recv(Packet* p, Handler* h) {

    struct hdr_cmn* ch = HDR_CMN(p);
    struct hdr_ip* ih  = HDR_IP(p);


    if (ih->saddr() == ra_addr()){
        // If there exists a loop, must drop the packet
        if (ch->num_forwards() > 0) {
            drop(p, DROP_RTR_ROUTE_LOOP);
            return;
        }
        // else if this is a packet I am originating, must add IP header
        else if (ch->num_forwards() == 0){
            cache_in_buffer(p);
            return;
        }
    }
    
    // If it is a sr packet, must process it
    if (ch->ptype() == PT_SR){
        recv_sr_pkt(p);
    }

    
    // Otherwise, must forward the packet (unless TTL has reached zero)
    else {
        ih->ttl_--;
        
        if (ih->ttl_ == 0) {
            drop(p, DROP_RTR_TTL);             
            return;
        }
    }
}


void Sr::recv_sr_pkt(Packet* p) {
    //struct hdr_ip* ih = HDR_IP(p);
    struct hdr_sr_pkt* sh = HDR_SR_PKT(p);

    /* All routing messages are sent from and to port RT_PORT,
       so we check it.*/
    assert(ih->sport() == RT_PORT);
    assert(ih->dport() == RT_PORT);

    /* ... processing of sr packet ... */
     switch(sh->sh_type_) {

     case SR_CONTROL:
        recv_sr_control(p);
        break;

     case SR_DATA:
        recv_sr_data(p);
        break;
        
     default:
        fprintf(stderr, "Invalid Sr type (%x)\n", sh->sh_type_);
        exit(1);
     }
}

/* Function to process a received control packet.
 * A control packet has its sh_type_ set to SR_CONTROL
 * Update the routing table based on the current updates.
 */
void Sr::recv_sr_control(Packet *p) {
    struct hdr_ip* ih = HDR_IP(p);
    struct hdr_sr_pkt* sh = HDR_SR_PKT(p);

    nsaddr_t id = ih->saddr();

    /* Search whether the neighbor from which this ctrl packet has been received
     * exists in the rtable of the current node and accordingly update its values.
     */
    bool exists = rtable_.lookup_entry(id);
    
    if (!exists){
        rtable_.add_entry(id, id, CURRENT_TIME, CURRENT_TIME, CURRENT_TIME + IEXPIRE);
    }
    else{
        /*deltaT denotes time between successive encounters*/
        double deltaT = CURRENT_TIME - rtable_.rt_[id].time_of_encounter;
        
        double t_elapsed = BETA*(deltaT)+(1-BETA)*rtable_.rt_[id].time_elapsed;

        rtable_.rt_[id].time_elapsed = t_elapsed;
        rtable_.rt_[id].time_of_encounter = CURRENT_TIME;
        rtable_.rt_[id].time_expire = CURRENT_TIME + IEXPIRE;
        rtable_.rt_[id].next_hop = id; //A neighbor's next hop is the neighbor itself.
    }

    
    //Update the routing table. id is the ip address of the neighbor.
    update_rtable(sh->nb_table_, id);

    /*Release resources */
    Packet::free(p);
}





/* Dumps the contents of the bufferList of this node*/
void Sr::print_buffer(Trace* out){
    sprintf(out->pt_->buffer(), "P\tsource_addr \tsource_seq_num \tnum_of_copies");
    out->pt_->dump();

     BufferRecord *bf = bufferList.lh_first;     
     for(; bf; bf = bf->record.le_next) {
            sprintf(out->pt_->buffer(), "P\t%d\t\t%d\t\t%d",
                    bf->get_source_addr(),
                    bf->get_source_seq_num(),
                    bf->get_num_copy_recvd()
                   );
            out->pt_->dump();
     }
}    



/* A data packet originating at this node first goes into the buffer at this node.
 * cache_in_buffer() stores the packet in the buffer.
 */
void Sr::cache_in_buffer(Packet* p){
    struct hdr_ip* ih = HDR_IP(p);
    struct hdr_cmn* ch = HDR_CMN(p);

    struct hdr_sr_pkt* sh   = HDR_SR_PKT(p);

    
    sh->pkt_len()        = sh->sr_hdr_size();
    increment_seq_num();
    sh->pkt_seq_num()    = seq_num();
    sh->nb_table_        = &rtable_.rt_;       /*Broadcast the current node's rtable_t (which is of type map).*/
    sh->sh_type_         = SR_DATA;            /*Set packet type to SR_DATA*/
    sh->pkt_src()        = ra_addr();
    sh->pkt_dest()       = ih->daddr();    

    ch->ptype()     = PT_SR;

    ch->size()      = IP_HDR_LEN + sh->pkt_len() + DATA_SIZE;//p->datalen();
    

    BufferRecord *buf = new BufferRecord(); // creating a new buffer record
    assert(buf);
    
    buf->init_bufferRecord(p, ih->saddr(), sh->pkt_seq_num());    
    LIST_INSERT_HEAD(&bufferList, buf, record);        
}


void Sr::recv_sr_data(Packet *p) {
    struct hdr_ip* ih = HDR_IP(p);
    struct hdr_cmn *ch = HDR_CMN(p);
    struct hdr_sr_pkt* sh   = HDR_SR_PKT(p);
    

    int seq_num = sh->pkt_seq_num();
    nsaddr_t prev_hop = ch->prev_hop_;
    nsaddr_t source_id = ih->saddr();       

    BufferRecord *bf = buffer_lookup(source_id, seq_num);

    /* If no previous entry exists for the given (source_id, seq_num) pair in the bufferList */
    if (bf == NULL){
        Sr_Neighbor* nb = new Sr_Neighbor(prev_hop); //creating a new neighbour
        assert(nb);
        BufferRecord* buf = new BufferRecord(); // creating a new buffer record
        assert(buf);
        buf->set_bufferRecord(p,source_id,seq_num,nb); //messages not routed back to the node from where it came
        LIST_INSERT_HEAD(&bufferList, buf, record);
    }

    /* Buffer Entry exists for the given (source_id, seq_num) pair */
    else{
        assert(bf);        
        bf->increment_num_copy_recvd(); //increasing the number of copies received.
        bf->add_packet(p);

        /* indicates that the earlier copies of the data had not come from prev hop. */
        int c = -1; 

        Sr_Neighbor *nb = bf->vnList.lh_first;
        
        for(; nb; nb = nb->nb_link.le_next) {
            if(nb->nb_addr == prev_hop){
                c = 1;
                break;
            }                
        }


        if (c == -1){
            Sr_Neighbor *new_nb = new Sr_Neighbor(prev_hop);
            assert(new_nb);
            LIST_INSERT_HEAD(&bf->vnList, new_nb, nb_link);
        }

        else if (c == 1){
          //do nothing
        }
    }    
}




/*Function to send control packets*/
void Sr::send_sr_ctrl_pkt() {
    Packet* p               = allocpkt();
    struct hdr_cmn* ch      = HDR_CMN(p);
    struct hdr_ip* ih       = HDR_IP(p);
    struct hdr_sr_pkt* sh   = HDR_SR_PKT(p);

    
    sh->pkt_len()        = sh->sr_hdr_size();    /*Pkt_length of sr*/
    sh->nb_table_        = &rtable_.rt_;         /*Broadcast the current node's rtable_t (which is of type map).*/
    sh->sh_type_         = SR_CONTROL;           /*Set packet type to SR_CONTROL*/
    sh->pkt_src()        = ra_addr();
    sh->pkt_next_hop()   = IP_BROADCAST;
    sh->pkt_prev_hop()   = ra_addr();
    sh->pkt_dest()       = IP_BROADCAST;

    ch->ptype()     = PT_SR;
    ch->direction() = hdr_cmn::DOWN;
    ch->size()      = IP_HDR_LEN + sh->pkt_len();
    ch->error()     = 0;
    ch->prev_hop_   = ra_addr();
    ch->next_hop()  = IP_BROADCAST;
    ch->addr_type() = NS_AF_INET;

    ih->saddr() = ra_addr();
    ih->daddr() = IP_BROADCAST;
    ih->sport() = RT_PORT;
    ih->dport() = RT_PORT;
    ih->ttl()   = 1;

    Scheduler::instance().schedule(target_, p, JITTER);
}




/*Function to send data packets*/
void Sr::send_sr_data_pkt() {    
    BufferRecord* buf = bufferList.lh_first;
    BufferRecord* next_buf;
    PacketRecord* pkt_record;
    PacketRecord* next_pkt_record;
    Packet* p;
    nsaddr_t next_visited_hop;
    bool flag;

    for(; buf; buf = next_buf) {        
            next_buf = buf->record.le_next;                       

            next_visited_hop = UNDEFINED;            
            flag = false;
            
            pkt_record = buf->pktList.lh_first;

            if(buf->get_num_copy_recvd() > 1){                
                int n = buf->get_num_copy_recvd()/2;

                for (int i = 1; i <= n; i++){                    
                    next_pkt_record = pkt_record->pkt_entry.le_next;
                    p = pkt_record->packet;

                    struct hdr_cmn* ch      = HDR_CMN(p);
                    struct hdr_ip* ih       = HDR_IP(p);


                    ch->direction() = hdr_cmn::DOWN;
                    ch->error()     = 0;
                    ch->addr_type() = NS_AF_INET;


                    ih->sport() = RT_PORT;
                    ih->dport() = RT_PORT;
                    ih->ttl()   = TTL_VALUE;    //IP_DEF_TTL;

                    
             /*If the destination is directly in the range, send only one packet and delete the buffer record*/
                    next_visited_hop = rtable_.lookup(ih->daddr());
    //              /*
                    if(next_visited_hop == ih->daddr()){
                        flag = true;
                        next_visited_hop = forward_data_focus(p);
                        break;                        
                    }                   
      //         */
                    next_visited_hop = forward_data_spray(p);

                    if(next_visited_hop != UNDEFINED){
                        buf->decrement_num_copy_recvd();
                        LIST_REMOVE(pkt_record, pkt_entry);
                        delete pkt_record;
                    }
                    pkt_record = next_pkt_record;                    
                }

                if (next_visited_hop != UNDEFINED){
                    Sr_Neighbor *nb = new Sr_Neighbor(next_visited_hop);
                    LIST_INSERT_HEAD(&buf->vnList, nb, nb_link);                 
                }

  //              /*
                if(flag){
                     pkt_record = buf->pktList.lh_first;                   

                       for(; pkt_record; pkt_record = next_pkt_record) {
                            next_pkt_record = pkt_record->pkt_entry.le_next;
                            LIST_REMOVE(pkt_record, pkt_entry);
                            delete pkt_record;
                        }
                     LIST_REMOVE(buf, record);
                     delete buf;
                }
//                */
            }

            else if (buf->get_num_copy_recvd() == 1){
                assert(buf);
                Packet* p = pkt_record->packet;
                next_visited_hop = forward_data_focus(p);                
                LIST_REMOVE(pkt_record, pkt_entry);
                delete pkt_record;
                
                LIST_REMOVE(buf, record);
               delete buf;
            }
    }
}

nsaddr_t Sr::forward_data_spray(Packet* p) {
    struct hdr_cmn* ch      = HDR_CMN(p);
    struct hdr_ip* ih       = HDR_IP(p);
    struct hdr_sr_pkt* sh   = HDR_SR_PKT(p);

    nsaddr_t next_hop;

    if (ch->direction() == hdr_cmn::UP &&
       ((u_int32_t)ih->daddr() == IP_BROADCAST || ih->daddr() == ra_addr())) {
        dmux_->recv(p, (Handler*)NULL) ;
        return UNDEFINED;
    }
    else {
        ch->direction() = hdr_cmn::DOWN;
        ch->addr_type() = NS_AF_INET;
        if ((u_int32_t)ih->daddr() == IP_BROADCAST)
            ch->next_hop() = IP_BROADCAST;
        else {

            /* Find the most recently encountered neighbor in the rtable_ which 
             * is not in the visited node list of the current message.
             */
            double max  = 0.0;
            nsaddr_t ip_addr = UNDEFINED;
            for (rtable_t::iterator it = rtable_.rt_.begin(); it != rtable_.rt_.end(); it++) {
                /*
                 * ((*it).first == (*it).second.next_hop): denotes neighbor.
                 * ((*it).second.time_of_encounter > max): denotes most recently encountered neighbor.
                 * (vn_list_lookup((*it).second.next_hop, ih->saddr(), sh->pkt_seq_num_))):
                 *        denotes the most recently encountered neighbor that has not been visited.
                 */
                if(((*it).first == (*it).second.next_hop) && ((*it).second.time_of_encounter > max) &&
                        (vn_list_lookup((*it).second.next_hop, ih->saddr(), sh->pkt_seq_num()))){
                    max = (*it).second.time_of_encounter;
                    ip_addr = (*it).second.next_hop;
                }
            }

            next_hop = ip_addr;

            if (next_hop == UNDEFINED){
                    return UNDEFINED;
            }

            else {
                 ch->next_hop() = next_hop;
                 ch->prev_hop_ = ra_addr();
            }
        }
        next_hop = ch->next_hop();
        sh->pkt_next_hop()   = ch->next_hop();
        sh->pkt_prev_hop()   = ch->prev_hop_;
        Scheduler::instance().schedule(target_, p, JITTER);
    }
    return next_hop;
}


nsaddr_t Sr::forward_data_focus(Packet* p) {
    struct hdr_cmn* ch = HDR_CMN(p);
    struct hdr_ip* ih = HDR_IP(p);
    struct hdr_sr_pkt* sh   = HDR_SR_PKT(p);
  
    nsaddr_t next_hop;

    if (ch->direction() == hdr_cmn::UP &&
       ((u_int32_t)ih->daddr() == IP_BROADCAST || ih->daddr() == ra_addr())) {        
        dmux_->recv(p, (Handler*)NULL);//(Handler*)NULL ) ;
        return UNDEFINED;
    }
    else {        
        ch->direction() = hdr_cmn::DOWN;
        ch->addr_type() = NS_AF_INET;
        if ((u_int32_t)ih->daddr() == IP_BROADCAST)
            ch->next_hop() = IP_BROADCAST;
        else {
            next_hop = rtable_.lookup(ih->daddr());
            
            if (next_hop == IP_BROADCAST) {
                debug("%f: Agent %d can not forward a packet destined to %d\n",
                    CURRENT_TIME,
                    ra_addr(),
                    ih->daddr());
                    drop(p, DROP_RTR_NO_ROUTE);
                    return UNDEFINED;
            }
            else{
                ch->next_hop() = next_hop;
                ch->prev_hop_ = ra_addr();
            }
        }
        next_hop = ch->next_hop();
        sh->pkt_next_hop()   = ch->next_hop();
        sh->pkt_prev_hop()   = ch->prev_hop_;
        Scheduler::instance().schedule(target_, p, JITTER);
    }
    return next_hop;
}