/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <map>
#include <string>
#include <fstream>
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/stats-module.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("try");

int 
main (int argc, char *argv[])
{
    Gnuplot2dDataset UTdataset, UDdataset, STdataset, SDdataset;
	UTdataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);
	UDdataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);
    SDdataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);
    STdataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);
    


    CommandLine cmd;
    cmd.Parse (argc, argv);
    std::string tcpModel ("ns3::TcpScalable");
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue (tcpModel));

    std::string rate_hr = "80Mbps";
	std::string lat_hr = "20ms";
	std::string rate_rr = "30Mbps";
	std::string lat_rr = "100ms";
    double simulation_time = 10.0; 

	uint qsize_hr;
	uint qsize_rr;
    std::string q_hr_str;
    std::string q_rr_str;

    uint16_t port = 2;
    // packet size in bytes
    std::cout<<"UDP packet flow from 172.16.1.2 to  172.16.3.2"<<std::endl;
    std::cout<<"UDP packet flow from 172.16.2.2 to  172.16.4.2"<<std::endl;
    for (uint32_t packet_size = 512; packet_size < 5000; packet_size+=64)
    {
        qsize_hr = 10000*20/packet_size;
        qsize_rr = 100*3750/packet_size;
        q_hr_str= std::to_string(qsize_hr) + "p";
        q_rr_str= std::to_string(qsize_rr) + "p";
        Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(packet_size));
        NodeContainer all_nodes;
        all_nodes.Create(6);

        NodeContainer rl_h1 = NodeContainer (all_nodes.Get (2), all_nodes.Get (0));
        NodeContainer rl_h2 = NodeContainer (all_nodes.Get (2), all_nodes.Get (1));
        NodeContainer rr_h3 = NodeContainer (all_nodes.Get (3), all_nodes.Get (4));
        NodeContainer rr_h4 = NodeContainer (all_nodes.Get (3), all_nodes.Get (5));
        NodeContainer rl_rr = NodeContainer (all_nodes.Get (2), all_nodes.Get (3));    

        InternetStackHelper stack;
        stack.Install (all_nodes);

        // std::cout<<"Create channels."<<std::endl;;
        PointToPointHelper p2p_hr,p2p_rr;
        p2p_hr.SetDeviceAttribute("DataRate", StringValue(rate_hr));
        p2p_hr.SetChannelAttribute("Delay", StringValue(lat_hr));
        p2p_hr.SetQueue ("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue (QueueSize (q_hr_str)));
        // p2p_hr.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize",StringValue("200000"));

        NetDeviceContainer dl_d1 = p2p_hr.Install (rl_h1);
        NetDeviceContainer dl_d2 = p2p_hr.Install (rl_h2);
        NetDeviceContainer dr_d3 = p2p_hr.Install (rr_h3);
        NetDeviceContainer dr_d4 = p2p_hr.Install (rr_h4);

        p2p_rr.SetDeviceAttribute("DataRate", StringValue(rate_rr));
        p2p_rr.SetChannelAttribute("Delay", StringValue(lat_rr));
        p2p_rr.SetQueue ("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue (QueueSize (q_rr_str))); 
        // p2p_rr.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize",StringValue("375000"));

        NetDeviceContainer dl_dr = p2p_rr.Install (rl_rr);


        // std::cout<<"Assign IP Addresses."<<std::endl;;
        Ipv4AddressHelper ipv4;
        ipv4.SetBase ("172.16.1.0", "255.255.255.0");
        Ipv4InterfaceContainer il_i0 = ipv4.Assign (dl_d1);

        ipv4.SetBase ("172.16.2.0", "255.255.255.0");
        Ipv4InterfaceContainer il_i2 = ipv4.Assign (dl_d2);

        ipv4.SetBase ("172.16.3.0", "255.255.255.0");
        Ipv4InterfaceContainer ir_i3 = ipv4.Assign (dr_d3);

        ipv4.SetBase ("172.16.4.0", "255.255.255.0");
        Ipv4InterfaceContainer ir_i4 = ipv4.Assign (dr_d4);

        ipv4.SetBase ("10.250.1.0", "255.255.255.0");
        Ipv4InterfaceContainer il_ir = ipv4.Assign (dl_dr);

        Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

        
        /* code */
        OnOffHelper onoff ("ns3::UdpSocketFactory", InetSocketAddress (ir_i3.GetAddress (1), port));    //h3
        onoff.SetConstantRate (DataRate ("1000kbps"));   // why 100kbps
        onoff.SetAttribute ("PacketSize", UintegerValue (packet_size));

        ApplicationContainer client_apps = onoff.Install (all_nodes.Get (0));  //h1-->server node


        // UDP receiver
        PacketSinkHelper sink ("ns3::UdpSocketFactory",
                                Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
        ApplicationContainer apps = sink.Install (all_nodes.Get (4));    // h3->client node
        apps.Start (Seconds (1.0));
        apps.Stop (Seconds (simulation_time));

        client_apps.Start (Seconds (1.0));
        client_apps.Stop (Seconds (simulation_time));

        //--------------------------------------------TCP
        BulkSendHelper ftp("ns3::TcpSocketFactory", InetSocketAddress (ir_i4.GetAddress (1), port));    // h4 is sink
        ftp.SetAttribute ("SendSize", UintegerValue (packet_size));
        ftp.SetAttribute ("MaxBytes", UintegerValue (0));
        ApplicationContainer ftpApp = ftp.Install (all_nodes.Get (1));
        ftpApp.Start (Seconds (5.0));
        ftpApp.Stop (Seconds (simulation_time+5)); 
        

        PacketSinkHelper sink3 ("ns3::TcpSocketFactory",
                                Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
        ApplicationContainer sink_ftpApp = sink3.Install (all_nodes.Get (5));
        sink_ftpApp.Start (Seconds (5.0));
        sink_ftpApp.Stop (Seconds (simulation_time+5));
        //------------------------------------------------

        //----------------------------------------------------------
        NS_LOG_INFO("Run Simulation");
        Ptr<FlowMonitor> flowmon;
        FlowMonitorHelper flowmonHelper;
        flowmon = flowmonHelper.InstallAll();

        flowmon->SetAttribute("DelayBinWidth", DoubleValue(0.001));
        flowmon->SetAttribute("JitterBinWidth", DoubleValue(0.001));
        flowmon->SetAttribute("PacketSizeBinWidth", DoubleValue(20)); 
        double Uthroughput=0, Hthroughput=0;
        int count=0;
        double Udelay=0, Hdelay=0;
        int Utotal_packets=0, Htotal_packets=0;
        Simulator::Stop(Seconds(16.0));
        Simulator::Run();
        
        flowmon->CheckForLostPackets(); 
        std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowmon->GetFlowStats();
        Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier());
        Time now = Simulator::Now (); 
        
        for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats)
        { 
            count++;
            Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow (stats->first);
            if(fiveTuple.sourceAddress=="172.16.1.2" && fiveTuple.destinationAddress=="172.16.3.2")
            {
                Utotal_packets = stats->second.txPackets;
                Uthroughput += stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds() - stats->second.timeFirstTxPacket.GetSeconds())/1024/1024;            // std::cout<<"Throughput: " << throughput << " Mbps"<<std::endl;
                Udelay = stats->second.delaySum.GetSeconds();
            }
            else if(fiveTuple.sourceAddress=="172.16.2.2" && fiveTuple.destinationAddress=="172.16.4.2")
            {
                Htotal_packets = stats->second.txPackets;
                Hthroughput += stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds() - stats->second.timeFirstTxPacket.GetSeconds())/1024/1024;            // std::cout<<"Throughput: " << throughput << " Mbps"<<std::endl;
                Hdelay = stats->second.delaySum.GetSeconds();
            }
        }
        std::cout<<"Throughput of UDP for packet size: "<<packet_size<<" is "<<Uthroughput<<"Mbps"<<std::endl;
        std::cout<<"Delay of UDP for packet size: "<<packet_size<<" is "<<Udelay/Utotal_packets<<"s "<<std::endl; 
        std::cout<<"Throughput of TCP for packet size: "<<packet_size<<" is "<<Hthroughput<<"Mbps"<<std::endl;
        std::cout<<"Delay of TCP for packet size: "<<packet_size<<" is "<<Hdelay/Htotal_packets<<"s "<<std::endl;
        std::cout<<"------------------------------------------------------"<<std::endl<<std::endl;

        UTdataset.Add(packet_size, Uthroughput);
        UDdataset.Add(packet_size, Udelay/Utotal_packets);
        STdataset.Add(packet_size, Hthroughput);
        SDdataset.Add(packet_size, Hdelay/Htotal_packets);

        Simulator::Destroy(); 
        //-------------------------------------- 
        
    }
    std :: string fileNameWithNoExtension = "USthroughput";
    std :: string graphicsFileName        = fileNameWithNoExtension + ".png";
    std :: string plotFileName            = fileNameWithNoExtension + ".plt";
    std :: string plotTitle               = "udp and scalable throughput vs packet size";

    std :: string fileNameWithNoExtension_delay = "USdelay";
    std :: string graphicsFileName_delay        = fileNameWithNoExtension_delay + ".png";
    std :: string plotFileName_delay            = fileNameWithNoExtension_delay + ".plt";
    std :: string plotTitle_delay               = "udp and scalable delay vs packet size";

   
    Gnuplot plot (graphicsFileName);
    Gnuplot plot_delay (graphicsFileName_delay);

    plot.SetTitle (plotTitle);
    plot_delay.SetTitle(plotTitle_delay);
    

    plot.SetTerminal ("png");
    plot_delay.SetTerminal("png");
    

    plot.AppendExtra ("set yrange [0:+3.0]");
    plot_delay.AppendExtra ("set yrange [0.135:+0.143]");
    
    
    plot.AddDataset (UTdataset);
    plot.AddDataset(STdataset);
    plot_delay.AddDataset(UDdataset);
    plot_delay.AddDataset(SDdataset);
    
    std::ofstream plotFile (plotFileName.c_str());
    std::ofstream plotFile_delay (plotFileName_delay.c_str());
    plot.GenerateOutput (plotFile);
    plot_delay.GenerateOutput (plotFile_delay);
    plotFile.close ();
    plotFile_delay.close();
    Simulator::Destroy ();
    NS_LOG_INFO ("Done :)");
    
}
