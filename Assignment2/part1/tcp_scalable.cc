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
    Gnuplot2dDataset  tcp_dataset, delay_dataset;
    tcp_dataset.SetTitle ("TCP_Scalable_throughput");
    tcp_dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);
    delay_dataset.SetTitle ("TCP_Scalable_delay");
    delay_dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);


    CommandLine cmd;
    cmd.Parse (argc, argv);
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TypeId::LookupByName ("ns3::TcpScalable")));

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
    
    // packet size is in bytes
    for (uint32_t packet_size_tcp = 512; packet_size_tcp < 5000; packet_size_tcp+=64)
    {
        qsize_hr = 10000*20/packet_size_tcp;
        qsize_rr = 100*3750/packet_size_tcp;
        q_hr_str= std::to_string(qsize_hr) + "p";
        q_rr_str= std::to_string(qsize_rr) + "p";
        Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(packet_size_tcp));
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


        std::cout<<"Assign IP Addresses."<<std::endl;;
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
        BulkSendHelper ftp("ns3::TcpSocketFactory", InetSocketAddress (ir_i4.GetAddress (1), port));    // h4 is sink
        ftp.SetAttribute ("SendSize", UintegerValue (packet_size_tcp));
        ftp.SetAttribute ("MaxBytes", UintegerValue (0));
        ApplicationContainer ftpApp = ftp.Install (all_nodes.Get (0));
        ftpApp.Start (Seconds (1.0));
        ftpApp.Stop (Seconds (simulation_time)); 
        

        PacketSinkHelper sink3 ("ns3::TcpSocketFactory",
                                Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
        ApplicationContainer sink_ftpApp = sink3.Install (all_nodes.Get (5));
        sink_ftpApp.Start (Seconds (1.0));
        sink_ftpApp.Stop (Seconds (simulation_time));
        //----------------------------------------------------------
        
        double throughput=0;
        int count=0;
        double delay=0;
        int total_packets=0;
        NS_LOG_INFO("Run Simulation");
        Ptr<FlowMonitor> flowmon;
        FlowMonitorHelper flowmonHelper;
        flowmon = flowmonHelper.InstallAll();

        flowmon->SetAttribute("DelayBinWidth", DoubleValue(0.001));
        flowmon->SetAttribute("JitterBinWidth", DoubleValue(0.001));
        flowmon->SetAttribute("PacketSizeBinWidth", DoubleValue(20)); 
        Simulator::Stop(Seconds(11.0));
        Simulator::Run();
        
        //-------------------------
        flowmon->CheckForLostPackets(); 
        std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowmon->GetFlowStats();
        Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier());
        Time now = Simulator::Now (); 
        
        for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats)
        { 
            count++;
            if(count==1)total_packets = stats->second.txPackets;
            if(count==1)throughput = stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds() - stats->second.timeFirstTxPacket.GetSeconds())/1024/1024;           
            if(count==1)delay = stats->second.delaySum.GetSeconds();
        }
        
        std::cout<<"Throughput for packet size: "<<packet_size_tcp<<" is "<<throughput<<"Mbps"<<std::endl;
        std::cout<<"Delay for packet size: "<<packet_size_tcp<<" is "<<delay/total_packets<<"s "<<std::endl;    
        tcp_dataset.Add(packet_size_tcp, throughput);
        delay_dataset.Add(packet_size_tcp, delay/total_packets);
        Simulator::Destroy(); 
        //-------------------------------------- 
        
    }

    std :: string fileNameWithNoExtension_tcp = "TCP_Scalable_throughput";
    std :: string graphicsFileName_tcp        = fileNameWithNoExtension_tcp + ".png";
    std :: string plotFileName_tcp            = fileNameWithNoExtension_tcp + ".plt";
    std :: string plotTitle_tcp               = "tcp scaalable throughput vs packet size";

    std :: string fileNameWithNoExtension_tcp_delay = "TCP_Scalable_delay";
    std :: string graphicsFileName_tcp_delay        = fileNameWithNoExtension_tcp_delay + ".png";
    std :: string plotFileName_tcp_delay            = fileNameWithNoExtension_tcp_delay + ".plt";
    std :: string plotTitle_tcp_delay               = "tcp scalable delay vs packet size";
    Gnuplot plot_tcp (graphicsFileName_tcp);
    Gnuplot plot_tcp_delay (graphicsFileName_tcp_delay);

    plot_tcp.SetTitle(plotTitle_tcp);
    plot_tcp_delay.SetTitle(plotTitle_tcp_delay);

    plot_tcp.SetTerminal("png");
    plot_tcp_delay.SetTerminal("png");

    plot_tcp.AppendExtra ("set yrange [0:+3.0]");
    plot_tcp_delay.AppendExtra ("set yrange [0.140:+0.143]");

    
    plot_tcp.AddDataset (tcp_dataset);
    plot_tcp_delay.AddDataset (delay_dataset);

    std::ofstream plotFile_tcp (plotFileName_tcp.c_str());
    std::ofstream plotFile_tcp_delay (plotFileName_tcp_delay.c_str());

    plot_tcp.GenerateOutput(plotFile_tcp);
    plot_tcp_delay.GenerateOutput(plotFile_tcp_delay);
    plotFile_tcp.close();
    plotFile_tcp_delay.close();

    Simulator::Destroy ();
    NS_LOG_INFO ("Done :)");
    
}
