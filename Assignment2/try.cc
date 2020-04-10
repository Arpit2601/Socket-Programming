/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/netanim-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("try");

int 
main (int argc, char *argv[])
{
    
    CommandLine cmd;
    cmd.Parse (argc, argv);

    Time::SetResolution (Time::NS);
    LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

    std::string rate_hr = "80Mbps";
	std::string lat_hr = "20ms";
	std::string rate_rr = "30Mbps";
	std::string lat_rr = "100ms";

	uint packetSize = 1.2*1024;		//1.2KB
	uint qsize_hr = (100000*20)/packetSize;
	uint qsize_rr = (10000*100)/packetSize;
    std::string q_hr_str= std::to_string(qsize_hr) + "p";
    std::string q_rr_str= std::to_string(qsize_rr) + "p";


    NS_LOG_INFO ("Create nodes.");
    NodeContainer all_nodes;
    all_nodes.Create(6);

    NodeContainer rl_h1 = NodeContainer (all_nodes.Get (2), all_nodes.Get (0));
    NodeContainer rl_h2 = NodeContainer (all_nodes.Get (2), all_nodes.Get (1));
    NodeContainer rr_h3 = NodeContainer (all_nodes.Get (3), all_nodes.Get (4));
    NodeContainer rr_h4 = NodeContainer (all_nodes.Get (3), all_nodes.Get (5));
    NodeContainer rl_rr = NodeContainer (all_nodes.Get (2), all_nodes.Get (3));    

    InternetStackHelper stack;
    stack.Install (all_nodes);

    NS_LOG_INFO ("Create channels.");
    PointToPointHelper p2p_hr, p2p_rr;
    p2p_hr.SetDeviceAttribute("DataRate", StringValue(rate_hr));
	p2p_hr.SetChannelAttribute("Delay", StringValue(lat_hr));
	p2p_hr.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue (q_hr_str));
	p2p_rr.SetDeviceAttribute("DataRate", StringValue(rate_rr));
	p2p_rr.SetChannelAttribute("Delay", StringValue(lat_rr));
	p2p_rr.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue (q_rr_str));


    NetDeviceContainer dl_d0 = p2p_hr.Install (rl_h1);
    NetDeviceContainer dl_d2 = p2p_hr.Install (rl_h2);
    NetDeviceContainer dr_d3 = p2p_hr.Install (rr_h3);
    NetDeviceContainer dr_d4 = p2p_hr.Install (rr_h4);
    NetDeviceContainer dl_dr = p2p_rr.Install (rl_rr);


    NS_LOG_INFO ("Assign IP Addresses.");
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
    onoff.SetConstantRate (DataRate ("10kbps"));
    onoff.SetAttribute ("PacketSize", UintegerValue (50));

    ApplicationContainer apps = onoff.Install (all_nodes.Get (1));
    apps.Start (Seconds (1.0));
    apps.Stop (Seconds (10.0));

    PacketSinkHelper sink ("ns3::UdpSocketFactory",
                            Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
    apps = sink.Install (all_nodes.Get (4));
    apps.Start (Seconds (1.0));
    apps.Stop (Seconds (10.0));
    // BulkSendHelper ftp("ns3::TcpSocketFactory", InetSocketAddress (ir_i4.GetAddress (1), port));
    // ftp.SetAttribute ("SendSize", UintegerValue (100));

    AnimationInterface anim ("animation.xml");
    anim.SetConstantPosition(all_nodes.Get(0),10.0,10.0);
    anim.SetConstantPosition(all_nodes.Get(1),10.0,30.0);
    anim.SetConstantPosition(all_nodes.Get(2),20.0,20.0);
    anim.SetConstantPosition(all_nodes.Get(3),30.0,20.0);
    anim.SetConstantPosition(all_nodes.Get(4),40.0,30.0);
    anim.SetConstantPosition(all_nodes.Get(5),40.0,10.0);
    NS_LOG_INFO ("Running Simulation.");
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_INFO ("Done :)");
}