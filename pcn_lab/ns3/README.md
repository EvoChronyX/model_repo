 # 1. bottleneckRate = "2Mbps"

Meaning: The transmission speed of the bottleneck link between the two routers.
In your router-based program:

Node 4 (Router 0) ── 2 Mbps ── Node 5 (Router 1)

This link can transmit at 2 million bits per second. Since it is slower than the 10 Mbps access links, it can become the network's limiting link.
If reduced to 1 Mbps, congestion may increase. If increased to 10 Mbps, the bottleneck may become le


# 2. accessRate = "10Mbps"

Meaning: The transmission speed of the ordinary access links connecting end nodes to routers, and the destination nodes to routers.

For example:

Node 0 ── 10 Mbps ── Router 0

These links are faster than the 2 Mbps bottleneck, allowing traffic to reach the bottleneck faster than it can be forwarded.

Increasing access bandwidth does not necessarily improve end-to-end throughput if the bottleneck remains at 2 Mbps.



# 3. accessDelay = "2ms"

Meaning: The configured propagation delay of each access link.

A packet travelling across one access link experiences a configured propagation delay of 2 milliseconds, excluding transmission and queueing delays.

For example, the path from Node 0 to Node 3 crosses two access links. Their propagation delays add up to 4 ms.







You will run the same simulation three times, changing only the TCP congestion-control algorithm.
Algorithm	Main idea	What to investigate

TCP Reno / NewReno ------>   	Uses ACKs and packet-loss signals to regulate its congestion window	How quickly it recovers from congestion

TCP Cubic -------->   	Uses a cubic congestion-window growth function, commonly used in Linux	Throughput under sustained congestion

TCP BBR	-------->   Estimates bottleneck bandwidth and round-trip propagation time to control sending	Throughput and delay when queues build up



## Commands to run




## flow_traffic_tcp_udp:

./ns3 run "scratch/your_filename --udpRateMbps=1"
./ns3 run "scratch/your_filename --udpRateMbps=3"


To test TCP and UDP together: your code already runs both sources simultaneously.
./ns3 run "scratch/your_filename --bottleneckRate=2Mbps"
./ns3 run "scratch/your_filename --bottleneckRate=500Kbps"


To disable PCAP tracing:
./ns3 run "scratch/your_filename --pcap=0"


Check the printed TCP/UDP flow results and wired-flowmon.xml.



## tcp_vanet:

./ns3 --run "scratch/vanet_tcp --tcpType=ns3::TcpNewReno"
./ns3 --run "scratch/vanet_tcp --tcpType=ns3::TcpCubic"
./ns3 --run "scratch/vanet_tcp --tcpType=ns3::TcpBbr"


## route_protocol:

./ns3 --run "scratch/routing_demo --protocol=rip --condition=normal"


Then run AODV:
./ns3 --run "scratch/routing_demo --protocol=aodv --condition=normal"


Run the Global Routing baseline:
./ns3-run "scratch/routing_demo --protocol=global --condition=normal"


Step 2 — Run the congestion condition
./ns3 run "scratch/routing_demo --protocol=rip --condition=congestion"
./ns3 run "scratch/routing_demo --protocol=aodv --condition=congestion"
./ns3 run "scratch/routing_demo --protocol=global --condition=congestion"

Step 3 — Run the link-failure condition
./ns3 run "scratch/routing_demo --protocol=rip --condition=failure"
./ns3 run "scratch/routing_demo --protocol=aodv --condition=failure"
./ns3 --run "scratch/routing_demo --protocol=global --condition=failure"


## wifi_wireless:

Run from your NS-3.42 directory:
./ns3 run "scratch/your_file_name --routing=olsr --mode=static"


UDP with OLSR:
./ns3 run "scratch/your_file_name --routing=olsr --mode=static --udpRate=1Mbps"


UDP with AODV:
./ns3 run "scratch/your_file_name --routing=aodv --mode=randomwalk --speed=5"

## wifi_bluetooth:

./ns3 --run "scratch/campus_wireless --speed=1"
./ns3 --run "scratch/campus_wireless --speed=5"




