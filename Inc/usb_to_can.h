#include "usbd_cdc_if.h"

#define STATE_START_BYTE   		1
#define STATE_DATA_TRANSMIT 	2
#define STATE_CRC_BYTE     	 	3
#define STATE_FINISH      		4
#define STATE_WAIT_BYTE     	5
#define STATE_SEND_MESG			6
#define	STATE_SEND_START_BYTE 	7
#define STATE_SEND_CRC 			8

const uint8_t ok_const    	=0xDD;
const uint8_t repeat_const	=0xDB;
const uint8_t en_byte		=0xB0;

uint8_t  	 len;
uint32_t  	 crc_byte  	 = 	0;
uint8_t  	 state_count =  1;
uint8_t 	 data[18];


void recv_mesg (uint8_t* Buf)
{
	static uint32_t counter;
		switch (state_count)
		  {
				case STATE_START_BYTE:
					{
						if (((*Buf & 0b11110000) >> 4) == 0xB)
							{
								len = (*Buf & 0b00001111);
								//data[0] = 0xDD;
								crc_byte = 0;
								counter = 0;
								state_count = STATE_DATA_TRANSMIT;
								//CDC_Transmit_HS(&ok_const, 1);
							}
						else  CDC_Transmit_HS(&repeat_const, 1);
					}
				break;

				case STATE_DATA_TRANSMIT:
					  {
						  counter++;
						  data[counter] = *Buf;
						  crc_byte = crc_byte ^ *Buf;

    					  if (counter==len)
    						  state_count = STATE_CRC_BYTE;
					  }
				break;

				case STATE_CRC_BYTE  :
							  {
								 if (crc_byte == *Buf)
									 {
									 	 CDC_Transmit_HS(&ok_const, 1);
									 	 state_count = STATE_FINISH;
									 }
								 else if (crc_byte != *Buf)
									 {
									  	 CDC_Transmit_HS(&repeat_const, 1);
									  	 state_count = STATE_START_BYTE;
									 }
							  }
				break;
		  }
}

void send_mesg (uint8_t* data, uint8_t* Buf)
{
	switch (state_count)
		{
			case STATE_SEND_START_BYTE:
				{
					state_count = STATE_SEND_MESG;
					len = (data[0] & 0b00001111);
					CDC_Transmit_HS(&data[0], 1);
				}
			break;

			case STATE_SEND_MESG:
				{
					if (*Buf == 0xDD)
					{
						for (int i = 1; i <len ; i++ )
							 {
							crc_byte = crc_byte ^ data[i];
							CDC_Transmit_HS(&data[i], 1);
							 }
					}
				}
			break;

			case STATE_SEND_CRC:
				{
					state_count = STATE_START_BYTE;
					CDC_Transmit_HS(&crc_byte, 1);
				}
			break;
		}
}
