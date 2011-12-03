# ===================================================================

# AWK Script for calculating: 

#     => Average End-to-End Delay.

# ===================================================================

 

BEGIN {
   seqno           = 0;    
   droppedPackets  = 0; 
   receivedPackets = 0; 
   total_delay     = 0;
}

{
   if($19 == "sh:2"){
       if(($1 == "s") && ($3=="_"$22"_") && (start_time[$25] == 0)){ 
       	    seqno++;
            start_time[$25] = $2;
       }    
    
       else if(($1 == "r") && (end_time[$25] == 0) && ($31 == $34) && ($3=="_"$31"_")){
            receivedPackets++;                
            end_time[$25] = $2;         #end-to-end delay
       }
   } 
}

 
END {
    for(i=1; i<=seqno; i++) {
            delay[i] = end_time[i] - start_time[i];
            if(delay[i] > 0)
                total_delay = total_delay + delay[i];
    }

    print "Generated Packets     : " seqno;
    print "Received Packets      : " receivedPackets;
    print "Packet Delivery Ratio : " receivedPackets/seqno*100"%";
    print "Average Delay         : " total_delay/receivedPackets;    
} 
