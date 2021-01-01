/*
 * lte.cc
 *
 *  Created on: Jul 17, 2019
 *      Author: ubuntu
 * Based on anas version https://groups.google.com/forum/#!searchin/ns-3-users/anas%7Csort:date/ns-3-users/ybKYurgIF98/UNlvKrdECwAJ
 * Version: 0.2 - 2020/03/24 by Laura -
 *
 *
 */


/*
To simulate with logs of internetIpIfaces
./waf --run "scratch/lte_wifi_anas" 2>&1 | tee analyse.txt


*/
#include "ns3/core-module.h"
#include "ns3/config-store-module.h"

#include <sstream>
#include <stdint.h>

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
uint32_t totalBytesReceived1=0;
uint32_t totalBytesReceived2=0;
uint32_t totalSumBytesReceived2=0;
uint32_t totalSumLteBytesReceived=0;
uint32_t nNodes=2;
std::vector<double> throughputA (100);
std::ofstream rdTrace;
std::ofstream rdTrace2;
std::vector<uint32_t> lteBytesReceived(100);
//uint32_t lteBytesReceived=0;
//uint32_t ltePacketsReceived=0;


std::vector<double> wifiThroughputPerNode (100);
std::vector<uint32_t> wifiPacketsReceived (100);
std::vector<double> wifiPacketsSent (100);
std::ofstream phyTxTraceFile;
std::ofstream macTxTraceFile;
std::ofstream socketRecvTraceFile;
std::vector<uint32_t> ltePacketsReceived(100);

uint32_t
ContextToNodeId1 (std::string context)
{
  std::string sub = context.substr (10);  // skip "/NodeList/"
  uint32_t pos = sub.find ("/Device");
  NS_LOG_DEBUG ("Found NodeId " << atoi (sub.substr (0, pos).c_str ()));
  return atoi (sub.substr (0,pos).c_str ());
}

uint32_t pktSize = 1500; //1500


/*uint32_t bytesTotal; ///< total bytes received by all nodes
std::string m_CSVfileName= "DsdvManetExample.csv";
uint32_t packetsReceived; ///< total packets received by all nodes
*/


void TearDownLink (Ptr<Node> nodeA, Ptr<Node> nodeB, uint32_t interfaceA, uint32_t interfaceB)
{
  std::cout << "Setting down Remote Host -> Ue 1" << std::endl;

  std::cout << "Remote Host " << nodeA->GetObject<Ipv4>()->GetAddress(interfaceA,0).GetLocal();
  std::cout << " -> Pgw " << nodeB->GetObject<Ipv4>()->GetAddress(interfaceB,0).GetLocal() << std::endl;

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

/////// Extra

void
SocketRecvStats (std::string context, Ptr<const Packet> p, const Address &addr)
{
      totalBytesReceived2 += p->GetSize ();
      std::cout<< "[" << Simulator::Now ().GetSeconds() << "]\t" << "Received_1 : " << totalBytesReceived2 << std::endl;
}

void
SinkRecvStats (std::string context, Ptr<const Packet> p, const Address &addr)
{
  uint32_t nodeId = ContextToNodeId1 (context);
  // Each packet is 1024 bytes UDP payload, but at L2, we need to add 8 + 20 header bytes to support throughput calculations at the LTE layer
  std::cout<< "  print packet size: " << p->GetSize () << std::endl;
//  if (Simulator::Now () >= Seconds (1))
//    {
      lteBytesReceived[nodeId] += (p->GetSize () + 20 + 8);
      ltePacketsReceived[nodeId]++;
//  }
}


///End Extra


void CalculateThroughput ()
{
//   //for (int f=0; f<TN; f++)
//  //{
    double lteThroughput = ((double)(lteBytesReceived[nNodes+3]*8.0))/(1000000*1);


  std::cout << "[" << Simulator::Now ().GetSeconds() << "]\t" << "\tThroughput LTE throughput =" << lteThroughput << "\tQtd bytes Received:" << lteBytesReceived[nNodes+3]<< std::endl;
  totalSumLteBytesReceived =(double)(lteBytesReceived[nNodes+3])+totalSumLteBytesReceived;
  std::cout<< "[" << Simulator::Now ().GetSeconds() << "]\t" << "Total bytes received LTE throughput=\t" << totalSumLteBytesReceived << std::endl;

//  fill(totalBytesReceived2.begin(), totalBytesReceived2.end(), 0);

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
    Simulator::Schedule (MilliSeconds(100), &CalculateThroughput);
 }

 void ReceivesPacket(std::string context, Ptr <const Packet> p)
  {
 	  //char c= context.at(24);
  	  //int index= c - '0';
  	  //totalBytesReceived[index] += p->GetSize();

    		totalBytesReceived += p->GetSize();
    	  std::cout<< "[" << Simulator::Now ().GetSeconds() << "]\t" << "Received : " << totalBytesReceived << std::endl;
 }

 void CalculateThroughput2 (uint32_t nNodes)
 {
 //   //for (int f=0; f<TN; f++)
 //  //{
//   double TotalThroughput2=0;

   double mbs2 = ((totalBytesReceived*8.0)/(1000000*1));
   double avg=0;
   avg+= mbs2 / nNodes;
 //    //mbs[f] = ((totalBytesReceived[f]*8.0)/(1000000*1));
 //    //cout<<"size of vector is  "<< totalBytesReceived.size()<< endl;
   //  TotalThroughput2+=mbs2+TotalThroughput2;
  std::cout<< "[" << Simulator::Now ().GetSeconds() << "]\t" << "Throughput Wifi network  : \t" << mbs2 << "\tQtd bytes Received:" << totalBytesReceived<< "\tAvg Throughput by node:\t" << avg <<  std::endl;
  totalBytesReceived1 =totalBytesReceived+totalBytesReceived1;
  std::cout<< "Sum of total bytes received Wifi up to " <<  "[" << Simulator::Now ().GetSeconds() << "s]\t=" << totalBytesReceived1 << std::endl;
   //  //rdTrace << Simulator::Now ().GetSeconds() << "\t"<< f << "\t" << mbs[f] <<"\n";
  //rdTrace2 << Simulator::Now ().GetSeconds() << "\t" << mbs2 <<"\n"<< "\nTotal bytes Received:" << totalBytesReceived1 << std::endl;
  Simulator::Schedule (MilliSeconds (100), &CalculateThroughput2, nNodes);
}





int main (int argc, char *argv[])
{
  std::cout << "Starting!" << std::endl;

  uint32_t numberOfUEs=2; //Default number of UEs attached to each eNodeB
  uint32_t eNBs=1; //Default number of eNbs in case of LTE interface
  std::string phyMode ("DsssRate1Mbps");
  double duration = 10; //seconds
  //double monitorInterval = 0.100;
   std::string rate ("1Mbps");
//  std::string phyMode ("ErpOfdmRate54Mbps");

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();


  CommandLine cmd;
  cmd.AddValue ("simulationTime", "Simulation time in seconds", duration);
  cmd.Parse (argc,argv);

 Config::SetDefault ("ns3::OnOffApplication::PacketSize", StringValue ("1000"));
 Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue (rate));
 Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));
 Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2000"));

