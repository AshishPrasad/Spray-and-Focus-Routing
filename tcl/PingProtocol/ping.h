/* 
 * File:   ping.h
 * Author: apurv
 *
 * Created on March 25, 2011, 5:51 PM
 */

#ifndef ns_ping_h
#define	ns_ping_h

#include "agent.h"
#include "tclcl.h"
#include "packet.h"
#include "address.h"
#include "ip.h"

struct hdr_ping{

    char ret;
    double send_time;
};

class PingAgent:public Agent{
public:
    PingAgent();
    int command(int argc, const char* const* argv);
    void recv(Packet*, Handler*);
protected:
    int off_ping_;
};


#endif	/* PING_H */

