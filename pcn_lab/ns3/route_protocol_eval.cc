#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/aodv-module.h"
#include "ns3/rip-helper.h"
#include "ns3/ipv4-global-routing-helper.h"

#include <iostream>
#include <fstream>
#include <string>

using namespace ns3;

static void
BringLinkDown(Ptr<Node> r1, Ptr<NetDevice> dev1,
              Ptr<Node> r2, Ptr<NetDevice> dev2,
              bool globalRouting)
{
    Ptr<Ipv4> ip1 = r1->GetObject<Ipv4>();
    Ptr<Ipv4> ip2 = r2->GetObject<Ipv4>();

    int32_t i1 = ip1->GetInterfaceForDevice(dev1);
    int32_t i2 = ip2->GetInterfaceForDevice(dev2);

    std::cout << "\n[" << Simulator::Now().GetSeconds()
              << " s] LINK FAILURE: R1 -- R2\n";

    if (i1 >= 0)
        ip1->SetDown(static_cast<uint32_t>(i1));

    if (i2 >= 0)
        ip2->SetDown(static_cast<uint32_t>(i2));

    // Global routing is a computed baseline, so explicitly recalculate routes.
    if (globalRouting)
    {
        Simulator::Schedule(MilliSeconds(100),
            &Ipv4GlobalRoutingHelper::RecomputeRoutingTables);
    }
}

static NetDeviceContainer
MakeLink(Ptr<Node> a, Ptr<Node> b,
         const std::string& rate, const std::string& delay)
{
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(rate));
    p2p.SetChannelAttribute("Delay", StringValue(delay));
    p2p.SetQueue("ns3::DropTailQueue",
                 "MaxSize", StringValue("50p"));

    NodeContainer pair(a, b);
    return p2p.Install(pair);
}

static Ipv4InterfaceContainer
AssignSubnet(NetDeviceContainer devices, const std::string& network)
{
    Ipv4AddressHelper address;
    address.SetBase(network.c_str(), "255.255.255.0");
    return address.Assign(devices);
}

