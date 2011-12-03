# ===================================================================

# AWK Script for calculating: 

#     => Average End-to-End Delay.

# ===================================================================

 

BEGIN {
   seqno           = -1;
   no_of_packets   = 0;    
   droppedPackets  = 0; 
   receivedPackets = 0; 
   total_delay     = 0;
}

{
   if($4 == "AGT"){
       if($1 == "s"){ 
       	    seqno = $6;
            no_of_packets++;
            start_time[$6] = $2;
       }    
    
       else if($1 == "r"){
            receivedPackets++;                
            end_time[$6] = $2;         #end-to-end delay
       }
   } 
}

 
END {
    for(i=0; i<=seqno; i++) {
            delay[i] = end_time[i] - start_time[i];
            if(delay[i] > 0)
                total_delay = total_delay + delay[i];
    }

    print "Generated Packets     : " no_of_packets;
    print "Received Packets      : " receivedPackets;
    print "Packet Delivery Ratio : " receivedPackets/no_of_packets*100"%";
    print "Average Delay         : " total_delay/receivedPackets;    
} 
