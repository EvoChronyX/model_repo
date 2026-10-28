
#include "ns3/core-module.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <iomanip>

using namespace ns3;
using namespace std;

struct Radio
{
    string name;
    double frequencyMHz;
    double txPowerDbm;
    double rangeMeters;
    double dataRateMbps;
    double delayMs;
};

int main(int argc, char *argv[])
{
    double speed = 1.0;
    double simTime = 20.0;

    CommandLine cmd;
    cmd.AddValue("speed", "Mobile speed in m/s", speed);
    cmd.AddValue("simTime", "Simulation time in seconds", simTime);
    cmd.Parse(argc, argv);

    if (speed <= 0 || simTime <= 0)
    {
        cerr << "Speed and simulation time must be positive.\n";
        return 1;
    }

    // Assumed radio parameters for the fallback model.
    Radio radios[] = {
        {"WiFi",      2400, 16, 100,  6.0,  5.0},
        {"Bluetooth", 2400,  4,  10,  1.0, 10.0},
        {"Zigbee",    2400,  3,  30,  0.25, 20.0}
    };

    ofstream out("heterogeneous_results.csv");

    out << "Technology,Speed_mps,Method,AvgRxPower_dBm,"
        << "Throughput_Mbps,PacketLoss_pct,Delay_ms\n";

    cout << fixed << setprecision(2);
    cout << "\nHeterogeneous Wireless Comparison\n";
    cout << "Speed: " << speed << " m/s\n";
    cout << "Duration: " << simTime << " seconds\n\n";

    cout << left << setw(12) << "Technology"
         << setw(12) << "RxPower"
         << setw(14) << "Throughput"
         << setw(12) << "Loss %"
         << setw(12) << "Delay ms" << "\n";

    for (const auto &r : radios)
    {
        double totalPower = 0;
        double totalPackets = 0;
        double receivedPackets = 0;
        double totalDelay = 0;

        // Mobile node moves away from the gateway.
        // Initial distance is 1 m.
        int samples = static_cast<int>(simTime * 10);

        for (int i = 0; i < samples; i++)
        {
            double t = i / 10.0;
            double distance = 1.0 + speed * t;

            // Free-space path-loss estimate.
            double distanceKm = distance / 1000.0;
            double pathLoss =
                32.44 +
                20.0 * log10(r.frequencyMHz) +
                20.0 * log10(distanceKm);

            double rxPower = r.txPowerDbm - pathLoss;
            totalPower += rxPower;

            // Estimate packet delivery using the assumed radio range.
            // This is NOT a real PHY/MAC packet reception calculation.
            int packetsThisSample = 10;
            totalPackets += packetsThisSample;

            if (distance <= r.rangeMeters)
            {
                receivedPackets += packetsThisSample;
                totalDelay += packetsThisSample * r.delayMs;
            }
        }

        double avgPower = totalPower / samples;
        double lossPct =
            100.0 * (totalPackets - receivedPackets) / totalPackets;

        double throughput =
            (receivedPackets / totalPackets) * r.dataRateMbps;

        double avgDelay = receivedPackets > 0
                              ? totalDelay / receivedPackets
                              : 0.0;

        cout << left << setw(12) << r.name
             << setw(12) << avgPower
             << setw(14) << throughput
             << setw(12) << lossPct
             << setw(12) << avgDelay << "\n";

        out << r.name << "," << speed
            << ",ANALYTICAL_ESTIMATE,"
            << avgPower << ","
            << throughput << ","
            << lossPct << ","
            << avgDelay << "\n";
    }

    out.close();

    cout << "\nSaved results to heterogeneous_results.csv\n";
    Simulator::Destroy();

    return 0;
}
