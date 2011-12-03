#include "sr_rtable.h"
#include <ip.h>

/* Dump the contents of the node’s routing table to the trace ﬁle */
void sr_rtable::print(Trace* out){
    sprintf(out->pt_->buffer(), "P\tdest \tnext \telapsed \tencounter \texpire");
    out->pt_->dump();
    for (rtable_t::iterator it = rt_.begin(); it != rt_.end(); it++) {
        sprintf(out->pt_->buffer(), "P\t%d\t%d\t%f\t%f\t%f",
                (*it).first,
                (*it).second.next_hop,                
                (*it).second.time_elapsed,
                (*it).second.time_of_encounter,
                (*it).second.time_expire);
        out->pt_->dump();
    }
}

/* Removes all entries in the routing table */
 void sr_rtable::clear() {
     rt_.clear();
 }

 /* Removes an entry given its destination address */
 void sr_rtable::rm_entry(nsaddr_t dest) {
     rt_.erase(dest);
 }

 /*  Add a new entry in the routing table given its destination
  *  and next hop addresses */
 void sr_rtable::add_entry(nsaddr_t dest, nsaddr_t next, double encounter, double elapsed, double expire) {
     rtable_entry entry;
     entry.next_hop = next;
     entry.time_of_encounter = encounter;
     entry.time_elapsed = elapsed;
     entry.time_expire = expire;
     rt_[dest] = entry;

 }

 /* Lookup() returns the next hop address of an entry given its destination ad-
dress. If such an entry doesn’t exist,the function returns IP BROADCAST. */
 bool sr_rtable::lookup_entry(nsaddr_t dest) {
     for (rtable_t::iterator it = rt_.begin(); it != rt_.end(); it++) {
        if ((*it).first == dest)
            return true;
     }
     return false;
 }




 /* Lookup() returns the next hop address of an entry given its destination ad-
dress. If such an entry doesn’t exist,the function returns IP BROADCAST. */
 nsaddr_t sr_rtable::lookup(nsaddr_t dest) {
     for (rtable_t::iterator it = rt_.begin(); it != rt_.end(); it++) {
        if ((*it).first == dest)
            return (*it).second.next_hop;
     }

     return UNDEFINED;
 }

 /* size() returns the number of entries in the routing table. */
 u_int32_t sr_rtable::size() {
     return rt_.size();
 }