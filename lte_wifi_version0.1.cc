/*
 * lte.cc
 *
 *  Created on: Jul 17, 2019
 *      Author: ubuntu
 * Based on anas version https://groups.google.com/forum/#!searchin/ns-3-users/anas%7Csort:date/ns-3-users/ybKYurgIF98/UNlvKrdECwAJ
 * Version: 0.1 - 2020/03/20 by Laura
 *change the wi-fi network to mode AdhocWifiMac, changing this interface to Lte after 3 seconds
 *Getting the throughput of both wifi and lte interfaces, but it seems the same using Simulator::Schedule(Seconds(1), &CalculateThroughput);
 * To exchange interfaces we used
 Simulator::Schedule(Seconds(3), &TearDownLink, ueNode.Get(0), pgw, 2, 2);
 Simulator::Schedule (Seconds (4), &reconfigureUdpClient, dlClient, ueNode.Get(0), dport1);
 */


/*
To simulate with logs of internetIpIfaces
./waf --run "scratch/lte_wifi_anas" 2>&1 | tee analyse.txt


*/


#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/applications-module.h"
#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/yans-wifi-helper.h"

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"

#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/internet-module.h"
#include "ns3/global-route-manager.h"
#include "ns3/ipcs-classifier-record.h"
#include "ns3/service-flow.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/point-to-point-helper.h"
#include <iomanip>
#include <string>
#include <fstream>
#include <vector>
#include <iostream>

NS_LOG_COMPONENT_DEFINE ("LteWifiSimpleExample");

using namespace ns3;
uint32_t totalBytesReceived = 0;
int const sources= 2;
std::vector <double> throughputA(sources);
std::ofstream rdTrace;
std::ofstream rdTrace2;
std::vector <uint32_t> totalBytesReceived2 (sources);


/*uint32_t bytesTotal; ///< total bytes received by all nodes
std::string m_CSVfileName= "DsdvManetExample.csv";
uint32_t packetsReceived; ///< total packets received by all nodes
*/

void TearDownLink (Ptr<Node> nodeA, Ptr<Node> nodeB, uint32_t interfaceA, uint32_t interfaceB)
{
  std::cout << "Setting down Remote Host -> Ue 1" << std::endl;
  /*
  std::cout << "Remote Host " << nodeA->GetObject<Ipv4>()->GetAddress(interfaceA,0).GetLocal();
  std::cout << " -> Pgw " << nodeB->GetObject<Ipv4>()->GetAddress(interfaceB,0).GetLocal() << std::endl;
  */
  nodeA->GetObject<Ipv4> ()->SetDown (interfaceA);
  nodeB->GetObject<Ipv4> ()->SetDown (interfaceB);
}

void reconfigureUdpClient(UdpClientHelper srcNode, Ptr<Node> dstNode, uint16_t dport){

  std::cout << "Changing UE 2 Source app destination" << std::endl;
  Ptr<Ipv4> ipv4 = dstNode->GetObject<Ipv4>();
  std::cout << "Got dst node ipv4 object" << std::endl;
  Ipv4Address ip = ipv4->GetAddress(1,0).GetLocal();
  std::cout << "Destination app Address :: " << ip << std::endl;

  srcNode.SetAttribute("RemotePort", UintegerValue(dport));
  std::cout << "Port Set" << std::endl;
  srcNode.SetAttribute("RemoteAddress", AddressValue(ip));
//  udp->SetRemote(ip, dport);

  //Read and check new destination ip and port values here

  std::cout << "Dest ip/port set in udp client app" << "\nPort:" << dport << "\tIp:" << ip << std::endl;
}

void reconfigureUdpClient2(UdpClientHelper srcNode, Ptr<Node> dstNode, uint16_t dport){

  double interface=0;
  if (dport = 6001){
    interface= 1; //Lte
  } else {
    interface=2; //Wifi
  }
  std::cout << "Interface Manager decision----->" << interface << std::endl;
  std::cout << "Changing UE 2 Source app destination according to Interface Manager" << std::endl;
  Ptr<Ipv4> ipv4 = dstNode->GetObject<Ipv4>();
  std::cout << "Got dst node ipv4 object" << std::endl;
  Ipv4Address ip = ipv4->GetAddress(1,0).GetLocal();
  std::cout << "Destination app Address :: " << ip << std::endl;

  srcNode.SetAttribute("RemotePort", UintegerValue(dport));
  std::cout << "Port Set" << std::endl;
  srcNode.SetAttribute("RemoteAddress", AddressValue(ip));
//  udp->SetRemote(ip, dport);

  //Read and check new destination ip and port values here
  std::cout << "Dest ip/port CHANGED in udp client app" << "\tIp:" << ip << "\nPort:" << dport << std::endl;
}

