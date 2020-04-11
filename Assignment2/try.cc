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
std::fstream throughfout,delayfout;
int64_t delays[3]={(int64_t)0};
NS_LOG_COMPONENT_DEFINE("try");

void ThroughputMonitor (FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> flowMon)
  { 
    flowMon->CheckForLostPackets(); 
    std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats();
    Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());
    Time now = Simulator::Now (); 
    throughfout<<now<<", ";
    delayfout<<now<<", ";
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats)
    { 
        Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow (stats->first);
        std::cout<<"Flow ID     : " << stats->first <<" ; "<< fiveTuple.sourceAddress <<" -----> "<<fiveTuple.destinationAddress<<std::endl;
        std::cout<<"Tx Packets = " << stats->second.txPackets<<std::endl;
        std::cout<<"Rx Packets = " << stats->second.rxPackets<<std::endl;
        std::cout<<"Duration    : "<<stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds()<<std::endl;
        std::cout<<"Last Received Packet  : "<< stats->second.timeLastRxPacket.GetSeconds()<<" Seconds"<<std::endl;
        double throughput =stats->second.rxBytes * 8.0 / ((stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds())*1024*1024);
        std::cout<<"Throughput: " << throughput << " Mbps"<<std::endl;
        std::cout<< "Net Packet Lost: " << stats->second.lostPackets << "\n";
        std::cout<<"Delay Sum :"<<stats->second.delaySum;
        // std::cout<<stats->second.rxBytes<<" "<<stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds()<<'\n';
        std::cout<<"---------------------------------------------------------------------------"<<std::endl;
        throughfout<<(double)stats->first<<", "<<throughput<<", ";
        delayfout<<(double)stats->first<<", "<<stats->second.delaySum.GetMilliSeconds()-delays[stats->first]<<", ";
        delays[stats->first]=stats->second.delaySum.GetMilliSeconds();
    } 
    throughfout<<'\n';
    delayfout<<'\n';
    flowMon->SerializeToXmlFile("lab-5.flowmon", true, true);
    Simulator::Schedule(MilliSeconds(100),&ThroughputMonitor, fmhelper, flowMon);      
}

