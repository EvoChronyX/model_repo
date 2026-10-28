
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/ipv4-global-routing-helper.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace ns3;

std::ofstream cwndFile;

// Record TCP congestion-window changes
void CwndChange(std::string context,
                uint32_t oldCwnd,
                uint32_t newCwnd)
{
    cwndFile << Simulator::Now().GetSeconds() << ","
             << context << ","
             << oldCwnd << ","
             << newCwnd << "\n";
}

// Connect traces after the TCP sockets have been created
void ConnectCwndTraces()
{
    Config::Connect(
        "/NodeList/*/$ns3::TcpL4Protocol/"
        "SocketList/*/CongestionWindow",
        MakeCallback(&CwndChange));
}

int main(int argc, char *argv[])
{
    // STEP 1: Simulation parameters
    std::string tcpType = "ns3::TcpNewReno";
    std::string bottleneckRate = "5Mbps";
    double simTime = 30.0;

    CommandLine cmd(__FILE__);
    cmd.AddValue("tcpType", "TCP algorithm TypeId", tcpType);
    cmd.AddValue("bottleneckRate", "Bottleneck bandwidth",
                 bottleneckRate);
    cmd.AddValue("simTime", "Simulation duration", simTime);
    cmd.Parse(argc, argv);

    // STEP 2: Check algorithm availability
    TypeId tid;
    if (!TypeId::LookupByNameFailSafe(tcpType, &tid))
    {
        std::cerr << "ERROR: TCP algorithm not available: "
                  << tcpType << "\n";
        std::cerr << "Check the installed NS-3 version and modules.\n";
        return 1;
    }

    Config::SetDefault(
        "ns3::TcpL4Protocol::SocketType",
        TypeIdValue(tid));

    // STEP 3: Create five vehicles, RSU, router and server
    NodeContainer vehicles;
    vehicles.Create(5);

    Ptr<Node> rsu = CreateObject<Node>();
    Ptr<Node> internetRouter = CreateObject<Node>();
    Ptr<Node> server = CreateObject<Node>();

    NodeContainer all;
    all.Add(vehicles);
    all.Add(rsu);
    all.Add(internetRouter);
    all.Add(server);

    InternetStackHelper internet;
    internet.Install(all);

    // STEP 4: Configure vehicle-to-RSU access links
    PointToPointHelper access;
    access.SetDeviceAttribute(
        "DataRate", StringValue("10Mbps"));
    access.SetChannelAttribute(
        "Delay", StringValue("2ms"));

    // STEP 5: Configure the shared Internet bottleneck
    PointToPointHelper bottleneck;
    bottleneck.SetDeviceAttribute(
        "DataRate", StringValue(bottleneckRate));
    bottleneck.SetChannelAttribute(
        "Delay", StringValue("20ms"));
    bottleneck.SetQueue(
        "ns3::DropTailQueue<Packet>",
        "MaxSize", StringValue("50p"));

    // STEP 6: Connect each vehicle to the RSU
    NetDeviceContainer vehicleDevices[5];

    Ipv4InterfaceContainer vehicleIf[5];

    Ipv4AddressHelper ip;

    for (uint32_t i = 0; i < 5; i++)
    {
        vehicleDevices[i] =
            access.Install(vehicles.Get(i), rsu);

        ip.SetBase(
            ("10.1." + std::to_string(i + 1) + ".0").c_str(),
            "255.255.255.0");

        vehicleIf[i] = ip.Assign(vehicleDevices[i]);
    }

    // RSU -> Internet router
    NetDeviceContainer dR =
        bottleneck.Install(rsu, internetRouter);

    ip.SetBase("10.1.6.0", "255.255.255.0");
    ip.Assign(dR);

    // Internet router -> Traffic server
    NetDeviceContainer dS =
        access.Install(internetRouter, server);

    ip.SetBase("10.1.7.0", "255.255.255.0");
    Ipv4InterfaceContainer serverIf = ip.Assign(dS);

    // STEP 7: Populate routing tables
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // STEP 8: Install the TCP sink on the server
    uint16_t port = 5000;

    PacketSinkHelper sink(
        "ns3::TcpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), port));

    ApplicationContainer sinkApp = sink.Install(server);
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(simTime));

    // STEP 9: Create five competing TCP sources
    ApplicationContainer sourceApps;

    for (uint32_t i = 0; i < 5; i++)
    {
        OnOffHelper source(
            "ns3::TcpSocketFactory",
            InetSocketAddress(serverIf.GetAddress(1), port));

        source.SetAttribute(
            "DataRate", StringValue("5Mbps"));
        source.SetAttribute(
            "PacketSize", UintegerValue(1024));
        source.SetAttribute(
            "OnTime",
            StringValue(
                "ns3::ConstantRandomVariable[Constant=1]"));
        source.SetAttribute(
            "OffTime",
            StringValue(
                "ns3::ConstantRandomVariable[Constant=0]"));

        ApplicationContainer app =
            source.Install(vehicles.Get(i));

        app.Start(Seconds(1.0));
        app.Stop(Seconds(simTime - 1.0));

        sourceApps.Add(app);
    }

    // STEP 10: Record congestion-window changes
    cwndFile.open("vanet-cwnd.csv");

    cwndFile << "Time,SocketContext,OldCwndBytes,NewCwndBytes\n";

    // Allow the applications to create TCP sockets first
    Simulator::Schedule(
        Seconds(1.1), &ConnectCwndTraces);

    // Capture packets at the shared bottleneck
    bottleneck.EnablePcap("vanet-bottleneck", dR, true);

    // STEP 11: Enable FlowMonitor
    FlowMonitorHelper fm;
    Ptr<FlowMonitor> monitor = fm.InstallAll();

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

    monitor->CheckForLostPackets();

    // STEP 12: Calculate performance metrics
    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(fm.GetClassifier());

    std::vector<double> throughputs;

    std::cout << "\n=== VANET TCP PERFORMANCE RESULTS ===\n";
    std::cout << "Algorithm: " << tcpType << "\n";
    std::cout << "Bottleneck: " << bottleneckRate << "\n\n";

    std::cout << std::fixed << std::setprecision(3);

    for (const auto &entry : monitor->GetFlowStats())
    {
        Ipv4FlowClassifier::FiveTuple tuple =
            classifier->FindFlow(entry.first);

        const FlowMonitor::FlowStats &s = entry.second;

        if (tuple.protocol != 6 || s.txPackets == 0)
            continue;

        double duration = 0.0;

        if (s.rxPackets > 0)
        {
            duration =
                (s.timeLastRxPacket -
                 s.timeFirstTxPacket).GetSeconds();
        }

        double throughput = (duration > 0)
            ? (s.rxBytes * 8.0 / duration / 1e6)
            : 0.0;

        double avgDelay = (s.rxPackets > 0)
            ? (s.delaySum.GetSeconds() * 1000.0 /
               s.rxPackets)
            : 0.0;

        double lossPercent = (s.txPackets > 0)
            ? (100.0 * s.lostPackets / s.txPackets)
            : 0.0;

        throughputs.push_back(throughput);

        std::cout << "Flow " << entry.first
                  << "  " << tuple.sourceAddress
                  << " -> " << tuple.destinationAddress << "\n";

        std::cout << "  Tx packets: " << s.txPackets << "\n";
        std::cout << "  Rx packets: " << s.rxPackets << "\n";
        std::cout << "  Lost packets (FlowMonitor): "
                  << s.lostPackets << "\n";
        std::cout << "  Loss percentage: "
                  << lossPercent << "%\n";
        std::cout << "  Throughput: "
                  << throughput << " Mbps\n";
        std::cout << "  Average delay: "
                  << avgDelay << " ms\n\n";
    }

    // STEP 13: Calculate Jain's fairness index
    double sum = 0.0;
    double sumSquares = 0.0;

    for (double x : throughputs)
    {
        sum += x;
        sumSquares += x * x;
    }

    double fairness = (throughputs.size() > 0 &&
                       sumSquares > 0)
        ? (sum * sum /
           (throughputs.size() * sumSquares))
        : 0.0;

    std::cout << "Jain's fairness index: "
              << fairness << "\n";

    // STEP 14: Save statistics and clean up
    monitor->SerializeToXmlFile(
        "vanet-flowmon.xml", true, true);

    cwndFile.close();

    std::cout << "\nFiles generated:\n";
    std::cout << "  vanet-cwnd.csv\n";
    std::cout << "  vanet-flowmon.xml\n";
    std::cout << "  vanet-bottleneck-*.pcap\n";

    Simulator::Destroy();
    return 0;
}


