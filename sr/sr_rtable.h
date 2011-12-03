#ifndef __sr_rtable_h__
#define __sr_rtable_h__

#include "sr_constants.h"
#include "trace/trace.h"
#include <map>

/*
 * The map rtable_t has the following structure
 * <nsaddr_t x, nsaddr_t y>
 * 'y' is the next hop for reaching the final destination 'x'
 */

struct rtable_entry{
    nsaddr_t next_hop;
    double time_of_encounter; // denotes the time of last encounter.
    double time_elapsed; //denotes the time between successive encounters.
    double time_expire; // denotes the time after which entry must be removed.
};


typedef std::map<nsaddr_t, rtable_entry> rtable_t;


class Sr_Neighbor;

/*sr_ncache type represents a list of neighbors of a node*/
LIST_HEAD(sr_ncache, Sr_Neighbor);

class sr_rtable {

  public:

     rtable_t rt_;

     sr_rtable(){}
     void print(Trace*);
     void clear();
     void rm_entry(nsaddr_t);
     void add_entry(nsaddr_t, nsaddr_t, double, double, double);
     bool lookup_entry(nsaddr_t);
     nsaddr_t lookup(nsaddr_t);
     u_int32_t size();
};


/*Adhering to AODV convention we place the Sr_Neighbor class here.*/
class Sr_Neighbor {
 public:
        Sr_Neighbor(nsaddr_t a) { nb_addr = a;}
        LIST_ENTRY(Sr_Neighbor)nb_link;          
        nsaddr_t nb_addr;
};

#endif /* __sr_rtable_h__ */