void CalculateThroughput ()
{
//   //for (int f=0; f<TN; f++)
//  //{
  double mbs = ((totalBytesReceived*8.0)/(1000000*1));
//    //mbs[f] = ((totalBytesReceived[f]*8.0)/(1000000*1));
//    //cout<<"size of vector is  "<< totalBytesReceived.size()<< endl;
  std::cout << "Throughput Interface 1=\t" << mbs << "\t" << Simulator::Now ().GetSeconds() << std::endl;
  totalBytesReceived =0;
    //  //rdTrace << Simulator::Now ().GetSeconds() << "\t"<< f << "\t" << mbs[f] <<"\n";
  rdTrace << Simulator::Now ().GetSeconds() << "\t" << mbs <<"\n";
//
//    //}
//   //fill(totalBytesReceived.begin(), totalBytesReceived.end(), 0);

    // double ThroughputSumA = 0;
  //    std::vector <double> throughputA(sources);
  //    for (int i = 0; i < sources; i++)
	//     {
  //      throughputA[i] = ((totalBytesReceived2[i] * 8.0) / 1000000);
  // //      ThroughputSumA+=throughputA[i]+ThroughputSumA;
  //      //std::cout << Simulator::Now ().GetSeconds() << "\t" << i << "\t" << throughputA[i] << "\n";
  //       //rdTrace << Simulator::Now ().GetSeconds() << "\t"<< f << "\t" << mbs[f] <<"\n";
  //      //    rdTrace << Simulator::Now ().GetSeconds() << "\t" << mbs <<"\n";
  //     }
//    std::cout << "Total Throughput:" << TotalThroughput << std::endl;
    std::cout << "\n";
    Simulator::Schedule (Seconds(1.0), &CalculateThroughput);
 }

 void CalculateThroughput2 ()
 {
 //   //for (int f=0; f<TN; f++)
 //  //{
//   double TotalThroughput2=0;
   double mbs2 = ((totalBytesReceived*8.0)/(1000000*1));
 //    //mbs[f] = ((totalBytesReceived[f]*8.0)/(1000000*1));
 //    //cout<<"size of vector is  "<< totalBytesReceived.size()<< endl;
   std::cout << "Throughput Interface 2=\t" << mbs2 << "\t" << Simulator::Now ().GetSeconds() << std::endl;
//  TotalThroughput2+=mbs2+TotalThroughput2;
  totalBytesReceived =0;

   //  //rdTrace << Simulator::Now ().GetSeconds() << "\t"<< f << "\t" << mbs[f] <<"\n";
  rdTrace2 << Simulator::Now ().GetSeconds() << "\t" << mbs2 <<"\n";
  Simulator::Schedule (Seconds (1.0), &CalculateThroughput2);
  }

void ReceivesPacket(std::string context, Ptr <const Packet> p)
 {
	  //char c= context.at(24);
 	  //int index= c - '0';
 	  //totalBytesReceived[index] += p->GetSize();

   		totalBytesReceived += p->GetSize();
   	  std::cout<< "Received : " << totalBytesReceived << std::endl;
}



