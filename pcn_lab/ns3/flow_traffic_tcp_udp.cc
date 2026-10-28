
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/ipv4-global-routing-helper.h"

#include <iostream>
#include <iomanip>

using namespace ns3;

int main(int argc, char *argv[])
{
    std::string bottleneckRate = "2Mbps";
    std::string accessRate = "10Mbps";
    std::string accessDelay = "2ms";
    std::string bottleneckDelay = "10ms";
    std::string queueSize = "50p";

    double udpRateMbps = 1.0;
    double simTime = 20.0;
    bool pcap = true;

    CommandLine cmd(__FILE__);
    cmd.AddValue("bottleneckRate", "Bottleneck bandwidth", bottleneckRate);
    cmd.AddValue("accessRate", "Access link bandwidth", accessRate);
    cmd.AddValue("accessDelay", "Access link delay", accessDelay);
    cmd.AddValue("bottleneckDelay", "Bottleneck delay", bottleneckDelay);
    cmd.AddValue("queueSize", "Queue size", queueSize);
    cmd.AddValue("udpRateMbps", "UDP rate in Mbps", udpRateMbps);
    cmd.AddValue("simTime", "Simulation duration", simTime);
    cmd.AddValue("pcap", "Enable PCAP tracing", pcap);
    cmd.Parse(argc, argv);

    // STEP 1: Create six nodes (0 to 5)
    NodeContainer n;
    n.Create(6);

    InternetStackHelper internet;
    internet.Install(n);

    // STEP 2: Configure access links
    PointToPointHelper access;
    access.SetDeviceAttribute("DataRate", StringValue(accessRate));
    access.SetChannelAttribute("Delay", StringValue(accessDelay));
    access.SetQueue("ns3::DropTailQueue<Packet>",
                    "MaxSize", StringValue(queueSize));

    // STEP 3: Configure bottleneck link
    PointToPointHelper bottleneck;
    bottleneck.SetDeviceAttribute("DataRate",
                                  StringValue(bottleneckRate));
    bottleneck.SetChannelAttribute("Delay",
                                   StringValue(bottleneckDelay));
    bottleneck.SetQueue("ns3::DropTailQueue<Packet>",
                        "MaxSize", StringValue(queueSize));

    // STEP 4: Build the given topology
    // Node 0 ----\
    //             Node 2 ---- Node 4 ---- Node 5
    // Node 1 ----/    |
    //                Node 3

    NetDeviceContainer d02 = access.Install(n.Get(0), n.Get(2));
    NetDeviceContainer d12 = access.Install(n.Get(1), n.Get(2));
    NetDeviceContainer d23 = access.Install(n.Get(2), n.Get(3));

    NetDeviceContainer d24 =
        bottleneck.Install(n.Get(2), n.Get(4));

    NetDeviceContainer d45 = access.Install(n.Get(4), n.Get(5));

    // STEP 5: Assign a different subnet to each link
    Ipv4AddressHelper ip;

    Ipv4InterfaceContainer i02, i12, i23, i24, i45;

    ip.SetBase("10.1.1.0", "255.255.255.0");
    i02 = ip.Assign(d02);

    ip.SetBase("10.1.2.0", "255.255.255.0");
    i12 = ip.Assign(d12);

    ip.SetBase("10.1.3.0", "255.255.255.0");
    i23 = ip.Assign(d23);

    ip.SetBase("10.1.4.0", "255.255.255.0");
    i24 = ip.Assign(d24);

    ip.SetBase("10.1.5.0", "255.255.255.0");
    i45 = ip.Assign(d45);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // STEP 6: Create TCP and UDP receivers on Node 5
    uint16_t tcpPort = 5000;
    uint16_t udpPort = 6000;

    PacketSinkHelper tcpSink(
        "ns3::TcpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), tcpPort));

    ApplicationContainer tcpSinkApp = tcpSink.Install(n.Get(5));
    tcpSinkApp.Start(Seconds(0.0));
    tcpSinkApp.Stop(Seconds(simTime));

    PacketSinkHelper udpSink(
        "ns3::UdpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), udpPort));

    ApplicationContainer udpSinkApp = udpSink.Install(n.Get(5));
    udpSinkApp.Start(Seconds(0.0));
    udpSinkApp.Stop(Seconds(simTime));

    // STEP 7: TCP source on Node 0, destination Node 5
    OnOffHelper tcpSource(
        "ns3::TcpSocketFactory",
        InetSocketAddress(i45.GetAddress(1), tcpPort));

    tcpSource.SetAttribute("DataRate", StringValue("2Mbps"));
    tcpSource.SetAttribute("PacketSize", UintegerValue(1024));
    tcpSource.SetAttribute("OnTime",
        StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    tcpSource.SetAttribute("OffTime",
        StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    ApplicationContainer tcpSourceApp = tcpSource.Install(n.Get(0));
    tcpSourceApp.Start(Seconds(1.0));
    tcpSourceApp.Stop(Seconds(simTime - 1));

    // STEP 8: UDP source on Node 0, destination Node 5
    uint64_t udpBps =
        static_cast<uint64_t>(udpRateMbps * 1000000);

    OnOffHelper udpSource(
        "ns3::UdpSocketFactory",
        InetSocketAddress(i45.GetAddress(1), udpPort));

    udpSource.SetAttribute("DataRate",
        DataRateValue(DataRate(udpBps)));
    udpSource.SetAttribute("PacketSize", UintegerValue(512));
    udpSource.SetAttribute("OnTime",
        StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    udpSource.SetAttribute("OffTime",
        StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    ApplicationContainer udpSourceApp = udpSource.Install(n.Get(0));
    udpSourceApp.Start(Seconds(1.0));
    udpSourceApp.Stop(Seconds(simTime - 1));

    // STEP 9: Enable PCAP on the bottleneck
    if (pcap)
    {
        bottleneck.EnablePcap("wired-bottleneck", d24, true);
    }

    // STEP 10: Monitor network flows
    FlowMonitorHelper flowHelper;
    Ptr<FlowMonitor> monitor = flowHelper.InstallAll();

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

    monitor->CheckForLostPackets();

    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(
            flowHelper.GetClassifier());

    auto stats = monitor->GetFlowStats();

    // STEP 11: Print performance results
    std::cout << "\n=== NS-3 WIRED PERFORMANCE SUMMARY ===\n";
    std::cout << "Topology: Node 0 -> Node 2 -> Node 4 -> Node 5\n";
    std::cout << "TCP and UDP sources: Node 0\n";
    std::cout << "TCP and UDP sinks: Node 5\n";
    std::cout << "Bottleneck: " << bottleneckRate << "\n";

    for (const auto &entry : stats)
    {
        Ipv4FlowClassifier::FiveTuple t =
            classifier->FindFlow(entry.first);

        const FlowMonitor::FlowStats &s = entry.second;

        if (s.txPackets == 0)
            continue;

        double duration =
            (s.timeLastRxPacket - s.timeFirstTxPacket).GetSeconds();

        double throughputMbps = (duration > 0)
            ? (s.rxBytes * 8.0 / duration / 1000000.0)
            : 0.0;

        double avgDelayMs = (s.rxPackets > 0)
            ? (s.delaySum.GetSeconds() * 1000.0 / s.rxPackets)
            : 0.0;

        uint64_t lost = (s.txPackets > s.rxPackets)
            ? (s.txPackets - s.rxPackets)
            : 0;

        double lossPct = (s.txPackets > 0)
            ? (100.0 * lost / s.txPackets)
            : 0.0;

        std::string proto = (t.protocol == 6) ? "TCP" :
                            (t.protocol == 17) ? "UDP" : "Other";

        std::cout << "\nFlow " << entry.first
                  << " (" << proto << ") "
                  << t.sourceAddress << ":" << t.sourcePort
                  << " -> "
                  << t.destinationAddress << ":" << t.destinationPort
                  << "\n";

        std::cout << "  Tx packets: " << s.txPackets << "\n";
        std::cout << "  Rx packets: " << s.rxPackets << "\n";
        std::cout << "  Lost/unreceived estimate: " << lost << "\n";
        std::cout << "  Loss percentage estimate: "
                  << std::fixed << std::setprecision(2)
                  << lossPct << "%\n";
        std::cout << "  Throughput: "
                  << throughputMbps << " Mbps\n";
        std::cout << "  Average end-to-end delay: "
                  << avgDelayMs << " ms\n";
        std::cout << "  Jitter sum: "
                  << s.jitterSum.GetMilliSeconds() << " ms\n";
    }

    // STEP 12: Save results and clean up
    monitor->SerializeToXmlFile("wired-flowmon.xml", true, true);

    Simulator::Destroy();
    return 0;
}
