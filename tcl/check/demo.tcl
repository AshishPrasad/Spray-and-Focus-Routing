# vim: set fenc=utf-8 ff=unix 

# Application/UserData バイナリ送受信テスト
#
# Wada Laboratory, Shizuoka University
# Takahiro Sugimoto	<keyaki.no.kokage@gmail.com>
#
# Last Modified: 2008/12/17 18:07:57

# 
# 設定用変数
# 

# -- Phy / Mac --
set val(chan)	Channel/WirelessChannel		;# 通信路の種類
set val(netif)	Phy/WirelessPhy			;# Network Interfaceの種類
set val(prop)	Propagation/Shadowing		;# 無線伝搬モデル
set val(ant)	Antenna/OmniAntenna		;# アンテナモデル
set val(mac)	Mac/802_11			;# MACの種類
set val(ifq)	Queue/DropTail/PriQueue		;# Interface Queue(IFQ)の種類
set val(ifqlen)	50				;# IFQの長さ
set val(ll)	LL				;# リンク層の種類
# -- Routing --
set val(rp)	AODV				;# ルーティングプロトコル
# -- Network Topology / Topography --
set val(n)	4				;# ノード数
set val(x)	500				;# Topography(地形)のx方向
set val(y)	500				;# Topography(地形)のy方向
# -- Other stuff --
set val(simid)	demo				;# シミュレーション識別名
set val(infile)	$val(simid).in			;# 入力ファイル名
set val(outfile)	$val(simid).out		;# 出力ファイル名
set val(sorted)	$val(simid).sorted		;# 並替済出力ファイル名
set val(logout)	$val(simid)_ud.log		;# UserDataログファイル名
set val(interval) 0.01				;# 送信間隔
set val(unitlen) 128				;# メッセージ長
set val(scheme)	round-robin		;# 経路割当: ラウンドロビン
#set val(scheme)	uniform			;# 経路割当: 一様分布
#set val(scheme)	hop-weighted		;# 経路割当: ホップ数利用
#set val(scheme)	clone			;# 経路割当: 各経路にコピー
#set val(scheme)	short-only		;# 経路割当: 最短経路のみ

#
# シナリオ
#

# シミュレータの初期化
set ns [new Simulator]

# トレースファイルの初期化
set tracefd [open $val(simid).tr w]
$ns trace-all $tracefd
#$ns use-newtrace
set namtrace [open $val(simid).nam w]
$ns namtrace-all-wireless $namtrace $val(x) $val(y)

# 入力・出力ファイルの初期化
set input [new UserDataFile]
$input setfile $val(infile) r
set output [new UserDataFile]
$output setfile $val(outfile) w
set sorted [new UserDataFile]
$sorted setfile $val(sorted) w
set logout [new UserDataLog]
$logout setfile $val(logout)

# Topographyの初期化
set topo [new Topography]
$topo load_flatgrid $val(x) $val(y)

# Godの生成
create-god $val(n)

# ノードの生成
set chan_1_ [new $val(chan)]
$ns node-config -adhocRouting $val(rp) \
                -llType $val(ll) \
                -macType $val(mac) \
                -ifqType $val(ifq) \
                -ifqLen $val(ifqlen) \
                -antType $val(ant) \
                -propType $val(prop) \
                -phyType $val(netif) \
                -topoInstance $topo \
                -agentTrace ON \
                -routerTrace ON \
                -macTrace ON \
                -movementTrace ON \
                -channel $chan_1_ 
for {set i 0} {$i < $val(n)} { incr i } {
	set node_($i) [$ns node]
}

# Agent・Applicationのアタッチ
set a(0) [new Agent/UDP]
set a(1) [new Agent/UDP]
$ns attach-agent $node_(0) $a(0)
$ns attach-agent $node_(1) $a(1)
$ns connect $a(0) $a(1)
set userdata(0) [new Application/UserData]
set userdata(1) [new Application/UserData]
$userdata(0) attach-agent $a(0)
$userdata(0) attach-file-in $input
$userdata(0) attach-log $logout
$userdata(0) set-interval $val(interval)
$input set-unitlen $val(unitlen)
$userdata(1) attach-agent $a(1)
$userdata(1) attach-file-out $output
$userdata(1) attach-file-sorted $sorted
$userdata(1) attach-log $logout
# クロスレイヤ連携
$userdata(0) attach-ragent [$node_(0) set ragent_]
$userdata(1) attach-ragent [$node_(1) set ragent_]
# ルート割り当て方式
$userdata(0) set-multiroute-scheme $val(scheme)

# ノードの位置の設定
# X: 横, Y: 縦
#  (2) ---- (1)
#   |        |
#   |        |
#  (0) ---- (3)
#
$node_(0) set X_ 100.0
$node_(0) set Y_ 0.0
$node_(0) set Z_ 0.0

$node_(1) set X_ 340.0
$node_(1) set Y_ 230.0
$node_(1) set Z_ 0.0

$node_(2) set X_ 100.0
$node_(2) set Y_ 230.0
$node_(2) set Z_ 0.0

$node_(3) set X_ 340.0
$node_(3) set Y_ 0.0
$node_(3) set Z_ 0.0

for {set i 0} {$i < $val(n)} {incr i} {
	$ns initial_node_pos $node_($i) 30
}

# イベントの設定
$ns at 1.0 "$userdata(0) start"
$ns at 500.0 "$userdata(0) stop"
$ns at 500.1 "$userdata(1) stop"
proc finish {} {
	global ns tracefd namtrace

	$ns flush-trace
	close $tracefd
	close $namtrace

	exit 0
}
$ns at 500.2 "finish; $ns halt"

# 実行
$ns run