int main (int argc, char *argv[])
{
  std::cout << "Starting!" << std::endl;
  //interface Manager
  //double throughputA=0;
  //double throughputB=0;

  //int duration = 10; //, schedType = 0
//  uint32_t totalBytesReceived = 0;
  uint16_t numberOfUEs=2; //Default number of UEs attached to each eNodeB
  double duration = 8; //seconds
  //double monitorInterval = 0.100;
//  std::string rate ("1Mbps");
//  std::string phyMode ("ErpOfdmRate54Mbps");
  CommandLine cmd;
  cmd.AddValue ("simulationTime", "Simulation time in seconds", duration);
  cmd.Parse (argc,argv);

//  Config::SetDefault ("ns3::OnOffApplication::PacketSize", StringValue ("1000"));
//  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue (rate));
//  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));
//  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2000"));

void ReceivePacket (Ptr <Socket> socket);
/// Check network throughput
void CheckThroughput ();
void ReceivedPacket (Ptr<const Packet> p, const Address & addr);
void ReceivesPacket (std::string context, Ptr <const Packet> p);
void CalculateThroughput();


  // NODES
  // Each node assum a behavior, ue in Lte networks and stas in wifi networks
  // once we have an ad-hoc network apNodes don't exist
  // condidering that in LTE networks we need one enbNode at list
  //NodeContainer ueNode, enbNode, apNode;
  NodeContainer ueNode, enbNode;
  ueNode.Create (numberOfUEs);
  enbNode.Create (1);
  //apNode.Create (1);
  std::cout << "Node Containers created!" << std::endl;

  //NetDeviceContainer enbDevs, ueDevs, apDevs;
  NetDeviceContainer enbDevs, ueDevs;

  PointToPointHelper p2ph; //Why these kind of communication?
  std::cout << "P2P helper created!" << std::endl;

  // Installing internet stack
  InternetStackHelper internet;
  //internet.Install(apNode);
  internet.Install(ueNode);

  std::cout << "Internet stack installed on Ue devices!" << std::endl;
/*
  // Installing Wifi Stuff on UEs
  std::cout << "Configuring WIFI!" << std::endl;
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
  phy.SetChannel (channel.Create ());

  WifiHelper wifi; //the default standard of 802.11a will be selected by this helper since the program doesn't specify another one
  wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
  WifiMacHelper mac;
  //Ssid ssid = Ssid("network");
  phy.Set ("ChannelNumber", UintegerValue (36));
  mac.SetType("ns3::ApWifiMac", "QosSupported", BooleanValue (true),
  		  "Ssid", SsidValue (ssid),
  		  "EnableBeaconJitter", BooleanValue (false));
  apDevs = wifi.Install (phy, mac, apNode);
  mac.SetType ("ns3::StaWifiMac", "QosSupported", BooleanValue (true),
                 "Ssid", SsidValue (ssid));
  ueDevs = wifi.Install (phy, mac, ueNode);
*/

    // Installing Wifi 2.4 Stuff on UEs
    std::cout << "Configuring ad-hoc network interface: WIFI 2.4GHz!" << std::endl;

    // Channel
    YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11);
    wifiPhy.SetChannel (channel.Create ());
    wifiPhy.Set ("ChannelNumber", UintegerValue (6));

    // Propagation loss models are additive.
    // modelo de perdas
    channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    channel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue (2.4e9));
    channel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel");

    // the frequency
    WifiHelper wifi;
    wifi.SetStandard (WIFI_PHY_STANDARD_80211n_2_4GHZ);
  //  wifi.EnableLogComponents ();  // Turn on all Wifi logging
    wifi.SetRemoteStationManager ("ns3::IdealWifiManager");

    //the MAC
    WifiMacHelper mac;
    mac.SetType ("ns3::AdhocWifiMac");
    ueDevs = wifi.Install (wifiPhy, mac, ueNode);

    // Tracing
    wifiPhy.EnablePcap ("wifi-simple-adhoc", ueDevs);


  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  // Installing LTE interface and network configuration
  std::cout << "Configuring LTE!" << std::endl;
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();     //Define LTE
  Ptr<EpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();    //Define EPC
  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->SetSchedulerType("ns3::RrFfMacScheduler");
  lteHelper->SetAttribute ("PathlossModel",
                           StringValue ("ns3::FriisPropagationLossModel"));
  Ptr<Node> pgw = epcHelper->GetPgwNode (); //Define the Packet Data Network Gateway(P-GW)



  //Define the Remote Host
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  internet.Install (remoteHostContainer);

  //Connect RemoteHost to PGW
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));

  NetDeviceContainer internetDevices;
  internetDevices = p2ph.Install (pgw, remoteHost);

  Packet::EnablePrinting ();


  Ipv4AddressHelper address1;
  address1.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = address1.Assign (internetDevices);      //Ipv4 interfaces
  //Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);
  //std::cout << "Remote host address : " << remoteHostAddr <<std::endl;

  address1.SetBase("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ueInterface;
  ueInterface = address1.Assign(ueDevs);


  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());    //Ipv4 static routing helper
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  std::cout << "LTE+EPC+remotehost installed. Done!" << std::endl;


  /* Mobility stuff */
  MobilityHelper mobility;
  double distance = 5;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  for (uint16_t i = 0; i < numberOfUEs; i++)
    {
	positionAlloc->Add (Vector(distance * i, 0, 0));
    }
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install(enbNode);
  mobility.Install(ueNode);
