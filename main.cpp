#include <iostream>
#include <tins/tins.h>
#include <thread>

using namespace std;
using namespace Tins;

const IPv4Address calcAdress(const IPv4Address & ip)
{
    IPv4Address incommingIp = IPv4Address(ip);
    IPv4Address networkStartIp = IPv4Address("10.0.0.0");
    IPv4Address subnetp = IPv4Address("255.255.240.0");

    uint32_t ipInt = (incommingIp & ~subnetp) | networkStartIp;
    IPv4Address result = IPv4Address(ipInt);
    std::cout << "Destination address: " << incommingIp << std::endl;
    std::cout << "new Destination address: " << result<< std::endl;
    std::cout << "####"<< std::endl;
    return result;
}

bool arpm(const PDU &pdu)
{
    // Retrieve the ARP layer
    const ARP &arp = pdu.rfind_pdu<ARP>();
    std::cout << arp.opcode()<< std::endl;
    if (arp.opcode() == ARP::REQUEST)
    {
        PacketSender sender;
        const NetworkInterface iface("vboxnet0");
        EthernetII req = ARP::make_arp_request(calcAdress(arp.target_ip_addr()),"10.0.3.42","08:00:27:d3:ef:1e");
        sender.send(req, iface);
    }

    return true;
}

bool arpm2(const PDU &pdu)
{
    // Retrieve the ARP layer
    const ARP &arp = pdu.rfind_pdu<ARP>();
    std::cout << arp.opcode()<< std::endl;
    if (arp.opcode() == ARP::REPLY)
    {
        PacketSender sender;
        const NetworkInterface iface("vboxnet1");
        EthernetII rep = ARP::make_arp_reply("172.16.3.42","172.17.0.20","08:00:27:d3:ef:1e","08:00:27:29:f2:55");
        sender.send(rep, iface);
    } else if (arp.target_ip_addr().to_string() == "10.0.3.42")
    {
        PacketSender sender;
        const NetworkInterface iface("vboxnet0");
        EthernetII rep = ARP::make_arp_reply("10.0.0.20","10.0.3.42","08:00:27:29:f2:55","08:00:27:d3:ef:1e");
        sender.send(rep, iface);
    }

    return true;
}

bool doo(PDU &some_pdu)
{
    //PacketWriter writer("before.pcap", PacketWriter::ETH2);
    //writer.write(some_pdu);

    IP &ip = some_pdu.rfind_pdu<IP>();
    ip.dst_addr(calcAdress(ip.dst_addr()));
    ip.src_addr("10.0.3.42");
    //writer = PacketWriter("after.pcap", PacketWriter::ETH2);
    //writer.write(some_pdu);
    PacketSender sender;
    some_pdu.send(sender,"vboxnet0");
    return true;
}



void test1()
{
    SnifferConfiguration config;
    config.set_promisc_mode(true);
    config.set_immediate_mode(true);
    //config.set_filter("ip src 192.168.0.100");
    Sniffer sniffer("vboxnet1", config);
    sniffer.sniff_loop(arpm);
}

void test2()
{
    SnifferConfiguration config;
    config.set_promisc_mode(true);
    config.set_immediate_mode(true);
    //config.set_filter("ip src 192.168.0.100");
    Sniffer sniffer("vboxnet0", config);
    sniffer.sniff_loop(arpm2);
}

void test3()
{
    SnifferConfiguration config;
    config.set_promisc_mode(true);
    config.set_immediate_mode(true);
    //config.set_filter("ip src 192.168.0.100");
    Sniffer sniffer("vboxnet1", config);
    sniffer.sniff_loop(doo);
}

int main()
{
    thread t1(test1);
    thread t2(test2);
    thread t3(test3);
    t1.join();
    t2.join();
    t3.join();
    return 0;
}