int
main(int argc, char *argv[])
{
    std::string protocol = "rip";       // rip, aodv, global
    std::string condition = "normal";   // normal, congestion, failure
    double simTime = 30.0;
    double offeredRate = 2.0;           // Mbps per UDP flow

    CommandLine cmd(__FILE__);
    cmd.AddValue("protocol", "rip, aodv, or global", protocol);
    cmd.AddValue("condition", "normal, congestion, or failure", condition);
    cmd.AddValue("simTime", "Simulation duration in seconds", simTime);
    cmd.AddValue("offeredRate", "UDP rate per flow in Mbps", offeredRate);
    cmd.Parse(argc, argv);

    if (protocol != "rip" && protocol != "aodv" &&
        protocol != "global")
    {
        std::cerr << "Use --protocol=rip, aodv, or global\n";
        return 1;
    }

    if (condition != "normal" && condition != "congestion" &&
        condition != "failure")
    {
        std::cerr << "Use --condition=normal, congestion, or failure\n";
        return 1;
    }

    if (simTime < 12.0 || offeredRate <= 0.0)
    {
        std::cerr << "Use simTime >= 12 and offeredRate > 0\n";
        return 1;
    }

    // Two source hosts, four routers, and two destination hosts.
    Ptr<Node> srcA = CreateObject<Node>();
    Ptr<Node> srcB = CreateObject<Node>();
    Ptr<Node> r1   = CreateObject<Node>();
    Ptr<Node> r2   = CreateObject<Node>();
    Ptr<Node> r3   = CreateObject<Node>();
    Ptr<Node> r4   = CreateObject<Node>();
    Ptr<Node> dstA = CreateObject<Node>();
    Ptr<Node> dstB = CreateObject<Node>();

    NodeContainer all;
    all.Add(srcA);
    all.Add(srcB);
    all.Add(r1);
    all.Add(r2);
    all.Add(r3);
    all.Add(r4);
    all.Add(dstA);
    all.Add(dstB);

    // Select the routing implementation.
    InternetStackHelper internet;

    if (protocol == "rip")
    {
        RipHelper rip;
        internet.SetRoutingHelper(rip);
    }
    else if (protocol == "aodv")
    {
        AodvHelper aodv;
        internet.SetRoutingHelper(aodv);
    }
    else
    {
        Ipv4GlobalRoutingHelper global;
        internet.SetRoutingHelper(global);
    }

    internet.Install(all);

    // Topology:
    // srcA -- R1 -- R2 -- R4 -- dstA
    // srcB --/       \       /-- dstB
    //                 R3 ----
    //
    // Main path: R1 -> R2 -> R4
    // Alternate path: R1 -> R3 -> R4

    NetDeviceContainer srcALink = MakeLink(srcA, r1, "10Mbps", "2ms");
    NetDeviceContainer srcBLink = MakeLink(srcB, r1, "10Mbps", "2ms");
    NetDeviceContainer r1r2 = MakeLink(r1, r2, "5Mbps", "5ms");
    NetDeviceContainer r2r4 = MakeLink(r2, r4, "5Mbps", "5ms");
    NetDeviceContainer r1r3 = MakeLink(r1, r3, "5Mbps", "8ms");
    NetDeviceContainer r3r4 = MakeLink(r3, r4, "5Mbps", "8ms");
    NetDeviceContainer r4dstA = MakeLink(r4, dstA, "10Mbps", "2ms");
    NetDeviceContainer r4dstB = MakeLink(r4, dstB, "10Mbps", "2ms");

    AssignSubnet(srcALink, "10.1.1.0");
    AssignSubnet(srcBLink, "10.1.2.0");
    AssignSubnet(r1r2,     "10.1.3.0");
    AssignSubnet(r2r4,     "10.1.4.0");
    AssignSubnet(r1r3,     "10.1.5.0");
    AssignSubnet(r3r4,     "10.1.6.0");
    Ipv4InterfaceContainer dstAIf =
        AssignSubnet(r4dstA, "10.1.7.0");
    Ipv4InterfaceContainer dstBIf =
        AssignSubnet(r4dstB, "10.1.8.0");

    // Global routing needs its initial routes populated.
    if (protocol == "global")
    {
        Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    }

    // Print routing-table snapshots before and after the failure.
    AsciiTraceHelper ascii;
    Ptr<OutputStreamWrapper> routeStream =
        ascii.CreateFileStream("routing-tables.txt");

    Ipv4RoutingHelper::PrintRoutingTableAllAt(
        Seconds(5.0), routeStream);

    Ipv4RoutingHelper::PrintRoutingTableAllAt(
        Seconds(15.0), routeStream);

    Ipv4RoutingHelper::PrintRoutingTableAllAt(
        Seconds(simTime - 1.0), routeStream);

    uint16_t portA = 9000;
    uint16_t portB = 9001;
    uint16_t portC = 9002;
    uint16_t portD = 9003;

    // UDP receivers on destination hosts.
    PacketSinkHelper sinkA("ns3::UdpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), portA));
    PacketSinkHelper sinkB("ns3::UdpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), portB));
    PacketSinkHelper sinkC("ns3::UdpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), portC));
    PacketSinkHelper sinkD("ns3::UdpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), portD));

    ApplicationContainer sinks;
    sinks.Add(sinkA.Install(dstA));
    sinks.Add(sinkB.Install(dstB));
    sinks.Add(sinkC.Install(dstA));
    sinks.Add(sinkD.Install(dstB));
    sinks.Start(Seconds(0.5));
    sinks.Stop(Seconds(simTime));

    // Every flow uses the same offered rate and packet size.
    auto addFlow = [&](Ptr<Node> source, Ipv4Address destination,
                       uint16_t port)
    {
        OnOffHelper app("ns3::UdpSocketFactory",
            InetSocketAddress(destination, port));

        app.SetAttribute("DataRate",
            DataRateValue(DataRate(
                static_cast<uint64_t>(offeredRate * 1000000.0))));

        app.SetAttribute("PacketSize", UintegerValue(512));
        app.SetAttribute("OnTime", StringValue(
            "ns3::ConstantRandomVariable[Constant=1]"));
        app.SetAttribute("OffTime", StringValue(
            "ns3::ConstantRandomVariable[Constant=0]"));

        ApplicationContainer installed = app.Install(source);
        installed.Start(Seconds(2.0));
        installed.Stop(Seconds(simTime - 0.5));
    };

    // Normal condition: one flow.
    addFlow(srcA, dstAIf.GetAddress(1), portA);

    // Congestion condition: four simultaneous flows.
    if (condition == "congestion")
    {
        addFlow(srcA, dstBIf.GetAddress(1), portB);
        addFlow(srcB, dstAIf.GetAddress(1), portC);
        addFlow(srcB, dstBIf.GetAddress(1), portD);
    }

    // Failure condition: two simultaneous flows and a mid-run link failure.
    if (condition == "failure")
    {
        addFlow(srcB, dstBIf.GetAddress(1), portD);

        Simulator::Schedule(Seconds(10.0),
            &BringLinkDown, r1, r1r2.Get(0),
            r2, r1r2.Get(1), protocol == "global");
    }

    FlowMonitorHelper flowHelper;
    Ptr<FlowMonitor> monitor = flowHelper.InstallAll();

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();
    monitor->CheckForLostPackets();

    uint64_t txPackets = 0;
    uint64_t rxPackets = 0;
    uint64_t lostPackets = 0;
    uint64_t rxBytes = 0;
    double totalDelay = 0.0;

    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());

    for (const auto &entry : monitor->GetFlowStats())
    {
        Ipv4FlowClassifier::FiveTuple tuple =
            classifier->FindFlow(entry.first);

        const FlowMonitor::FlowStats &s = entry.second;

        // Only aggregate the application's UDP data flows to ports 9000-9003.
        if (tuple.protocol != 17 ||
            tuple.destinationPort < portA ||
            tuple.destinationPort > portD)
        {
            continue;
        }

        txPackets += s.txPackets;
        rxPackets += s.rxPackets;
        lostPackets += s.lostPackets;
        rxBytes += s.rxBytes;
        totalDelay += s.delaySum.GetSeconds();
    }

    double pdr = txPackets
        ? 100.0 * rxPackets / txPackets : 0.0;

    double avgDelayMs = rxPackets
        ? 1000.0 * totalDelay / rxPackets : 0.0;

    double throughputMbps =
        rxBytes * 8.0 / simTime / 1000000.0;

    double lossPercent = txPackets
        ? 100.0 * lostPackets / txPackets : 0.0;

    std::cout << "\n===== ROUTING SIMULATION RESULTS =====\n";
    std::cout << "Protocol: " << protocol << "\n";
    if (protocol == "global")
        std::cout << "Note: Global Routing is an OSPF-like baseline, not OSPF.\n";

    std::cout << "Condition: " << condition << "\n";
    std::cout << "Simulation time: " << simTime << " seconds\n";
    std::cout << "Offered rate per flow: " << offeredRate << " Mbps\n";
    std::cout << "Tx packets: " << txPackets << "\n";
    std::cout << "Rx packets: " << rxPackets << "\n";
    std::cout << "FlowMonitor lost packets: " << lostPackets << "\n";
    std::cout << "PDR: " << pdr << " %\n";
    std::cout << "Loss estimate: " << lossPercent << " %\n";
    std::cout << "Average end-to-end delay: " << avgDelayMs << " ms\n";
    std::cout << "Throughput: " << throughputMbps << " Mbps\n";

    // Append results to a CSV file so different runs can be compared.
    bool writeHeader = false;
    {
        std::ifstream check("results.csv");
        writeHeader = !check.good();
    }

    std::ofstream resultFile("results.csv", std::ios::app);
    if (writeHeader)
    {
        resultFile << "protocol,condition,pdr,delay_ms,throughput_mbps,"
                   << "tx_packets,rx_packets,lost_packets\n";
    }

    resultFile << protocol << "," << condition << ","
               << pdr << "," << avgDelayMs << ","
               << throughputMbps << "," << txPackets << ","
               << rxPackets << "," << lostPackets << "\n";

    monitor->SerializeToXmlFile("flowmon.xml", true, true);

    Simulator::Destroy();
    std::cout << "\nSaved: results.csv, routing-tables.txt, flowmon.xml\n";
    return 0;
}