int 
main (int argc, char *argv[])
{
    
    CommandLine cmd;
    cmd.Parse (argc, argv);
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TypeId::LookupByName ("ns3::TcpScalable")));
    throughfout.open("throughput.csv", std::ios::out);
    delayfout.open("delay.csv", std::ios::out);

    std::string rate_hr = "100Mbps";
	std::string lat_hr = "20ms";
	std::string rate_rr = "30Mbps";
	std::string lat_rr = "100ms";
    double simulation_time = 10.0;
    uint32_t packet_size_tcp = 1000;
    uint32_t packet_size_udp = 50;  

	uint packetSize = 1.2*1024;		//1.2KB
	uint qsize_hr = 100000*20;
	uint qsize_rr = 100*30000;
    std::string q_hr_str= std::to_string(qsize_hr) + "p";
    std::string q_rr_str= std::to_string(qsize_rr) + "p";


   std::cout<<"Create nodes.";
    NodeContainer all_nodes;
    all_nodes.Create(6);

    NodeContainer rl_h1 = NodeContainer (all_nodes.Get (2), all_nodes.Get (0));
    NodeContainer rl_h2 = NodeContainer (all_nodes.Get (2), all_nodes.Get (1));
    NodeContainer rr_h3 = NodeContainer (all_nodes.Get (3), all_nodes.Get (4));
    NodeContainer rr_h4 = NodeContainer (all_nodes.Get (3), all_nodes.Get (5));
    NodeContainer rl_rr = NodeContainer (all_nodes.Get (2), all_nodes.Get (3));    

    InternetStackHelper stack;
    stack.Install (all_nodes);

    std::cout<<"Create channels.";
    PointToPointHelper p2p_hr,p2p_rr;
    p2p_hr.SetDeviceAttribute("DataRate", StringValue(rate_hr));
	p2p_hr.SetChannelAttribute("Delay", StringValue(lat_hr));
	p2p_hr.SetQueue ("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue (QueueSize (q_hr_str)));
    
    NetDeviceContainer dl_d0 = p2p_hr.Install (rl_h1);
    NetDeviceContainer dl_d2 = p2p_hr.Install (rl_h2);
    NetDeviceContainer dr_d3 = p2p_hr.Install (rr_h3);
    NetDeviceContainer dr_d4 = p2p_hr.Install (rr_h4);

	p2p_rr.SetDeviceAttribute("DataRate", StringValue(rate_rr));
	p2p_rr.SetChannelAttribute("Delay", StringValue(lat_rr));
	p2p_rr.SetQueue ("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue (QueueSize (q_rr_str))); 


    NetDeviceContainer dl_dr = p2p_rr.Install (rl_rr);


    std::cout<<"Assign IP Addresses.";
    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("172.16.1.0", "255.255.255.0");
    Ipv4InterfaceContainer il_i0 = ipv4.Assign (dl_d0);

    ipv4.SetBase ("172.16.2.0", "255.255.255.0");
    Ipv4InterfaceContainer il_i2 = ipv4.Assign (dl_d2);

    ipv4.SetBase ("172.16.3.0", "255.255.255.0");
    Ipv4InterfaceContainer ir_i3 = ipv4.Assign (dr_d3);

    ipv4.SetBase ("172.16.4.0", "255.255.255.0");
    Ipv4InterfaceContainer ir_i4 = ipv4.Assign (dr_d4);

    ipv4.SetBase ("10.250.1.0", "255.255.255.0");
    Ipv4InterfaceContainer il_ir = ipv4.Assign (dl_dr);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    uint16_t port = 2;
    OnOffHelper onoff ("ns3::UdpSocketFactory", InetSocketAddress (ir_i3.GetAddress (1), port));
    onoff.SetConstantRate (DataRate ("100kbps"));
    onoff.SetAttribute ("PacketSize", UintegerValue (packet_size_udp));

    ApplicationContainer apps = onoff.Install (all_nodes.Get (1));
    apps.Start (Seconds (1.0));
    apps.Stop (Seconds (simulation_time));

    PacketSinkHelper sink ("ns3::UdpSocketFactory",
                            Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
    apps = sink.Install (all_nodes.Get (4));
    apps.Start (Seconds (1.0));
    apps.Stop (Seconds (simulation_time));


    BulkSendHelper ftp("ns3::TcpSocketFactory", InetSocketAddress (ir_i4.GetAddress (1), port));
    ftp.SetAttribute ("SendSize", UintegerValue (packet_size_tcp));
    ftp.SetAttribute ("MaxBytes", UintegerValue (0));
    ApplicationContainer ftpApp = ftp.Install (all_nodes.Get (0));
    ftpApp.Start (Seconds (1.0));
    ftpApp.Stop (Seconds (simulation_time)); 
    

    PacketSinkHelper sink3 ("ns3::TcpSocketFactory",
                            Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
    ftpApp = sink3.Install (all_nodes.Get (5));
    ftpApp.Start (Seconds (1.0));
    ftpApp.Stop (Seconds (simulation_time));
    Ptr<FlowMonitor> flowmon;
	FlowMonitorHelper flowmonHelper;
	flowmon = flowmonHelper.InstallAll();

    flowmon->SetAttribute("DelayBinWidth", DoubleValue(0.001));
    flowmon->SetAttribute("JitterBinWidth", DoubleValue(0.001));
    flowmon->SetAttribute("PacketSizeBinWidth", DoubleValue(20)); 

    AnimationInterface anim ("animation.xml");
    anim.SetConstantPosition(all_nodes.Get(0),10.0,10.0);
    anim.SetConstantPosition(all_nodes.Get(1),10.0,60.0);
    anim.SetConstantPosition(all_nodes.Get(2),40.0,40.0);
    anim.SetConstantPosition(all_nodes.Get(3),60.0,40.0);
    anim.SetConstantPosition(all_nodes.Get(4),80.0,60.0);
    anim.SetConstantPosition(all_nodes.Get(5),80.0,10.0);


	Simulator::Stop(Seconds(simulation_time+2));
    ThroughputMonitor(&flowmonHelper ,flowmon);
	Simulator::Run();
    Simulator::Destroy ();
    NS_LOG_INFO ("Done :)");
    
}