//void ReceivePacket (Ptr <Socket> socket);
;
//void ReceivedPacket (Ptr<const Packet> p, const Address & addr);
//void ReceivesPacket (std::string context, Ptr <const Packet> p);
//void CalculateThroughput();
//void CalculateThroughput2();


  // NODES
  // Each node assum a behavior, ue in Lte networks and stas in wifi networks
  // once we have an ad-hoc network apNodes don't exist
  // condidering that in LTE networks we need one enbNode at list
  //NodeContainer ueNode, enbNode, apNode;
  NodeContainer ueNode, enbNode;
  ueNode.Create (numberOfUEs);
  enbNode.Create (eNBs);
  //apNode.Create (1);
  std::cout << "Node Containers created, for " << numberOfUEs << "nodes clients!" << std::endl;
  std::cout << "Node Containers created, for " << eNBs << "eNBs or ERBs in case of LTE interface!" << std::endl;

  //NetDeviceContainer enbDevs, ueDevs, apDevs;
  NetDeviceContainer enbDevs, ueDevs;

  PointToPointHelper p2ph; //Why these kind of communication?
  std::cout << "P2P helper created!" << std::endl;

  // Installing internet stack
  InternetStackHelper internet;
  //internet.Install(apNode);
  internet.Install(ueNode);

  std::cout << "Internet stack installed on Ue devices!" << std::endl;
/* In case of centralized networks
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

    // Installing Wifi 5.0 GHz Stuff on UEs
    std::cout << "Configuring ad-hoc network interface: WIFI 5 GHz!" << std::endl;

    // Channel
    YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
    wifiPhy.SetChannel (channel.Create ());
    wifiPhy.Set ("ChannelNumber", UintegerValue (36));

    // Propagation loss models are additive.
    // modelo de perdas
    channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    channel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue (5e9));
    //channel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel");

    // the frequency
    WifiHelper wifi;
    wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  //  wifi.EnableLogComponents ();  // Turn on all Wifi logging
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));

    //the MAC
    WifiMacHelper mac;
    mac.SetType ("ns3::AdhocWifiMac");
    ueDevs = wifi.Install (wifiPhy, mac, ueNode);

    // Tracing
    wifiPhy.EnablePcap ("wifi-simple-adhoc", ueDevs);


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

  uint32_t nNodes = ueNode.GetN ();

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



  // Log Wifi packet receptions
  Config::Connect ("/NodeList/*/$ns3::Node/ApplicationList/*/$ns3::PacketSocketServer/Rx", MakeCallback (&SocketRecvStats));

  // Log LTE packet receptions
  std::ostringstream oss;
  oss << "/NodeList/" << nNodes+3 << "/$ns3::Node/ApplicationList/*/$ns3::PacketSink/Rx";
  std::string var = oss.str();
  Config::Connect (var, MakeCallback (&SinkRecvStats));



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
    Simulator::Schedule(Seconds(4), &TearDownLink, ueNode.Get(0), pgw, 2, 2);
    Simulator::Schedule (Seconds (5), &reconfigureUdpClient, wdlClient, ueNode.Get(0), wdport);
    Simulator::Schedule(Seconds(5), &CalculateThroughput2, nNodes);





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

  double lteThroughput = (double)(lteBytesReceived[nNodes+3]* 8) / 1000 / 1000 / 5 ;
  std::cout << "Total LTE throughput " << lteThroughput << std::endl;


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