//  mobility.Install(apNode);
  std::cout << "Mobility installed" << std::endl;

  /* Install Lte devices to the nodes */
  enbDevs = lteHelper->InstallEnbDevice (enbNode);
  ueDevs = lteHelper->InstallUeDevice (ueNode);

  /* install internet stack on ues, wifi nodes */
  Ipv4InterfaceContainer iueIpIface;
  iueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs));
  for (uint32_t u = 0; u < ueNode.GetN (); ++u)
  	{
    Ptr<Node> ueNod = ueNode.Get(u);
    Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNod->GetObject<Ipv4> ());
    ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
  	}
  std::cout << "Internet on Ues installed" << std::endl;


  // Attach all UEs to eNodeB
  for (uint16_t j=0; j < numberOfUEs; j++)
  	{
	lteHelper->Attach (ueDevs.Get(j), enbDevs.Get(0));
  	}
  //std::cout << "test 4" << std::endl;

  Ptr<Node> node = ueNode.Get(0);
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
  Ipv4Address addr = ipv4->GetAddress(0,0).GetLocal();
  std::cout << std::endl << "Ue 1 address 0: " << addr <<std::endl;
  addr = ipv4->GetAddress(1,0).GetLocal();
  std::cout << "Ue 1 address 1: " << addr <<std::endl;
  addr = ipv4->GetAddress(2,0).GetLocal();
  std::cout << "Ue 1 address 2: " << addr <<std::endl;

  node = ueNode.Get(1);
  ipv4 = node->GetObject<Ipv4>();
  addr = ipv4->GetAddress(0,0).GetLocal();
  std::cout << std::endl << "Ue 2 address 0: " << addr <<std::endl;
  addr = ipv4->GetAddress(1,0).GetLocal();
  std::cout << "Ue 2 address 1: " << addr <<std::endl;
  addr = ipv4->GetAddress(2,0).GetLocal();
  std::cout << "Ue 2 address 2: " << addr <<std::endl;

  ipv4 = remoteHost->GetObject<Ipv4>();
  addr = ipv4->GetAddress(0,0).GetLocal();
  std::cout << std::endl << "Remote Host address 0: " << addr <<std::endl;
  addr = ipv4->GetAddress(1,0).GetLocal();
  std::cout << "Remote Host address 1: " << addr <<std::endl;

  ipv4 = pgw->GetObject<Ipv4>();
  addr = ipv4->GetAddress(0,0).GetLocal();
  std::cout << std::endl << "PGW address 0: " << addr <<std::endl;
  addr = ipv4->GetAddress(1,0).GetLocal();
  std::cout << "PGW address 1: " << addr <<std::endl;
  addr = ipv4->GetAddress(2,0).GetLocal();
  std::cout << "PGW address 2: " << addr <<std::endl;
/*
  node = apNode.Get(0);
  ipv4 = node->GetObject<Ipv4>();
  addr = ipv4->GetAddress(1,0).GetLocal();
  std::cout << std::endl << "Ap address 1: " << addr <<std::endl;
*/

  // application stuff
  uint16_t dport = 5001, dport1 = 6001;
  uint32_t payloadSize = 1472; //bytes
  double interPacketInterval = 200;
  ApplicationContainer clientApps, serverApps;

  // Downlink (source) client on Ue1 :: sends data to Ue 0 with LTE
  UdpClientHelper dlClient (iueIpIface.GetAddress (0), dport);
  dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
  dlClient.SetAttribute ("MaxPackets", UintegerValue(100000000));
  dlClient.SetAttribute ("PacketSize", UintegerValue(payloadSize));
  dlClient.SetAttribute("StartTime", TimeValue(MilliSeconds(1000)));
  dlClient.SetAttribute("StopTime", TimeValue(MilliSeconds(10000)));
  clientApps.Add (dlClient.Install (ueNode.Get(1)));
  // Downlink (sink) Sink on Ue 0 :: receives data from Remote Host
