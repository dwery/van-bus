#include "stdafx.h"
#include "stdio.h"
#include "van_parser.h"


char deb[100];

uint8 GetBCD(uint8 bcd)
{
	uint8 ret;

	ret = ((bcd>>4)&0x0F)*10 + (bcd&0x0F);
	
	return ret;
}

/*
uint8 GetBCD(uint8* pkt)
{
	uint8 ret;

	//ret = ((bcd>>4)&0x0F)*10 + (bcd&0x0F);
	ret = (pkt[0]-0x30)*10 + (pkt[1]-0x30);

	return ret;
}*/

bool isHex(uint8 c)
{
	if(((c >= 0x30) && (c <= 0x39)) || ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')) )
		return true;

	return false;
}


uint8 GetHex(uint8* pkt)
{
	uint8 val;
	if((pkt[0] >= 0x30) && (pkt[0] <= 0x39))
	{
		val = pkt[0]-0x30;
	}
	else if((pkt[0] >= 'A') && (pkt[0] <= 'F'))
	{
		val = pkt[0]-'A'+0x0a;
	}
	else if((pkt[0] >= 'a') && (pkt[0] <= 'a'))
	{
		val = pkt[0]-'a'+0x0a;
	}
	else
		return 0;

	val = val<<4;

	if((pkt[1] >= 0x30) && (pkt[1] <= 0x39))
	{
		val |= pkt[1]-0x30;
	}
	else if((pkt[1] >= 'A') && (pkt[1] <= 'F'))
	{
		val |= pkt[1]-'A'+0x0a;
	}
	else if((pkt[1] >= 'a') && (pkt[1] <= 'a'))
	{
		val |= pkt[1]-'a'+0x0a;
	}
	else if(pkt[1] == ' ')
	{
	}
	else
		return 0;

	return val;
}


int ParseVANPacket(int addr, uint8* pkt, uint16 len, CParserPlugin* ptr)
{
	//First bytes are RAK,R/W and RAT flags... ignore
	pkt += 1; // header is 8 bytes
	len -= 1;
	switch(addr)
	{
		case VIN_ADDR:
			{
				VIN_MSG_T msg;
				
				if(len < 18)
				{
					OutputDebugString("Invalid van packet (VIN)\n");
					return 0;
				}
			
				memcpy(msg.VIN, pkt, 17);
				msg.VIN[17] = 0;

				ptr->deliverMsgToPlugins(addr, VIN_MSG, &msg);
			}
			break;

		case TIME_ADDR:
			{
				TIME_MSG_T msg;
			}
			break;

		case ENGINE_ADDR:
			{
				ENGINE_MSG_T msg;

				if(len < 7)
				{
					OutputDebugString("Invalid van packet (ENGINE)\n");
					return 0;
				}

				msg.brightness = pkt[0]&0x0f;
				msg.contact = pkt[1] & 0x01;
				msg.engine = pkt[1] & 0x02;
				msg.economy = pkt[1] & 0x04;
				msg.water_temp = pkt[2]/2;
				msg.oil_level = pkt[3];
				msg.fuel_level = pkt[4];
				msg.oil_temp = pkt[5]/2;
				msg.ext_temp = (pkt[6]-0x50)/2;

				ptr->deliverMsgToPlugins(addr, ENGINE_MSG, &msg);

			}
			break;

		case REMOTE_CONTROL_ADDR:
			{
				REMOTE_CONTROL_MSG_T msg;

				if(len < 3)
				{
					OutputDebugString("Invalid van packet (REMOTE_CONTROL)\n");
					return 0;
				}

				msg.controls = pkt[0];
				msg.wheel = pkt[1];
				

				ptr->deliverMsgToPlugins(addr, REMOTE_CONTROL_MSG, &msg);
			}
			break;

		case LIGHTS_STATUS_ADDR:
			{
				LIGHTS_STATUS_MSG_T msg;

				msg.lights_status = pkt[5];

				ptr->deliverMsgToPlugins(addr, LIGHTS_STATUS_MSG, &msg);
			}
			break;

		case RADIO_BUTTONS_ADDR:
			{
				RADIO_BUTTONS_MSG_T msg;
			}
			break;

		case CAR_STATUS1_ADDR:
			{
				CAR_STATUS1_MSG_T msg;

				if(len < 16)
				{
					OutputDebugString("Invalid van packet (CAR_STATUS1_MSG)\n");
					return 0;
				}

				msg.avg_consumption = /*((((uint16)(pkt[16]))<<8) | pkt[17])/10*/pkt[17]/10.0f;
				msg.avg_speed = pkt[11];
				msg.doors_status = pkt[7];
				msg.inst_consumption = /*((((uint16)(pkt[22]))<<8) | pkt[23])*/pkt[23];
				msg.range = ((((uint16)(pkt[14]))<<8) | pkt[15]);
				msg.right_stick_button = pkt[10]&0x01;
				msg.mileage = ((((uint16)(pkt[24]))<<8) | pkt[25]);

				ptr->deliverMsgToPlugins(addr, CAR_STATUS1_MSG, &msg);
			}
			break;

		case CAR_STATUS2_ADDR:
			{
				CAR_STATUS2_MSG_T msg;
			}
			break;

		case DASHBOARD_ADDR:
			{
				DASHBOARD_MSG_T msg;
				
				if(len < 3)
				{
					OutputDebugString("Invalid van packet (DASHBOARD)\n");
					return 0;
				}

				msg.rpm = ((((uint16)(pkt[0]))<<8) | pkt[1])/10;
				msg.speed = ((((uint16)(pkt[2]))<<8) | pkt[3])/100;
				msg.seq_num = (((uint32)(pkt[4]))<<16) | (((uint32)(pkt[5]))<<8) | pkt[6];
				
				ptr->deliverMsgToPlugins(addr, DASHBOARD_MSG, &msg);
			}
			break;

		case DASHBOARD_BUTTONS_ADDR:
			{
				DASHBOARD_BUTTONS_MSG_T msg;
			}
			break;

		case HEAD_UNIT_ADDR:
			{
				uint8 info_type;
				uint8 seq;
				if(len < 2)
				{
					OutputDebugString("Invalid van packet (HEAD_UNIT_INFOS)\n");
					return 0;
				}
				seq = pkt[0];
				info_type = pkt[1];
				switch(info_type)
				{
					case INFO_TYPE_RADIO:
						{
							RADIO_INFOS_MSG_T msg;
							uint16 freq;
							
							pkt += 2;
							len -= 2;
							if(len < 8)
							{
								OutputDebugString("Invalid van packet (INFO_TYPE_RADIO)\n");
								return 0;
							}

							msg.seq = seq;

							msg.rds_txt[0] = 0;
							if(len >=18)
							{
								memcpy(msg.rds_txt, pkt+10, 8); 
								msg.rds_txt[8] = 0;
							}
							
							//19 80 0E 04 05 67 F2

							freq = pkt[3];
							freq = (freq<<8) | pkt[2];
							
							
							msg.freq = (((float)freq)/20) + 50;
							
							sprintf(deb, "Radio info : rds '%s', freq %.1f\n", msg.rds_txt, msg.freq);
							OutputDebugString(deb);

							ptr->deliverMsgToPlugins(addr, RADIO_INFOS_MSG, &msg);
						}
						break;

					case INFO_TYPE_CD:
						{
							CD_INFOS_MSG_T msg;

							pkt += 2;
							len -= 2;
							if(len < 10)
							{
								OutputDebugString("Invalid van packet (INFO_TYPE_CD)\n");
								return 0;
							}

							msg.seq = seq;

							msg.minutes = GetBCD(pkt[3]);
							msg.seconds = GetBCD(pkt[4]);
							msg.track_num = GetBCD(pkt[5]);
							msg.track_count = GetBCD(pkt[6]);

							msg.total_cd_minutes = GetBCD(pkt[7]);
							msg.total_cd_seconds = GetBCD(pkt[8]);

							sprintf(deb, "INFO_TYPE_CD : CD infos : track %u (%u:%u), CD(%u:%u), %u tracks\n", msg.track_num, msg.minutes, msg.seconds, msg.total_cd_minutes, msg.total_cd_seconds, msg.track_count);
							OutputDebugString(deb);

							ptr->deliverMsgToPlugins(addr, CD_INFOS_MSG, &msg);
						}
						break;

					case INFO_TYPE_PRESET:
						{
							PRESET_INFOS_MSG_T msg;

							if(len < 6)
							{
								OutputDebugString("Invalid van packet (INFO_TYPE_PRESET)\n");
								return 0;
							}

							msg.seq = seq;

							msg.preset_txt_type = ((pkt[0])>>4)&0x0F;
							msg.preset_num = pkt[0]&0x0F;

							memcpy(msg.txt, pkt+1, 8);
							msg.txt[8] = 0;

							ptr->deliverMsgToPlugins(addr, PRESET_INFOS_MSG, &msg);
						}
						break;

					case INFO_TYPE_CDC:
						{
							CDC_INFOS_MSG_T msg;
							
							ptr->deliverMsgToPlugins(addr, CDC_INFOS_MSG, &msg);
						}
						break;

					default:
						{
							sprintf(deb, "*** Unknown HEAD_UNIT_INFOS info type 0x%02x\n", info_type);
							OutputDebugString(deb);
							//return 0;
						}
						break;
				}
			}
			break;

		case AUDIO_SETTINGS_ADDR:
			{
				AUDIO_SETTINGS_MSG_T msg;
			}
			break;

		case DISPLAY_STATUS_ADDR:
			{
				DISPLAY_STATUS_MSG_T msg;
			}
			break;

		
	}

	return 1;
}
