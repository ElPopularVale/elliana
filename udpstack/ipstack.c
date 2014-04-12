#include "ipstack.h"
//#include "enc28j60.h"
#include "usart_jose.h"
#include <string.h>
// Switch to host order for the enc28j60
#define HTONS(x) ((x<<8)|(x>>8))

// MAC address of the enc28j60
const uint8_t deviceMAC[6] = {0x00,0xa0,0xc9,0x14,0xc8,0x00};
// Router MAC is not known at beginning and requires an ARP reply to know.
uint8_t routerMAC[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
// IP address of the enc28j60
uint8_t deviceIP[4] = {192,168,0,103};
// IP address of the router
uint8_t routerIP[4] = {192,168,0,1};

uint8_t serverIP[4];

uint8_t dnsIP[4] = {192,168,0,1};


void
add32(uint8_t *op32, uint16_t op16)
{
  op32[3] += (op16 & 0xff);
  op32[2] += (op16 >> 8);
  
  if(op32[2] < (op16 >> 8)) {
    ++op32[1];
    if(op32[1] == 0) {
      ++op32[0];
    }
  }
  
  
  if(op32[3] < (op16 & 0xff)) {
    ++op32[2];
    if(op32[2] == 0) {
      ++op32[1];
      if(op32[1] == 0) {
	++op32[0];
      }
    }
  }
}

static uint16_t
chksum(uint16_t sum, uint8_t *data, uint16_t len)
{
  uint16_t t;
  const uint8_t *dataptr;
  const uint8_t *last_byte;

  dataptr = data;
  last_byte = data + len - 1;
  
  while(dataptr < last_byte) {	/* At least two more bytes */
    t = (dataptr[0] << 8) + dataptr[1];
    sum += t;
    if(sum < t) {
      sum++;		/* carry */
    }
    dataptr += 2;
  }
  
  if(dataptr == last_byte) {
    t = (dataptr[0] << 8) + 0;
    sum += t;
    if(sum < t) {
      sum++;		/* carry */
    }
  }

  /* Return sum in host byte order. */
  return sum;
}

void SetupBasicIPPacket( uint8_t* packet, uint8_t protocol, uint8_t* destIP )
{
  IPhdr* ip = (IPhdr*)packet;
  
  ip->eth.type =  (uint16_t) HTONS(IPPACKET);
  memcpy( ip->eth.DestAddrs, routerMAC, sizeof(routerMAC) );
  memcpy( ip->eth.SrcAddrs, deviceMAC, sizeof(deviceMAC) );
  
  ip->version = 0x4;
  ip->hdrlen = 0x5;
  ip->diffsf = 0;
  ip->ident = 2; //Chosen at random roll of dice
  ip->flags = 0x2;
  ip->fragmentOffset1 = 0x0;
  ip->fragmentOffset2 = 0x0;
  ip->ttl = 128;
  ip->protocol = protocol;
  ip->chksum = 0x0;
  memcpy( ip->source , deviceIP, sizeof(deviceIP) );
  memcpy( ip->dest, destIP, sizeof(deviceIP) );
}

void SendArpPacket(uint8_t* targetIP)
{
  ARP arpPacket;
  
  /*----Setup EtherNetII Header----*/
  //The source of the packet will be the device mac address.
  memcpy( arpPacket.eth.SrcAddrs, deviceMAC, sizeof(deviceMAC) );
  //The destination is broadcast ie all bits are 0xff.
  memset( arpPacket.eth.DestAddrs, 0xff, sizeof(deviceMAC) );
  //The type of packet being sent is an ARP
  arpPacket.eth.type =  (uint16_t) HTONS(ARPPACKET);
  
  /*----Setup ARP Header----*/
  arpPacket.hardware = (uint16_t) HTONS(ETHERNET);
  //We are wanting IP address resolved.
  arpPacket.protocol =  (uint16_t) HTONS(IPPACKET);
  arpPacket.hardwareSize = sizeof(deviceMAC);
  arpPacket.protocolSize = sizeof(deviceIP);
  arpPacket.opCode =  (uint16_t) HTONS(ARPREQUEST);
  
  //Target MAC is set to 0 as it is unknown.
  memset( arpPacket.targetMAC, 0, sizeof(deviceMAC) );
  //Sender MAC is the device MAC address.
  memcpy( arpPacket.senderMAC, deviceMAC, sizeof(deviceMAC) );
  //The target IP is the IP address we want resolved.
  memcpy( arpPacket.targetIP, targetIP, sizeof(routerIP));
  
  //If we are just making sure this IP address is not in use fill in the 
  //sender IP address as 0. Otherwise use the device IP address.
  if ( !memcmp( targetIP, deviceIP, sizeof(deviceIP) ) )
  { 
    memset( arpPacket.senderIP, 0, sizeof(deviceIP) );
  }
  else
  {
    memcpy( arpPacket.senderIP, deviceIP, sizeof(deviceIP) );
  }
  
  //Send out the packet.
  ETH_send_packet((uint8_t*)&arpPacket,sizeof(ARP));
}

void ReplyArpPacket(ARP* arpPacket)
{
  //To reply we want to make sure the IP address is for us first
  if( !memcmp( arpPacket->targetIP, deviceIP, sizeof(deviceIP) ) )
  {
    //The arp is for us so swap the src and dest addrs
    memcpy( arpPacket->eth.DestAddrs, arpPacket->eth.SrcAddrs, sizeof(deviceMAC) );
    memcpy( arpPacket->eth.SrcAddrs, deviceMAC, sizeof(deviceMAC) );
    //Swap the target and sender MACs
    memcpy( arpPacket->targetMAC, arpPacket->senderMAC, sizeof(deviceMAC) );
    memcpy( arpPacket->senderMAC, deviceMAC, sizeof(deviceMAC) );
    //Swap the target and sender IPs
    memcpy( arpPacket->targetIP, arpPacket->senderIP, sizeof(deviceIP) );
    memcpy( arpPacket->senderIP, deviceIP, sizeof(deviceIP));
    //Change the opCode to reply
    arpPacket->opCode = (uint16_t) HTONS(ARPREPLY);
    
    ETH_send_packet((uint8_t*) arpPacket, sizeof(ARP));
  }
}

uint16_t ackTcp(TCPhdr* tcp, uint16_t len)
{
  //Zero the checksums
  tcp->chksum = 0x0;
  tcp->ip.chksum = 0x0;
  // First sort out the destination mac and source
  memcpy( tcp->ip.eth.DestAddrs, tcp->ip.eth.SrcAddrs, sizeof(deviceMAC) );
  memcpy( tcp->ip.eth.SrcAddrs, deviceMAC, sizeof(deviceMAC) );
  // Next flip the ips
  memcpy( tcp->ip.dest, tcp->ip.source, sizeof(deviceIP) );
  memcpy( tcp->ip.source, deviceIP, sizeof(deviceIP) );
  // Next flip ports
  uint16_t destPort = tcp->destPort;
  tcp->destPort = tcp->sourcePort;
  tcp->sourcePort = destPort;
  // Swap ack and seq
  uint8_t ack[4];
  memcpy( ack, tcp->ackNo, sizeof(ack) );
  memcpy( tcp->ackNo, tcp->seqNo, sizeof(ack) );
  memcpy( tcp->seqNo, ack, sizeof(ack) );
  
  if( tcp->SYN )
  {
    add32( tcp->ackNo, 1 );
  }
  else
  {
    add32( tcp->ackNo, len - sizeof(TCPhdr) );
  }
  tcp->SYN = 0;
  tcp->ACK = 1;
  tcp->hdrLen = (sizeof(TCPhdr)-sizeof(IPhdr))/4;
  len = sizeof(TCPhdr);
  tcp->ip.len =  (uint16_t) HTONS((len-sizeof(EtherNetII)));
  
  uint16_t pseudochksum = chksum(TCPPROTOCOL+len-sizeof(IPhdr),
                            tcp->ip.source, sizeof(deviceIP)*2);
  tcp->chksum =  (uint16_t) HTONS(~(chksum(pseudochksum,
                               ((uint8_t*)tcp) + sizeof(IPhdr),
                               len-sizeof(IPhdr))));
  
  tcp->ip.chksum =  (uint16_t) HTONS(~(chksum(0,((uint8_t*)tcp) + sizeof(EtherNetII),
                                  sizeof(IPhdr)-sizeof(EtherNetII))));
  return len;
}

void SendPing( uint8_t* targetIP )
{
  ICMPhdr ping;
  SetupBasicIPPacket( (uint8_t*)&ping, ICMPPROTOCOL, targetIP );
  
  ping.ip.flags = 0x0;
  ping.type = 0x8;
  ping.code = 0x0;
  ping.chksum = 0x0;
  ping.iden = (uint16_t) HTONS(0x1);
  ping.seqNum = (uint16_t) HTONS(76);

  ping.chksum = (uint16_t) HTONS( ~( chksum(0, ((uint8_t*)&ping) + sizeof(IPhdr ),
                                    sizeof(ICMPhdr) - sizeof(IPhdr) ) ) );
  ping.ip.len = (uint16_t) HTONS((60-sizeof(EtherNetII)));
  ping.ip.chksum = (uint16_t) HTONS( ~( chksum( 0, (uint8_t*)&ping + sizeof(EtherNetII),
                                  sizeof(IPhdr) - sizeof(EtherNetII))));
  
  ETH_send_packet( &ping, sizeof(ICMPhdr) );
}

void PingReply(ICMPhdr* ping, uint16_t len)
{
  if ( ping->type == ICMPREQUEST )
  {

    ping->type = ICMPREPLY;
    ping->chksum = 0x0;
    ping->ip.chksum = 0x0;
    //Swap the destination MAC address'
    memcpy( ping->ip.eth.DestAddrs, ping->ip.eth.SrcAddrs, sizeof(deviceMAC) );
    memcpy( ping->ip.eth.SrcAddrs, deviceMAC, sizeof(deviceMAC) );
    //Swap the destination IP address'
    memcpy( ping->ip.dest, ping->ip.source, sizeof(deviceIP) );
    memcpy( ping->ip.source, deviceIP, sizeof(deviceIP) );
    
    ping->chksum = (uint16_t) HTONS( ~( chksum(0, ((uint8_t*) ping) + sizeof(IPhdr ),
                                    len - sizeof(IPhdr) ) ) );
    ping->ip.chksum = (uint16_t) HTONS( ~( chksum(0, ((uint8_t*) ping) + sizeof(EtherNetII),
                                       sizeof(IPhdr) - sizeof(EtherNetII) ) ) );
    usartSend(0xC3);
    usartSend(len);
//    printStruct(ping->ip.eth.DestAddrs, 14);
    ETH_send_packet(ping, len);
  }
}

uint8_t GetPacket( uint8_t protocol, uint8_t* packet )
{
	uint8_t i;
  for(i = 0; i < 255; ++i)//Should make this return 0 after so many failed packets
  {
    uint16_t len;
    if ( len = ETH_ReceivePacket( packet, MAXPACKETLEN ) )
    {
      EtherNetII* eth = (EtherNetII*)packet;
      if ( eth->type == (uint16_t) HTONS(ARPPACKET) )
      {
        ARP* arpPacket = (ARP*)packet;
        if ( arpPacket->opCode == (uint16_t) HTONS(ARPREQUEST))
          //We have an arp and we should reply
          ReplyArpPacket(arpPacket);
      }
      else if( eth->type == (uint16_t) HTONS(IPPACKET) )
      {

        //We have an IP packet and we need to check protocol.
        IPhdr* ip = (IPhdr*)packet;
        if( ip->protocol == protocol )
        {
          return 1;
        }
        
        //Reply to any Pings
        if( ip->protocol == ICMPPROTOCOL )
        {
          PingReply((ICMPhdr*)packet, len);
        }
      }
    }
  }
  return 0;
}
  
uint16_t IPstackInit()
{
//  initMAC( deviceMAC );
  ETH_MAC_init(deviceMAC);
  // Announce we are here
  SendArpPacket( deviceIP );
  // Just waste a bit of time confirming no one has our IP
  uint16_t i;
  for(i = 0; i < 0x0fff; i++)
  {
    ARP arpPacket;
    if( ETH_ReceivePacket( (uint8_t*) &arpPacket, sizeof(ARP) ) )
    {
      if( arpPacket.eth.type == (uint16_t) HTONS(ARPPACKET) )
      {
        if( !memcmp( arpPacket.senderIP, deviceIP, sizeof(deviceIP) ) )
        {
          // Uh oh this IP address is already in use
          return 0;
        }
      }
    }
  }
  // Well no one replied so its safe to assume IP address is OK
  // Now we need to get the routers MAC address
  SendArpPacket( routerIP );
  for(i = 0; i < 0x5fff; i++)
  {
    ARP arpPacket;
    if( ETH_ReceivePacket( (uint8_t*) &arpPacket, sizeof(ARP) ) )
    {
      if( arpPacket.eth.type == (uint16_t) HTONS(ARPPACKET) )
      {
        if( arpPacket.opCode == (uint16_t) HTONS(ARPREPLY) && !memcmp( arpPacket.senderIP, routerIP, sizeof(routerIP) ) )
        {
          // Should be the routers reply so copy the mac address
          memcpy( routerMAC, arpPacket.senderMAC, sizeof(routerMAC) );
          return 1;
        }
      }
    }
    // Every now and then send out another ARP
    if( !(i % 0x1000) )
    {
      SendArpPacket( routerIP );
    }
  }
  return 0;
}


void DNSLookup( const uint8_t* url )
{
  uint8_t packet[MAXPACKETLEN];
  DNShdr* dns = (DNShdr*)packet;
  SetupBasicIPPacket(packet, UDPPROTOCOL, dnsIP);
  
  dns->udp.sourcePort = (uint16_t) HTONS(0xDC6F);//Place a number in here
  dns->udp.destPort = (uint16_t) HTONS(DNSUDPPORT);
  dns->udp.len = 0;
  dns->udp.chksum = 0;
  
  dns->id = (uint16_t) HTONS(0xfae3); //Chosen at random roll of dice
  dns->flags = (uint16_t) HTONS(0x0100);
  dns->qdCount = (uint16_t) HTONS(1);
  dns->anCount = 0;
  dns->nsCount = 0;
  dns->arCount = 0;
  //Add in question header
  uint8_t* dnsq = (uint8_t*)(packet + sizeof(DNShdr) + 1);//Note the +1
  uint16_t noChars = 0;
  const uint8_t* c;
  for(c = url; *c != '\0' && *c !='\\'; ++c, ++dnsq)
  {
    *dnsq = *c;
    if ( *c == '.' )
    {
      *(dnsq-(noChars+1)) = noChars;
      noChars = 0;
    }
    else ++noChars;
  }
  *(dnsq-(noChars+1)) = noChars;
  *dnsq++ = 0;
  //Next finish off the question with the host type and class
  *dnsq++ = 0;
  *dnsq++ = 1;
  *dnsq++ = 0;
  *dnsq++ = 1;
  //uint8_t lenOfQuery = (uint8_t*)dnsq-&uip_buf[sizeof(DNShdr)];
  uint16_t len = (uint8_t*)dnsq-packet;
  //Calculate all lengths
  dns->udp.len = (uint16_t) HTONS((len-sizeof(IPhdr)));
  dns->udp.ip.len = (uint16_t) HTONS((len-sizeof(EtherNetII)));
  //Calculate all checksums
  uint16_t pseudochksum = (uint16_t) chksum(UDPPROTOCOL+len-sizeof(IPhdr),
                            dns->udp.ip.source, sizeof(deviceIP)*2);
  dns->udp.chksum = (uint16_t) HTONS(~(chksum(pseudochksum,
                                   packet+sizeof(IPhdr),
                                   len-sizeof(IPhdr))));
  dns->udp.ip.chksum = (uint16_t) HTONS(~(chksum(0,
                                      packet+sizeof(EtherNetII),
                                      sizeof(IPhdr)-sizeof(EtherNetII))));
  
  
  ETH_send_packet(packet,len);
  while(1)
  {
    GetPacket(UDPPROTOCOL, packet);
    if( ((UDPhdr*)packet)->sourcePort == (uint16_t) HTONS(DNSUDPPORT))
    {
      dns = (DNShdr*)packet;
      if ( dns->id == (uint16_t) HTONS(0xfae3)) //See above for reason
      {
        //IP address is after original query so we should go to the end plus lenOfQuery
        //There are also 12 bytes of other data we do not need to know
        //Grab IP from message and copy into serverIP
        //The address should be the last 4 bytes of the buffer. This is not fool proof and
        //should be changed in the future
        memcpy( serverIP, packet+len-4, sizeof(serverIP));
        return;
      }
    }
  }
}

uint16_t IPstackHTMLPost( const uint8_t* url, const char * data, uint8_t* reply)
{
  //First we need to do some DNS looking up
  DNSLookup(url); //Fills in serverIP
  //Now that we have the IP we can connect to the server
  uint8_t packet[MAXPACKETLEN];
  
  memset(packet,0,sizeof(TCPhdr));//zero the memory as we need all flags = 0
  
  //Syn server
  SetupBasicIPPacket( packet, TCPPROTOCOL, serverIP );
  TCPhdr* tcp = (TCPhdr*)packet;
  tcp->sourcePort = (uint16_t) HTONS(0xe2d7);
  tcp->destPort = (uint16_t) HTONS(80);
  //Seq No and Ack No are zero
  tcp->seqNo[0] = 1;
  tcp->hdrLen = (sizeof(TCPhdr)-sizeof(IPhdr))/4;
  tcp->SYN = 1;
  tcp->wndSize = (uint16_t) HTONS(200);
  //Add in some basic options
  uint8_t opts[] = { 0x02, 0x04,0x05,0xb4,0x01,0x01,0x04,0x02};
  //memcpy(&tcp->options[0],&opts[0],8);
  uint16_t len = sizeof(TCPhdr);
  tcp->ip.len = (uint16_t) HTONS((sizeof(TCPhdr)-sizeof(EtherNetII)));
  uint16_t pseudochksum = chksum(TCPPROTOCOL+len-sizeof(IPhdr), tcp->ip.source, 8);
  tcp->chksum = (uint16_t) HTONS(~(chksum(pseudochksum,packet + sizeof(IPhdr), len-sizeof(IPhdr))));
  tcp->ip.chksum = (uint16_t) HTONS(~(chksum(0, packet+sizeof(EtherNetII), sizeof(IPhdr)-sizeof(EtherNetII))));
  
  ETH_send_packet(packet,len);
  //ack syn/ack
  do{
    GetPacket(TCPPROTOCOL, packet);
  }while(!(tcp->destPort==(uint16_t) HTONS(0xe2d7)));
  ackTcp( tcp, len );
  ETH_send_packet( packet, len );
  
  //Send the actual data
  //first we need change to a push
  tcp->PSH = 1;
  tcp->ip.ident = 0xDEED;
  //Now copy in the data
  uint16_t datalen = strlen(data);
  memcpy( packet + sizeof(TCPhdr), data, datalen);
  len = sizeof(TCPhdr) + datalen;
  tcp->ip.len = (uint16_t) HTONS((len-sizeof(EtherNetII)));
  tcp->chksum = 0;
  tcp->ip.chksum = 0;
  pseudochksum = chksum(TCPPROTOCOL+len-sizeof(IPhdr),tcp->ip.source,sizeof(deviceIP)*2);
  tcp->chksum = (uint16_t) HTONS(~(chksum(pseudochksum, packet + sizeof(IPhdr),len-sizeof(IPhdr))));
  tcp->ip.chksum = (uint16_t) HTONS(~(chksum(0, packet + sizeof(EtherNetII), sizeof(IPhdr)-sizeof(EtherNetII))));
  ETH_send_packet(packet, len);
  
  do{
    GetPacket(TCPPROTOCOL, packet);
  }while(!(tcp->destPort==(uint16_t) HTONS(0xe2d7)));
  memcpy( reply, packet + len -7, 7);
  ackTcp(tcp, len);
  ETH_send_packet( packet, len);
  
  tcp->PSH=0;
  tcp->FIN=1;
  tcp->ACK=1;
  tcp->chksum = 0;
  tcp->ip.chksum = 0;
  pseudochksum = chksum(TCPPROTOCOL+len-sizeof(IPhdr), tcp->ip.source, sizeof(deviceIP)*2);
  tcp->chksum = (uint16_t) HTONS(~(chksum(pseudochksum,packet + sizeof(IPhdr), len-sizeof(IPhdr))));
  tcp->ip.chksum = (uint16_t) HTONS(~(chksum(0, packet + sizeof(EtherNetII), sizeof(IPhdr)-sizeof(EtherNetII))));
  ETH_send_packet(packet,len);
  //Now we need to copy the payload to the reply buffer and ack the reply
  return 0;
}

uint16_t IPstackIdle()
{
  uint8_t packet[MAXPACKETLEN];
  GetPacket(0,packet);
  return 1;
}

void printStruct(void *s, uint8_t size){
	int i;
	uint8_t const *ss = s;
	for(i = 0;i<size;i++){
		usartSend(ss[i]);
	}
}
