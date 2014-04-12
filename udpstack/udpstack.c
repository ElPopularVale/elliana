#include "udpstack.h"
#include "enc28j60_jose.h"
#include "usart_jose.h"
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
uint8_t deviceIP[4] = { 192, 168, 88, 153 };
// IP address of the router
uint8_t routerIP[4] = { 192, 168, 88, 1 };

uint8_t serverIP[4];

uint8_t dnsIP[4] = { 192, 168, 88, 1 };

char GetPacket(int protocol, uint8_t* packet) {
	uint8_t i = 0;
	uint16_t len;

	if (len = ETH_ReceivePacket(packet, MAXPACKETLEN)) {
		usartSend(0xC9);	//Packet received
		EtherNetII* eth = (EtherNetII*) packet;
//		printStruct(eth,14);
		if (eth->type == (uint16_t) HTONS(IPPACKET)) {
			//We have an IP packet and we need to check protocol.
			IPhdr* ip = (IPhdr*) packet;
			usartSend(0xC4); //IP packet received
			if (ip->protocol == protocol) {
				return 1;
			}
		}
		usartSend(0xC8);

//		if (eth->type == HTONS(ARPPACKET)) {
//			ARP* arpPacket = (ARP*) packet;
//			if (arpPacket->opCode == HTONS(ARPREQUEST)) {
//				//We have an arp and we should reply
//				// ReplyArpPacket(arpPacket);
//				usartSend(0xC3);
//			}
//		} else
//
//			//Reply to any Pings
//			if (ip->protocol == ICMPPROTOCOL) {
//				usartSend(0xC5);
//				PingReply((ICMPhdr*) packet, len);
//			}
//		}

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

		ping->chksum =
				HTONS( ~( chksum(0, ((uint8_t*) ping) + sizeof(IPhdr ),
										len - sizeof(IPhdr) ) ) );
		ping->ip.chksum =
				HTONS( ~( chksum(0, ((uint8_t*) ping) + sizeof(EtherNetII),
										sizeof(IPhdr) - sizeof(EtherNetII) ) ) );
		ETH_send_packet((uint8_t*) ping, len);
	}
}

static uint16_t chksum(uint16_t sum, uint8_t *data,
		uint16_t len) {
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

void printStruct(void *s, uint8_t size){
	int i;
	uint8_t const *ss = s;
	for(i = 0;i<size;i++){
		usartSend(ss[i]);
	}
}
