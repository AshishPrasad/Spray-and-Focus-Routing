# ====================================================================
# Define options
# ======================================================================
set opt(chan)	Channel/WirelessChannel
set opt(prop)	Propagation/TwoRayGround
set opt(netif)	Phy/WirelessPhy
set opt(mac)	Mac/802_11
#set opt(ifq)	Queue/DropTail/PriQueue
set opt(ifq)	CMUPriQueue
set opt(ll)		LL
set opt(ant)        Antenna/OmniAntenna
set opt(x)		500   ;# X dimension of the topography
set opt(y)		500   ;# Y dimension of the topography
set opt(ifqlen)	50	      ;# max packet in ifq
set opt(seed)	0.0
set opt(tr)		dsr-25-0-5.tr    ;# trace file
set opt(adhocRouting)   AODV
set opt(ifq)    Queue/DropTail/PriQueue
#set opt(rpr)	1	;#1 for DSR and anything else for AODV
set opt(nn)             	25           ;# how many nodes are simulated
set opt(scen)		"movement/scen-25-0" 
set opt(tfc)		"traffic/cbr-25-5" 
set opt(stop)		100.0		;# simulation time

# ======================================================================
# Main Program
# ======================================================================

if { $argc != 6 } {
        puts "Wrong no. of cmdline args."
	puts "Usage: ns compare.tcl -scen <scen> -tfc <tfc> -tr <tr>"
        exit 0
}


# proc getopt {argc argv} {
 
        for {set i 0} {$i < $argc} {incr i} {
                set arg [lindex $argv $i]
                if {[string range $arg 0 0] != "-"} continue
                set name [string range $arg 1 end]
#		puts $name
                set opt($name) [lindex $argv [expr $i+1]]
        }
	set opt(scen) [lindex $argv 1]
	set opt(tfc) [lindex $argv 3]

#        if {$opt(rpr) == 1} {
#	set opt(adhocRouting)   DSR
#	set opt(ifq)	CMUPriQueue
#	set opt(ifq)	Queue/DropTail/PriQueue
#        } else {
#	set opt(adhocRouting)   AODV
#	set opt(ifq) Queue/DropTail/PriQueue
#        }

#	set val(mov) $opt(scen)
#	set val(traf) $opt(tfc)
#	set opt(trace) $opt(tr)

	puts $opt(scen)
	puts $opt(tfc)
	puts $opt(tr)
# }


# getopt $argc $argv

	
	puts $opt(adhocRouting)
#	puts $val(mov)
#	puts $val(traf)
#	puts $opt(trace)

# Initialize Global Variables
# create simulator instance
set ns_		[new Simulator]

# set wireless channel, radio-model and topography objects
set wtopo	[new Topography]

# create trace object for ns and nam
set tracefd	[open $opt(tr) w]
$ns_ trace-all $tracefd
# use new trace file format
$ns_ use-newtrace 

# define topology
$wtopo load_flatgrid $opt(x) $opt(y)

# Create God
set god_ [create-god $opt(nn)]

#set chan_1_ [new $opt(chan)]
#set chan_2_ [new $opt(chan)]

# define how node should be created
#global node setting
$ns_ node-config -adhocRouting $opt(adhocRouting) \
		 -llType $opt(ll) \
		 -macType $opt(mac) \
		 -ifqType $opt(ifq) \
		 -ifqLen $opt(ifqlen) \
		 -antType $opt(ant) \
		 -propType $opt(prop) \
		 -phyType $opt(netif) \
		 -channelType $opt(chan) \
		 -topoInstance $wtopo \
		 -agentTrace ON \
                 -routerTrace ON \
                 -macTrace OFF 
#	-channel $chan_1_

#  Create the specified number of nodes [$opt(nn)] and "attach" them
#  to the channel. 
for {set i 0} {$i < $opt(nn) } {incr i} {
	set node_($i) [$ns_ node]	
	$node_($i) random-motion 0		;# disable random motion
}

# Define node movement model
puts "Loading connection pattern..."
source $opt(scen)
 
# Define traffic model
puts "Loading traffic file..."
source $opt(tfc)

# Define node initial position in nam
for {set i 0} {$i < $opt(nn)} {incr i} {

    # 20 defines the node size in nam, must adjust it according to your scenario
    # The function must be called after mobility model is defined
   $ns_ initial_node_pos $node_($i) 20
}

# Tell nodes when the simulation ends
for {set i 0} {$i < $opt(nn) } {incr i} {
    $ns_ at $opt(stop).000000001 "$node_($i) reset";
}

# tell nam the simulation stop time
#$ns_ at  $opt(stop)	"$ns_ nam-end-wireless $opt(stop)"
$ns_ at  $opt(stop).000000001 "puts \"NS EXITING...\" ; $ns_ halt"
puts "Starting Simulation..."
$ns_ run
