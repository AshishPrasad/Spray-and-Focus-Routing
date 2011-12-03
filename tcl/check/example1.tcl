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

#Tell the simulator object to execute finish procedure after 5.0s of simulation time.
$ns at 5.0 "finish"
#Start the simulation
$ns run