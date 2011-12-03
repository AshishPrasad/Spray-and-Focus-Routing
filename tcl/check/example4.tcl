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

set n0 [$ns node]
set n1 [$ns node]
set n2 [$ns node]
set n3 [$ns node]

$ns duplex-link $n0 $n2 1Mb 10ms DropTail
$ns duplex-link $n1 $n2 1Mb 10ms DropTail
$ns duplex-link $n3 $n2 1Mb 10ms SFQ

#For the orientation of the links.
$ns duplex-link-op $n0 $n2 orient right-down
$ns duplex-link-op $n1 $n2 orient right-up
$ns duplex-link-op $n2 $n3 orient right

#Create a UDP agent and attach it to node n0
set udp0 [new Agent/UDP]
$ns attach-agent $n0 $udp0

#Create a Traffic Source and attach it to udp0
set cbr0 [new Application/Traffic/CBR]
$cbr0 set packetSize_ 500
$cbr0 set interval_ 0.005
$cbr0 attach-agent $udp0

#Create a UDP agent and attach it to node n1
set udp1 [new Agent/UDP]
$ns attach-agent $n1 $udp1

#Create a Traffic Source and attach it to udp1
set cbr1 [new Application/Traffic/CBR]
$cbr1 set packetSize_ 500
$cbr1 set interval_ 0.005
$cbr1 attach-agent $udp1

#Create a null agent which acts as the traffic sink and attach it to node n3
set null0 [new Agent/Null]
$ns attach-agent $n3 $null0

#Now the 2 CBR agents need to be connected to the null agent
$ns connect $udp0 $null0
$ns connect $udp1 $null0


$udp0 set class_ 1
$udp1 set class_ 2
$ns color 1 Blue
$ns color 2 Green

#Monitoring the queue
$ns duplex-link-op $n2 $n3 queuePos 0.5

$ns at 0.5 "$cbr0 start"
$ns at 1.0 "$cbr1 start"
$ns at 4.0 "$cbr1 stop"
$ns at 4.5 "$cbr0 stop"




#Tell the simulator object to execute finish procedure after 5.0s of simulation time.
$ns at 5.0 "finish"
#Start the simulation
$ns run