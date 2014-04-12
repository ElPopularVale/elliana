#include "stdint.h"

#define MAXPACKETLEN 100
// Ethernet packet types
#define ARPPACKET 0x0806
#define IPPACKET 0x0800
//ARP opCodes
#define ARPREPLY  0x0002
#define ARPREQUEST 0x0001
//ARP hardware types
#define ETHERNET 0x0001
// IP protocols
#define ICMPPROTOCOL 0x1
#define UDPPROTOCOL 0x11
#define TCPPROTOCOL 0x6
#define DNSUDPPORT 53
#define DNSQUERY 0
#define DNSREPLY 1
#define ICMPREPLY 0x0
#define ICMPREQUEST 0x8

typedef struct
{
  uint8_t DestAddrs[6];
  uint8_t SrcAddrs[6];
  uint16_t type;
}  EtherNetII;

typedef struct
{
  EtherNetII eth;
  uint8_t hdrlen : 4;
  uint8_t version : 4;
  uint8_t diffsf;
  uint16_t len;
  uint16_t ident;
  uint16_t fragmentOffset1: 5;
  uint16_t flags : 3;
  uint16_t fragmentOffset2 : 8;
  uint8_t ttl;
  uint8_t protocol;
  uint16_t chksum;
  uint8_t source[4];
  uint8_t dest[4];
}IPhdr;

typedef struct
{
  IPhdr ip;
  uint8_t type;
  uint8_t code;
  uint16_t chksum;
  uint16_t iden;
  uint16_t seqNum;
}ICMPhdr;

typedef struct
{
  EtherNetII eth;
  uint16_t hardware;
  uint16_t protocol;
  uint8_t hardwareSize;
  uint8_t protocolSize;
  uint16_t opCode;
  uint8_t senderMAC[6];
  uint8_t senderIP[4];
  uint8_t targetMAC[6];
  uint8_t targetIP[4];
}ARP;

char GetPacket( int protocol, uint8_t* packet );
void PingReply(ICMPhdr* ping, uint16_t len);
static uint16_t chksum(uint16_t sum, uint8_t *data, uint16_t len);
void printStruct(void *s, uint8_t size);


