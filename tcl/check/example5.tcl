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


#Creating a larger topology
for {set i 0} { $i<7 } {incr i} {
  set n($i) [$ns node]
}

#Creating a larger topology
for {set i 0} {$i<7} {incr i} {
  $ns duplex-link $n($i) $n([expr ($i+1)%7]) 1Mb 10ms DropTail
}

#Create a UDP agent and attach it to node n(0)
set udp0 [new Agent/UDP]
$ns attach-agent $n(0) $udp0

#Create a CBR traffic source and attach it to udp0
set cbr0 [new Application/Traffic/CBR]
$cbr0 set packetSize_ 500
$cbr0 set interval_ 0.005
$cbr0 attach-agent $udp0

#Create a null agent which acts as a null agent and attach it to node n3
set null0 [new Agent/Null]
$ns attach-agent $n(3) $null0


#Now connect the udp agent to the null agent
$ns connect $udp0 $null0

$udp0 set class_ 1
$ns color 1 Red

$ns rtmodel-at 1.0 down $n(1) $n(2)
$ns rtmodel-at 2.0 up $n(1) $n(2)

$ns rtproto DV

$ns at 0.5 "$cbr0 start"
$ns at 4.5 "$cbr0 stop"


#Tell the simulator object to execute finish procedure after 5.0s of simulation time.
$ns at 5.0 "finish"
#Start the simulation
$ns run