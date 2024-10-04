
#include "ping.h"
PINGMSGR PingRequest;	 // Variable for Ping Request
PINGMSGR PingReply;	     // Variable for Ping Reply
static uint16_t RandomID = 0x1234; 
static uint16_t RandomSeqNum = 0x4321;
uint8_t ping_reply_received = 0; 
uint8_t req=0;
uint8_t rep=0;



uint8_t ping_auto(uint8_t s, uint8_t *addr)
{
	uint8_t i;
	int32_t len = 0;
	uint8_t cnt=0;
for(i = 0; i<=3;i++){

		switch(getSn_SR(s))
		{
			case SOCK_CLOSED:
				close(s);
				IINCHIP_WRITE(Sn_PROTO(s), IPPROTO_ICMP);              // set ICMP Protocol
				if(socket(s,Sn_MR_IPRAW,3000,0)!=0){       // open the SOCKET with IPRAW mode, if fail then Error
					printf( "\r\n socket %d fail r\n",   (0)) ;
#ifdef PING_DEBUG
					return SOCKET_ERROR;
#endif
				}
		/* Check socket register */
				while(getSn_SR(s)!=SOCK_IPRAW);
					wait_1ms(1000); // wait 1000ms
					wait_1ms(1000); // wait 1000ms
					break;
			case SOCK_IPRAW:
    			ping_request(s, addr);
    			req++;
    			while(1){
    				if ( (len = getSn_RX_RSR(s) ) > 0)
    				{
    					ping_reply(s, addr, len);
    					rep++;
    					break;
    				}
    				else if(cnt > 100)
    				{
    					printf( "Request Time out. \r\n");
    					cnt = 0;
    					break;
    				}
    				else
    				{
    					cnt++;
    					wait_1ms(50); // wait 50ms
    			    }
    	// wait_time for 2 seconds, Break on fail
    			}

    			break;
			default:
				break;

		}
#ifdef PING_DEBUG
		if(req>=3)
	   	{

	  		printf("Ping Request = %d, PING_Reply = %d\r\n",req,rep);

	  		if(rep == req)
	  			return SUCCESS;
	  		else
	  			return REPLY_ERROR;

	  	}
#endif
	}
}


uint8_t ping_count(uint8_t s, uint16_t pCount, uint8_t *addr){

	uint16_t rlen, cnt,i;
	cnt = 0;
	 	 
	for(i=0; i<pCount+1;i++){

	if(i!=0){
		/* Output count number */
			printf( "\r\nNo.%d\r\n",   (i-1));
	}

  	switch(getSn_SR(s))
		{
			case SOCK_CLOSED:
				close(s);  
				                                                  // close the SOCKET
				/* Create Socket */  
				IINCHIP_WRITE(Sn_PROTO(s), IPPROTO_ICMP);              // set ICMP Protocol
				if(socket(s,Sn_MR_IPRAW,3000,0)!=s){       // open the SOCKET with IPRAW mode, if fail then Error
					printf( "\r\n socket %d fail r\n",   (s)) ;
#ifdef PING_DEBUG
					return SOCKET_ERROR;
#endif
				}	
				/* Check socket register */
				while(getSn_SR(s)!=SOCK_IPRAW);

				wait_1ms(1000); // wait 1000ms
				wait_1ms(1000); // wait 1000ms
				break;

			case SOCK_IPRAW:
				 ping_request(s, addr);
				 req++;
				while(1){
						if ( (rlen = getSn_RX_RSR(s) ) > 0){
							ping_reply(s, addr, rlen);
							rep++;
							if (ping_reply_received)  break;
						   
						}

					     /* wait_time for 2 seconds, Break on fail*/
						if ( (cnt > 100) ) {
							printf( "\r\nRequest Time out. \r\n") ;
							cnt = 0;
							break;
						}else { 
							cnt++; 	
							wait_1ms(50); // wait 50ms
						}
				     }

				break;

			default:		
				break;
		
       }
#ifdef PING_DEBUG
  		if(req>=pCount)
  		   	{
  		  		printf("Ping Request = %d, PING_Reply = %d\r\n",req,rep);

  		  		if(rep == req)
  		  			return SUCCESS;
  		  		else
  		  			return REPLY_ERROR;
  		  	}
#endif
   }


}