//  PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dport));
  //PacketSinkHelper dlPacketSinkHelper1 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dport1));

  UdpServerHelper dlPacketSinkHelper(dport);
  UdpServerHelper dlPacketSinkHelper1(dport1);
  serverApps.Add (dlPacketSinkHelper.Install (ueNode.Get(0)));
  serverApps.Add (dlPacketSinkHelper1.Install (ueNode.Get(0)));

  // Wifi test apps
  uint16_t wdport = 5004;

  // Downlink (source) client on Ue 0 :: sends data to Ue 1 with WIFI
  std::cout << std::endl;
  std::cout << "wifi dest add :: " << ueNode.Get(1)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal() << std::endl;
  std::cout << "wifi src add :: " << ueNode.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal() << std::endl;

  UdpClientHelper wdlClient ((ueNode.Get(1))->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), wdport);
  wdlClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
  wdlClient.SetAttribute ("MaxPackets", UintegerValue(1000000));
  wdlClient.SetAttribute ("PacketSize", UintegerValue(payloadSize));
  clientApps.Add (wdlClient.Install (ueNode.Get(0)));
  // Downlink (sink) Sink on Ue 1 :: receives data from Ue 0
  PacketSinkHelper wdlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), wdport));
  serverApps.Add (wdlPacketSinkHelper.Install (ueNode.Get(1)));



  std::stringstream ST;
  ST<<"/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx";

  //ST<<"/NodeList/"<< 0 <<"/ApplicationList/*/$ns3::PacketSink/Rx";                 //

  Config::Connect (ST.str(), MakeCallback(&ReceivesPacket));

 // Connect MacRx event at MAC layer of sink node to ReceivePacket function for throughput calculation

    //node = ueNode.Get(0);
  //Simulator::Schedule(Seconds(6), &TearDownLink, remoteHost, pgw, 1, 2);
  //Simulator::Schedule (Seconds (1), &CalculateThroughput);
    // uint32_t apNum = 0;

  /*  uint64_t totalPacketsThroughWiFi = DynamicCast<UdpServer> (serverApps.Get (0))->GetReceived ();
    std::cout << "Throughput Int Manager Wifi : " << totalPacketsThroughWiFi <<std::endl;
    uint64_t totalPacketsThroughLte = DynamicCast<UdpServer> (serverApps.Get (0))->GetReceived ();
    std::cout << "Throughput Int Manager Lte : " << totalPacketsThroughLte <<std::endl; */
    Simulator::Schedule(Seconds(1), &CalculateThroughput);
    Simulator::Schedule(Seconds(3), &TearDownLink, ueNode.Get(0), pgw, 2, 2);
    Simulator::Schedule (Seconds (4), &reconfigureUdpClient, dlClient, ueNode.Get(0), dport1);
    Simulator::Schedule(Seconds(4), &CalculateThroughput2);


  //Simulator::Schedule (Seconds (3), &reconfigureUdpClient, wdlClient, ueNode.Get(0), wdport);
  //Simulator::Schedule (Seconds (4), &reconfigureUdpClient2, wdlClient, ueNode.Get(0), wdport);
//  Simulator::Schedule (Seconds (1), &CalculateThroughput, ueNode.GetN());



    //verifying interface manager
    /*if (totalPacketsThroughWifi > totalPacketsThroughLte) {
      //allocate result from interface manager
      Simulator::Schedule (Seconds (4), &reconfigureUdpClient2, wdlClient, ueNode.Get(0), wdport);
      std::cout << "Throughput Int Manager Wifi : " << totalPacketsThroughWifi <<std::endl;
    } else {
      Simulator::Schedule (Seconds (4), &reconfigureUdpClient2, dlClient, ueNode.Get(0), dport1);
      std::cout << "Throughput Int Manager Lte : " << totalPacketsThroughLte <<std::endl;
    } */

//-------------------------------------


  // Flow Monitor
  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;
  flowMonitor = flowHelper.InstallAll();


  NS_LOG_INFO ("Starting simulation.....");
  Simulator::Stop(Seconds(duration + 1));

  AnimationInterface anim ("lte+wifi.xml");


  anim.SetConstantPosition(ueNode.Get(0), 40.0, 100.0);
  anim.SetConstantPosition(ueNode.Get(1), 60.0, 100.0);
