#include "udpstack.h"
#include "enc28j60_jose.h"
#include "usart_jose.h"
#include "stm32f0_spi_jose.h"
#include <string.h>
// Switch to host order for the enc28j60
#define HTONS(x) ((x<<8)|(x>>8))
#define HIGH_BYTE(X)  (uint8_t)((X) >> 8)
#define LOW_BYTE(X)   (uint8_t)(X & 0xFF)

// MAC address of the enc28j60
const uint8_t deviceMAC[6] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
// Router MAC is not known at beginning and requires an ARP reply to know.
uint8_t routerMAC[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
// IP address of the enc28j60
uint8_t deviceIP[4] = { 192, 168, 0, 111 };
// IP address of the router
uint8_t routerIP[4] = { 192, 168, 0, 1 };

uint8_t serverIP[4];

uint8_t dnsIP[4] = { 192, 168, 0, 1 };

char GetPacket(int protocol, uint8_t* packet) {
	uint8_t i = 0;
	uint16_t len;


	if (len = ETH_ReceivePacket(packet, MAXPACKETLEN)) {
		EtherNetII* eth = (EtherNetII*) packet;
		if (eth->type == (uint16_t) HTONS(ARPPACKET)) {
					ARP* arpPacket = (ARP*) packet;
					if (arpPacket->opCode == (uint16_t) HTONS(ARPREQUEST)) {
						//We have an arp and we should reply
						ReplyArpPacket(arpPacket);
						return 1;

					}

				}
		else if (eth->type == (uint16_t) HTONS(IPPACKET)) {
			//We have an IP packet and we need to check protocol.
			IPhdr* ip = (IPhdr*) packet;
//			if (ip->protocol == protocol) {
//				return 1;
//			}

			//Reply to any Pings
			if (ip->protocol == ICMPPROTOCOL) {
				PingReply((ICMPhdr*) packet, len);
				return 1;
			}

	}
}
	return 0;
}

void PingReply(ICMPhdr* ping, uint16_t len) {
	if (ping->type == ICMPREQUEST) {
		ping->type = ICMPREPLY;
		ping->chksum = 0x0;
		ping->ip.chksum = 0x0;
		//Swap the destination MAC address'
		memcpy(ping->ip.eth.DestAddrs, ping->ip.eth.SrcAddrs,
				sizeof(deviceMAC));
		memcpy(ping->ip.eth.SrcAddrs, deviceMAC, sizeof(deviceMAC));
		//Swap the destination IP address'
		memcpy(ping->ip.dest, ping->ip.source, sizeof(deviceIP));
		memcpy(ping->ip.source, deviceIP, sizeof(deviceIP));

		uint16_t chk1,chk2,chk3,chk4;

		chk1=chksum(0, ((uint8_t*) ping) + sizeof(IPhdr ),len - sizeof(IPhdr) );
		chk2 = ~chk1;
		ping->chksum =	(uint16_t) HTONS(chk2);

		chk3=chksum(0, ((uint8_t*) ping) + sizeof(EtherNetII),sizeof(IPhdr) - sizeof(EtherNetII) );
		chk4 = ~chk3;
		ping->ip.chksum =(uint16_t) HTONS(chk4);
		ETH_send_packet((uint8_t*) ping, len);
	}
}

uint16_t chksum(uint16_t sum, uint8_t *data,	uint16_t len) {
	uint16_t t;
	const uint8_t *dataptr;
	const uint8_t *last_byte;

	dataptr = data;
	last_byte = data + len - 1;

	while (dataptr < last_byte) { /* At least two more bytes */
		t = (dataptr[0] << 8) + dataptr[1];
		sum += t;
		if (sum < t) {
			sum++; /* carry */
		}
		dataptr += 2;
	}

	if (dataptr == last_byte) {
		t = (dataptr[0] << 8) + 0;
		sum += t;
		if (sum < t) {
			sum++; /* carry */
		}
	}

	/* Return sum in host byte order. */
	return sum;
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
	  usartSend(0xC3);
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
  }
    return 0;
  }

void printStruct(void *s, uint8_t size){
	int i;
	uint8_t const *ss = s;
	for(i = 0;i<size;i++){
		usartSend(ss[i]);
	}
}