uint8_t ping_request(uint8_t s, uint8_t *addr){
  uint16_t i;

	//Initailize flag for ping reply
	ping_reply_received = 0;
	/* make header of the ping-request  */
	PingRequest.Type = PING_REQUEST;                   // Ping-Request
	PingRequest.Code = CODE_ZERO;	                   // Always '0'
	PingRequest.ID = htons(RandomID++);	       // set ping-request's ID to random integer value
	PingRequest.SeqNum =htons(RandomSeqNum++);// set ping-request's sequence number to ramdom integer value
	//size = 32;                                 // set Data size

	/* Fill in Data[]  as size of BIF_LEN (Default = 32)*/
  	for(i = 0 ; i < BUF_LEN; i++){	                                
		PingRequest.Data[i] = (i) % 8;		  //'0'~'8' number into ping-request's data 	
	}
	 /* Do checksum of Ping Request */
	PingRequest.CheckSum = 0;		               // value of checksum before calucating checksum of ping-request packet
	PingRequest.CheckSum = htons(checksum((uint8_t*)&PingRequest,sizeof(PingRequest)));  // Calculate checksum
	
     /* sendto ping_request to destination */
	if(sendto(s,(uint8_t *)&PingRequest,sizeof(PingRequest),addr,3000)==0){  // Send Ping-Request to the specified peer.
	  	 printf( "\r\n Fail to send ping-reply packet  r\n") ;					
	}else{
	 	  printf( "Send Ping Request  to Destination (") ;					
          printf( "%d.%d.%d.%d )",   (addr[0]),  (addr[1]),  (addr[2]),  (addr[3])) ;
		  printf( " ID:%x  SeqNum:%x CheckSum:%x\r\n",   htons(PingRequest.ID),  htons(PingRequest.SeqNum),  htons(PingRequest.CheckSum)) ;
	}
	return 0;
} // ping request

uint8_t ping_reply(uint8_t s, uint8_t *addr,  uint16_t rlen){
	 
	 uint16_t tmp_checksum;	
	 uint16_t len;
	 uint16_t i;
	 uint8_t data_buf[128];
	 uint16_t port = 3000;
	 PINGMSGR PingReply;
		/* receive data from a destination */
	  	len = recvfrom(s, (uint8_t *)data_buf,rlen,addr,&port);
			if(data_buf[0] == PING_REPLY) {
				PingReply.Type 		 = data_buf[0];
				PingReply.Code 		 = data_buf[1];
				PingReply.CheckSum   = (data_buf[3]<<8) + data_buf[2];
				PingReply.ID 		 = (data_buf[5]<<8) + data_buf[4];
				PingReply.SeqNum 	 = (data_buf[7]<<8) + data_buf[6];

				for(i=0; i<len-8 ; i++)
				{
					PingReply.Data[i] = data_buf[8+i];
				}
					/* check Checksum of Ping Reply */
				tmp_checksum = ~checksum(&data_buf,len);
				if(tmp_checksum != 0xffff)
					printf("tmp_checksum = %x\r\n",tmp_checksum);
				else{
					/*  Output the Destination IP and the size of the Ping Reply Message*/
				    	printf("Reply from %d.%d.%d.%d  ID:%x SeqNum:%x  :data size %d bytes\r\n",
						  (addr[0]),  (addr[1]),  (addr[2]),  (addr[3]),  htons(PingReply.ID),  htons(PingReply.SeqNum),  (rlen+6) );
				    	printf("\r\n");
				    	/*  SET ping_reply_receiver to '1' and go out the while_loop (waitting for ping reply)*/
					ping_reply_received =1;
				}
			}
			else if(data_buf[0] == PING_REQUEST){
				PingReply.Code 	 = data_buf[1];
				PingReply.Type 	 = data_buf[2];
				PingReply.CheckSum  = (data_buf[3]<<8) + data_buf[2];
				PingReply.ID 		 = (data_buf[5]<<8) + data_buf[4];
				PingReply.SeqNum 	 = (data_buf[7]<<8) + data_buf[6];

				for(i=0; i<len-8 ; i++)
				{
					PingReply.Data[i] = data_buf[8+i];
				}
					/* check Checksum of Ping Reply */
				tmp_checksum = PingReply.CheckSum;
				PingReply.CheckSum = 0;
				PingReply.CheckSum = htons(checksum(&PingReply,len));

						if(tmp_checksum != PingReply.CheckSum){
							printf( " \n CheckSum is in correct %x shold be %x \n",   (tmp_checksum),  htons(PingReply.CheckSum)) ;
						}else{
							//printf( "\r\n Checksum is correct  \r\n") ;					
						}
		
					/*  Output the Destination IP and the size of the Ping Reply Message*/
				    	printf("Request from %d.%d.%d.%d  ID:%x SeqNum:%x  :data size %d bytes\r\n",
						  (addr[0]),  (addr[1]),  (addr[2]),  (addr[3]),  (PingReply.ID),  (PingReply.SeqNum),  (rlen+6) );
					/*  SET ping_reply_receiver to '1' and go out the while_loop (waitting for ping reply)*/		   
					ping_reply_received =1;

			}
			else{      
 					 printf(" Unkonwn msg. \n");
			}


			return 0;
}// ping_reply