//  anim.SetConstantPosition(apNode.Get(0), 50.0, 80.0);
  anim.SetConstantPosition(enbNode.Get(0), 80.0, 80.0);
  anim.SetConstantPosition(pgw, 80.0, 60.0);
  anim.SetConstantPosition(remoteHostContainer.Get(0), 80.0, 40.0);
  anim.EnablePacketMetadata (true);
  lteHelper->EnableTraces ();
  // Uncomment to enable PCAP tracing
  p2ph.EnablePcapAll("lte-wifi-epc");



  Simulator::Run ();
  // trying to get throughput



  /* Show results */
  flowMonitor->CheckForLostPackets();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowHelper.GetClassifier());
  std::map<FlowId, FlowMonitor::FlowStats> stats = flowMonitor->GetFlowStats();

  double ThroughputsumLte = 0;
  double ThroughputsumWiFi = 0;
  double rxDuration;


  for(std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin(); i !=stats.end(); ++i)
  {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
	  if((t.sourceAddress == "7.0.0.3" && t.destinationAddress == "7.0.0.2"))
	  {
		  std::cout << std::endl;
		  std::cout << "Flow : " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
		  std::cout << " Tx bytes : " << i->second.txBytes << "\n";
		  std::cout << " Rx bytes : " << i->second.rxBytes << "\n";
		  std::cout << " First Tx Pkt time : " << i->second.timeFirstTxPacket.GetSeconds() << std::endl;
		  std::cout << " Last Rx Pkt time : " << i->second.timeLastRxPacket.GetSeconds() << std::endl;
		  std::cout << " LTE Throughput : " << i->second.rxBytes *8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024<< " Mbps\n";
      rxDuration = i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstRxPacket.GetSeconds ();
      ThroughputsumLte += (i->second.rxBytes * 8.0 / rxDuration / 1000 / 1000);  //Mbps
      }
	  if((t.sourceAddress == "192.168.1.1" && t.destinationAddress == "192.168.1.2"))
	  {
		  std::cout << std::endl;
		  std::cout << "Flow : " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
	  	std::cout << " Tx bytes : " << i->second.txBytes << "\n";
	  	std::cout << " Rx bytes : " << i->second.rxBytes << "\n";
		  std::cout << " First Tx Pkt time : " << i->second.timeFirstTxPacket.GetSeconds() << std::endl;
		  std::cout << " Last Rx Pkt time : " << i->second.timeLastRxPacket.GetSeconds() << std::endl;
	  	std::cout << " Wifi Sender Throughput : " << i->second.rxBytes *8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024 << " Mbps\n";
      rxDuration = i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstRxPacket.GetSeconds ();
      ThroughputsumWiFi += (i->second.rxBytes * 8.0 / rxDuration / 1000 / 1000);  //Mbps

	  }
	  if((t.sourceAddress == "192.168.1.2" && t.destinationAddress == "192.168.1.1"))
	  {
		  std::cout << std::endl;
		  std::cout << "Flow : " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
		  std::cout << " Tx bytes : " << i->second.txBytes << "\n";
	  	  std::cout << " Rx bytes : " << i->second.rxBytes << "\n";
	  	  std::cout << " First Tx Pkt time : " << i->second.timeFirstTxPacket.GetSeconds() << std::endl;
	  	  std::cout << " Last Rx Pkt time : " << i->second.timeLastRxPacket.GetSeconds() << std::endl;
	  	  std::cout << " Wifi Reception Throughput : " << i->second.rxBytes *8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024 << " Mbps\n";
        }
  }
  flowMonitor->SerializeToXmlFile("lte.xml", true, true);

  std::cout << "Throughput sum WiFi: " << ThroughputsumWiFi << "\n";
  std::cout << "Throughput sum Lte: " << ThroughputsumLte << "\n";
  std::cout << "Avg throughput Wifi: " << ThroughputsumWiFi/numberOfUEs << "\n";
  std::cout << "Avg throughput Lte: " << ThroughputsumLte/numberOfUEs << "\n";

  Simulator::Destroy ();


  NS_LOG_INFO ("Done.");

  return 0;
}
