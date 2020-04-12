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

void ThroughputMonitor (FlowMonitorHelper* flowmonHelper, Ptr<FlowMonitor> flowMon)
  { 
    flowMon->CheckForLostPackets(); 
    std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats();
    Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (flowmonHelper->GetClassifier());
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
        std::cout<<"Delay Sum :"<<stats->second.delaySum<<std::endl;;
        // std::cout<<stats->second.rxBytes<<" "<<stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds()<<'\n';
        std::cout<<"---------------------------------------------------------------------------"<<std::endl;
        throughfout<<(double)stats->first<<", "<<throughput<<", ";
        delayfout<<(double)stats->first<<", "<<stats->second.delaySum.GetMilliSeconds()-delays[stats->first]<<", ";
        delays[stats->first]=stats->second.delaySum.GetMilliSeconds();
    } 
    throughfout<<'\n';
    delayfout<<'\n';
    flowMon->SerializeToXmlFile("lab-5.flowmon", true, true);
    Simulator::Schedule(MilliSeconds(100),&ThroughputMonitor, flowmonHelper, flowMon);      
}

int 
main (int argc, char *argv[])
{
    Gnuplot2dDataset UTdataset, UDdataset, STdataset, SDdataset;
    // dataset.SetTitle ("UDP_throughput");
	UTdataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);
    // delay_dataset.SetTitle ("UDP_delay");
	UDdataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);
    SDdataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);
    STdataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);
    


    CommandLine cmd;
    cmd.Parse (argc, argv);
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TypeId::LookupByName ("ns3::TcpScalable")));
    throughfout.open("throughput_udp_alone.csv", std::ios::out);
    delayfout.open("delay_udp_alone.csv", std::ios::out);

    std::string rate_hr = "80Mbps";
	std::string lat_hr = "20ms";
	std::string rate_rr = "30Mbps";
	std::string lat_rr = "100ms";
    double simulation_time = 10.0;
    // uint32_t packet_size_tcp = 1000;
    // uint32_t packet_size = 50;  

	// uint packetSize = 1.2*1024;		//1.2KB
	uint qsize_hr = 100000*20;
	uint qsize_rr = 100*3750;
    std::string q_hr_str= std::to_string(qsize_hr) + "p";
    std::string q_rr_str= std::to_string(qsize_rr) + "p";


    std::cout<<"Create nodes."<<std::endl;;
    

   
    //-----------------------------------------------------
    // Sending data from h1 to h3
    // UDP server
    uint16_t port = 2;
    for (uint32_t packet_size = 512; packet_size < 5000; packet_size+=64)
    {
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

        std::cout<<"Create channels."<<std::endl;;
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
        OnOffHelper onoff ("ns3::UdpSocketFactory", InetSocketAddress (ir_i3.GetAddress (1), port));    //h3
        onoff.SetConstantRate (DataRate ("1000kbps"));   // why 100kbps
        onoff.SetAttribute ("PacketSize", UintegerValue (packet_size));

        ApplicationContainer client_apps = onoff.Install (all_nodes.Get (0));  //h1-->server node


        // UDP receiver
        PacketSinkHelper sink ("ns3::UdpSocketFactory",
                                Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
        ApplicationContainer apps = sink.Install (all_nodes.Get (4));    // h3->client node
        // apps = sink.Install (all_nodes.Get (4));    // h3
        apps.Start (Seconds (1.0));
        apps.Stop (Seconds (simulation_time));

        client_apps.Start (Seconds (1.0));
        client_apps.Stop (Seconds (simulation_time));

        //--------------------------------------------TCP
        BulkSendHelper ftp("ns3::TcpSocketFactory", InetSocketAddress (ir_i4.GetAddress (1), port));    // h4 is sink
        ftp.SetAttribute ("SendSize", UintegerValue (packet_size));
        ftp.SetAttribute ("MaxBytes", UintegerValue (0));
        ApplicationContainer ftpApp = ftp.Install (all_nodes.Get (1));
        ftpApp.Start (Seconds (1.0));
        ftpApp.Stop (Seconds (simulation_time)); 
        

        PacketSinkHelper sink3 ("ns3::TcpSocketFactory",
                                Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
        ApplicationContainer sink_ftpApp = sink3.Install (all_nodes.Get (5));
        sink_ftpApp.Start (Seconds (1.0));
        sink_ftpApp.Stop (Seconds (simulation_time));
        //------------------------------------------------

        //----------------------------------------------------------
        NS_LOG_INFO("Run Simulation");
        Ptr<FlowMonitor> flowmon;
        FlowMonitorHelper flowmonHelper;
        flowmon = flowmonHelper.InstallAll();

        flowmon->SetAttribute("DelayBinWidth", DoubleValue(0.001));
        flowmon->SetAttribute("JitterBinWidth", DoubleValue(0.001));
        flowmon->SetAttribute("PacketSizeBinWidth", DoubleValue(20)); 
        double Uthroughput=0, Sthroughput=0;
        int count=0;
        double Udelay=0, Sdelay=0;
        int Utotal_packets=0, Stotal_packets=0;
        Simulator::Stop(Seconds(12.0));
        Simulator::Run();
        

        // Simulator::Stop(Seconds(simulation_time+2));
        // ThroughputMonitor(&flowmonHelper ,flowmon);
        //-------------------------
        flowmon->CheckForLostPackets(); 
        std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowmon->GetFlowStats();
        Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier());
        Time now = Simulator::Now (); 
        // throughfout<<now<<", ";
        // delayfout<<now<<", ";
        
        for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats)
        { 
            count++;
            Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow (stats->first);
            if(fiveTuple.sourceAddress=="172.16.1.2" && fiveTuple.destinationAddress=="172.16.3.2")
            {
                std::cout<<"UDP found"<<std::endl;
                Utotal_packets = stats->second.txPackets;
                Uthroughput += stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds() - stats->second.timeFirstTxPacket.GetSeconds())/1024/1024;            // std::cout<<"Throughput: " << throughput << " Mbps"<<std::endl;
                Udelay = stats->second.delaySum.GetSeconds();
            }
            else if(fiveTuple.sourceAddress=="172.16.2.2" && fiveTuple.destinationAddress=="172.16.4.2")
            {
                // if(count==1)
                // {
                std::cout<<"TCP found"<<std::endl;
                Stotal_packets = stats->second.txPackets;
                Sthroughput += stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds() - stats->second.timeFirstTxPacket.GetSeconds())/1024/1024;            // std::cout<<"Throughput: " << throughput << " Mbps"<<std::endl;
                Sdelay = stats->second.delaySum.GetSeconds();
                // }
            }
            
            std::cout<<"Flow ID     : " << stats->first <<" ; "<< fiveTuple.sourceAddress <<" -----> "<<fiveTuple.destinationAddress<<std::endl;
            std::cout<<"Tx Packets = " << stats->second.txPackets<<std::endl;
            // total_packets = stats->second.txPackets;
            // std::cout<<"Rx Packets = " << stats->second.rxPackets<<std::endl;
            // std::cout<<"Duration    : "<<stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds()<<std::endl;
            // std::cout<<"Last Received Packet  : "<< stats->second.timeLastRxPacket.GetSeconds()<<" Seconds"<<std::endl;
            // throughput += stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds() - stats->second.timeFirstTxPacket.GetSeconds())/1024/1024;            // std::cout<<"Throughput: " << throughput << " Mbps"<<std::endl;
            // std::cout<< "Net Packet Lost: " << stats->second.lostPackets << "\n";
            std::cout<<"Delay Sum :"<<stats->second.delaySum<<std::endl;
            // delay = stats->second.delaySum.GetSeconds();
            // std::cout<<stats->second.rxBytes<<" "<<stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds()<<'\n';
            std::cout<<"---------------------------------------------------------------------------"<<std::endl;
            // throughfout<<(double)stats->first<<", "<<throughput<<", ";
            // delayfout<<(double)stats->first<<", "<<stats->second.delaySum.GetMilliSeconds()-delays[stats->first]<<", ";
            // delays[stats->first]=stats-/>second.delaySum.GetMilliSeconds();
        }
        std::cout<<packet_size<<" "<<Uthroughput<<" "<<count<<std::endl;
        std::cout<<packet_size<<" "<<Udelay/Utotal_packets<<" "<<count<<std::endl;
        std::cout<<packet_size<<" "<<Sthroughput<<" "<<count<<std::endl;
        std::cout<<packet_size<<" "<<Sdelay/Stotal_packets<<" "<<count<<std::endl;

        // UTdataset, UDdataset, STdataset, SDdataset;
        UTdataset.Add(packet_size, Uthroughput);
        UDdataset.Add(packet_size, Udelay/Utotal_packets);
        STdataset.Add(packet_size, Sthroughput);
        SDdataset.Add(packet_size, Sdelay/Stotal_packets);

        Simulator::Destroy(); 
        //-------------------------------------- 
        
    }
    
    

    //Initialize Plot file names  
    std :: string fileNameWithNoExtension = "USthroughput";
    std :: string graphicsFileName        = fileNameWithNoExtension + ".png";
    std :: string plotFileName            = fileNameWithNoExtension + ".plt";
    std :: string plotTitle               = "udp and scalable throughput vs packet size";

    std :: string fileNameWithNoExtension_delay = "USdelay";
    std :: string graphicsFileName_delay        = fileNameWithNoExtension_delay + ".png";
    std :: string plotFileName_delay            = fileNameWithNoExtension_delay + ".plt";
    std :: string plotTitle_delay               = "udp and scalable delay vs packet size";

   
    // Instantiate the plot and set its title.
    Gnuplot plot (graphicsFileName);
    Gnuplot plot_delay (graphicsFileName_delay);

    plot.SetTitle (plotTitle);
    plot_delay.SetTitle(plotTitle_delay);
    
    // Make the graphics file, which the plot file will create when it
    // is used with Gnuplot, be a PNG file.
    plot.SetTerminal ("png");
    plot_delay.SetTerminal("png");
    
    // Set the labels for each axis.
    // plot.SetLegend ("Buffer Size(packets)", "Fairness");
    
    //Set the x value ranges for each plots
    // plot.AppendExtra ("set xrange [0:800]");
    plot.AppendExtra ("set yrange [0:+3.0]");
    plot_delay.AppendExtra ("set yrange [0.135:+0.143]");
    
    
    // Add the dataset to the plot.
    plot.AddDataset (UTdataset);
    plot.AddDataset(STdataset);
    plot_delay.AddDataset(UDdataset);
    plot_delay.AddDataset(SDdataset);
    
    // Open the plot file for fairness
    std::ofstream plotFile (plotFileName.c_str());
    std::ofstream plotFile_delay (plotFileName_delay.c_str());
    // Write the plot file.
    plot.GenerateOutput (plotFile);
    plot_delay.GenerateOutput (plotFile_delay);
    // Close the plot file.
    plotFile.close ();
    plotFile_delay.close();
    //Open the plot file for TCP throughput
    Simulator::Destroy ();
    NS_LOG_INFO ("Done :)");
    
}