uint16_t checksum(uint8_t * data_buf, uint16_t len)

{
  uint16_t sum, tsum, i, j;
  uint32_t lsum;

  j = len >> 1;
  lsum = 0;
  tsum = 0;
  for (i = 0; i < j; i++)
    {
      tsum = data_buf[i * 2];
      tsum = tsum << 8;
      tsum += data_buf[i * 2 + 1];
      lsum += tsum;
    }
   if (len % 2)
    {
      tsum = data_buf[i * 2];
      lsum += (tsum << 8);
    }
    sum = (uint16_t)lsum;
    sum = ~(sum + (lsum >> 16));
  return sum;

}


uint16_t htons( uint16_t hostshort)
{
#if 1
  //#ifdef LITTLE_ENDIAN
	uint16_t netshort=0;
	netshort = (hostshort & 0xFF) << 8;

	netshort |= ((hostshort >> 8)& 0xFF);
	return netshort;
#else
	return hostshort;
#endif
}


/*****************************************************************************************
	Function name: wait_1us
	Input		:	cnt; Delay duration = cnt * 1u seconds
	Output	:	non
	Description
	: A delay function for waiting cnt*1u second.
*****************************************************************************************/
void wait_1us(unsigned int cnt)
{
	unsigned int i;

	for(i = 0; i<cnt; i++) {

		}
}


/*****************************************************************************************
	Function name: wait_1ms
	Input		:	cnt; Delay duration = cnt * 1m seconds
	Output	:	non
	Description
	: A delay function for waiting cnt*1m second. This function use wait_1us but the wait_1us
		has some error (not accurate). So if you want exact time delay, please use the Timer.
*****************************************************************************************/
void wait_1ms(unsigned int cnt)
{
	unsigned int i;
	for (i = 0; i < cnt; i++) wait_1us(1000);
}

/*****************************************************************************************
	Function name: wait_10ms
	Input		:	cnt; Delay duration = cnt * 10m seconds
	Output	:	non
	Description
	: A delay function for waiting cnt*10m second. This function use wait_1ms but the wait_1ms
		has some error (not accurate more than wait_1us). So if you want exact time delay,
		please use the Timer.
*****************************************************************************************/
void wait_10ms(unsigned int cnt)
{
	unsigned int i;
	for (i = 0; i < cnt; i++) wait_1ms(10);
}

