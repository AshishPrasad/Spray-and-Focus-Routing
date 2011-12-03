#ifndef __sr_constants_h_
#define	__sr_constants_h_

#define CURRENT_TIME Scheduler::instance().clock()
#define JITTER (Random::uniform()*0.5)
/*BUFFER_SIZE limit has not been imposed yet.*/

#define UNDEFINED -5
#define ICTRL 10
#define DATA_SIZE 1500
#define IDATA 0.5
#define BETA 0.875
#define IEXPIRE 30.0

#define SR_L 4
#define TTL_VALUE 32

#endif	/*__sr_constants_h_*/
