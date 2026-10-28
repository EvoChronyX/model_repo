
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpUdpTraffic");

int main(int argc, char *argv[])
{
    double simTime = 20.0;
    double tcpRate = 5.0; // TCP offered load in Mbps
    double udpRate = 1.0; // UDP offered load in Mbps

    CommandLine cmd(__FILE__);
    cmd.AddValue("simTime", "Simulation time in seconds", simTime);
    cmd.AddValue("tcpRate", "TCP application rate in Mbps", tcpRate);
    cmd.AddValue("udpRate", "UDP application rate in Mbps", udpRate);
    cmd.Parse(argc, argv);

    // Four end nodes and two routers
    NodeContainer hosts;
    hosts.Create(4);

    Ptr<Node> r1 = CreateObject<Node>();
    Ptr<Node> r2 = CreateObject<Node>();

    Ptr<Node> n0 = hosts.Get(0);
    Ptr<Node> n1 = hosts.Get(1);
    Ptr<Node> n2 = hosts.Get(2);
    Ptr<Node> n3 = hosts.Get(3);

    // Configure links
    PointToPointHelper access;
    access.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    access.SetChannelAttribute("Delay", StringValue("2ms"));
    access.SetQueue("ns3::DropTailQueue<Packet>",
                    "MaxSize", StringValue("50p"));

    PointToPointHelper bottleneck;
    bottleneck.SetDeviceAttribute("DataRate", StringValue("2Mbps"));
    bottleneck.SetChannelAttribute("Delay", StringValue("10ms"));
    bottleneck.SetQueue("ns3::DropTailQueue<Packet>",
                        "MaxSize", StringValue("50p"));

    // Access links: end hosts to routers
    NetDeviceContainer d0 = access.Install(n0, r1);
    NetDeviceContainer d1 = access.Install(n1, r1);

    // Shared bottleneck between routers
    NetDeviceContainer dr = bottleneck.Install(r1, r2);

    NetDeviceContainer d3 = access.Install(r2, n3);
    NetDeviceContainer d2 = access.Install(r2, n2);

    // Install the Internet protocol stack
    InternetStackHelper internet;
    internet.Install(hosts);
    internet.Install(r1);
    internet.Install(r2);

    // Assign a different subnet to every link
    Ipv4AddressHelper address;

    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i0 = address.Assign(d0);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer i1 = address.Assign(d1);

    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer ir = address.Assign(dr);

    address.SetBase("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer i3 = address.Assign(d3);

    address.SetBase("10.1.5.0", "255.255.255.0");
    Ipv4InterfaceContainer i2 = address.Assign(d2);

    // Automatically populate routes through both routers
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // TCP flow: Node 0 -> Node 3
    uint16_t tcpPort = 5000;

    PacketSinkHelper tcpSinkHelper(
        "ns3::TcpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), tcpPort));

    ApplicationContainer tcpSink = tcpSinkHelper.Install(n3);
    tcpSink.Start(Seconds(0.0));
    tcpSink.Stop(Seconds(simTime));

    OnOffHelper tcpSource(
        "ns3::TcpSocketFactory",
        InetSocketAddress(i3.GetAddress(1), tcpPort));

    tcpSource.SetAttribute("DataRate",
                           StringValue(std::to_string(tcpRate) + "Mbps"));
    tcpSource.SetAttribute("PacketSize", UintegerValue(1024));
    tcpSource.SetAttribute("OnTime",
        StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    tcpSource.SetAttribute("OffTime",
        StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    ApplicationContainer tcpApp = tcpSource.Install(n0);
    tcpApp.Start(Seconds(1.0));
    tcpApp.Stop(Seconds(simTime - 0.1));

    // UDP flow: Node 1 -> Node 2
    uint16_t udpPort = 6000;

    PacketSinkHelper udpSinkHelper(
        "ns3::UdpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), udpPort));

    ApplicationContainer udpSink = udpSinkHelper.Install(n2);
    udpSink.Start(Seconds(0.0));
    udpSink.Stop(Seconds(simTime));

    OnOffHelper udpSource(
        "ns3::UdpSocketFactory",
        InetSocketAddress(i2.GetAddress(1), udpPort));

    udpSource.SetAttribute("DataRate",
                           StringValue(std::to_string(udpRate) + "Mbps"));
    udpSource.SetAttribute("PacketSize", UintegerValue(512));
    udpSource.SetAttribute("OnTime",
        StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    udpSource.SetAttribute("OffTime",
        StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    ApplicationContainer udpApp = udpSource.Install(n1);
    udpApp.Start(Seconds(1.0));
    udpApp.Stop(Seconds(simTime - 0.1));

    // Enable packet-level tracing
    AsciiTraceHelper ascii;
    access.EnableAsciiAll(ascii.CreateFileStream("access.tr"));
    bottleneck.EnableAsciiAll(ascii.CreateFileStream("bottleneck.tr"));

    access.EnablePcapAll("access");
    bottleneck.EnablePcapAll("bottleneck");

    // Enable FlowMonitor statistics
    FlowMonitorHelper flowHelper;
    Ptr<FlowMonitor> monitor = flowHelper.InstallAll();

    std::cout << "\n=== TCP AND UDP TRAFFIC SIMULATION ===\n";
    std::cout << "Topology: Node 0 -> Router 1 -> Router 2 -> Node 3\n";
    std::cout << "           Node 1 -> Router 1 -> Router 2 -> Node 2\n";
    std::cout << "Access links: 10 Mbps, 2 ms\n";
    std::cout << "Bottleneck: 2 Mbps, 10 ms, DropTail 50 packets\n";
    std::cout << "TCP packet size: 1024 bytes\n";
    std::cout << "UDP packet size: 512 bytes\n";
    std::cout << "Simulation time: " << simTime << " seconds\n";

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

    monitor->CheckForLostPackets();

    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());

    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();

    std::cout << "\n=== PERFORMANCE RESULTS ===\n";

    for (auto it = stats.begin(); it != stats.end(); ++it)
    {
        Ipv4FlowClassifier::FiveTuple t =
            classifier->FindFlow(it->first);

        const FlowMonitor::FlowStats &st = it->second;

        // Identify only the application data flows by destination port
        if (t.destinationPort != tcpPort &&
            t.destinationPort != udpPort)
            continue;

        double duration =
            (st.timeLastRxPacket.GetSeconds() -
             st.timeFirstTxPacket.GetSeconds());

        double throughput = 0.0;
        if (duration > 0)
            throughput = st.rxBytes * 8.0 / duration / 1e6;

        double avgDelayMs = 0.0;
        if (st.rxPackets > 0)
            avgDelayMs =
                st.delaySum.GetSeconds() * 1000.0 / st.rxPackets;

        uint64_t lost = st.lostPackets;
        uint64_t total = st.txPackets;

        double lossPercent = 0.0;
        if (total > 0)
            lossPercent = 100.0 * lost / total;

        std::cout << "\nFlow ID: " << it->first << "\n";
        std::cout << "Protocol: "
                  << (t.protocol == 6 ? "TCP" : "UDP") << "\n";
        std::cout << "Source: " << t.sourceAddress << "\n";
        std::cout << "Destination: " << t.destinationAddress << "\n";
        std::cout << "Source port: " << t.sourcePort << "\n";
        std::cout << "Destination port: " << t.destinationPort << "\n";
        std::cout << "Tx packets: " << st.txPackets << "\n";
        std::cout << "Rx packets: " << st.rxPackets << "\n";
        std::cout << "Lost packets: " << lost << "\n";
        std::cout << "Packet loss: " << lossPercent << "%\n";
        std::cout << "Throughput: " << throughput << " Mbps\n";
        std::cout << "Average delay: " << avgDelayMs << " ms\n";
    }

    // Save detailed FlowMonitor output
    monitor->SerializeToXmlFile("tcp_udp_flowmon.xml", true, true);

    Simulator::Destroy();
    return 0;
}
