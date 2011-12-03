#Create a simulator object.
set ns [new Simulator]

#Open out.nam for writing nam trace data and give this file a handle nf
set nf [open out.nam w]

#Tell the simulator object that relevant data for nam is going to be written in this file.
$ns namtrace-all $nf

#The finish procedure
proc finish {} {
  global ns nf
  $ns flush-trace
  close $nf
  exec nam out.nam &
  exit 0
}

#Create two nodes and assign them handles n0 and n1
set n0 [$ns node]
set n1 [$ns node]

#Setup a link between the two nodes with the following parameters.
$ns duplex-link $n0 $n1 1Mb 10ms DropTail


#Create a UDP agent and attach it to node n0
set udp0 [new Agent/UDP]
$ns attach-agent $n0 $udp0

#Create a CBR traffic source and attach it to udp0
set cbr0 [new Application/Traffic/CBR]
$cbr0 set packetSize_ 500
$cbr0 set interval_ 0.005
$cbr0 attach-agent $udp0

#Create a null agent which acts as a traffic sink and attach it to node n1
set null0 [new Agent/Null]
$ns attach-agent $n1 $null0
$ns connect $udp0 $null0

#Tell CBR agent when to send data and when to stop
$ns at 0.5 "$cbr0 start"
$ns at 4.5 "$cbr0 stop"


#Tell the simulator object to execute finish procedure after 5.0s of simulation time.
$ns at 5.0 "finish"
#Start the simulation
$ns run