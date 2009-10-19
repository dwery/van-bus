#ifndef _PLUGINS_DEFS_H_
#define _PLUGINS_DEFS_H_

#include "../types.h"

#define BACKGROUND_PLUGIN	1
#define RENDERER_PLUGIN		2
#define LOGGER_PLUGIN		3




typedef int (_stdcall *TFGetPluginType)(
	void
);

typedef int (_stdcall *TFInitPlugin)(
	void* pParent
);

typedef int (_stdcall *TFClosePlugin)(
	void
);

typedef int (_stdcall *TFProcessMessage)(
	int addr, unsigned int msgType, void* msg
);

typedef int (_stdcall *TFProcessRawVANPacket)(
	int addr, uint8* pkt, uint16 len
);


// Some VAN addresses
#define VIN_ADDR					0xE24
#define ENGINE_ADDR					0x8A4
#define REMOTE_CONTROL_ADDR			0x9C4
#define LIGHTS_STATUS_ADDR			0x4FC
#define RADIO_BUTTONS_ADDR			0x8C4
#define CAR_STATUS1_ADDR			0x564
#define CAR_STATUS2_ADDR			0x524
#define DASHBOARD_ADDR				0x824
#define DASHBOARD_BUTTONS_ADDR		0x664
#define HEAD_UNIT_ADDR				0x554
#define TIME_ADDR					0x984
#define AUDIO_SETTINGS_ADDR			0x4D4
#define DISPLAY_STATUS_ADDR			0x5E4
//#define ???						0x8D4
//#define ???						0x200
//#define ???						0xF00
//#define ???						0x504
//#define ???						0x4C5
//////////////////////////////////////////////
// The corresponding message types
#define VIN_MSG					1
#define ENGINE_MSG				2
#define REMOTE_CONTROL_MSG		3
#define LIGHTS_STATUS_MSG		4
#define RADIO_BUTTONS_MSG		5
#define CAR_STATUS1_MSG			6
#define CAR_STATUS2_MSG			7
#define DASHBOARD_MSG			8
#define DASHBOARD_BUTTONS_MSG	9
#define TIME_MSG				10
#define AUDIO_SETTINGS_MSG		11
#define DISPLAY_STATUS_MSG		12
//Messages related to Head Unit
#define RADIO_INFOS_MSG			31
#define CD_INFOS_MSG			32
#define PRESET_INFOS_MSG		33
#define CDC_INFOS_MSG			34

//////////////////////////////////////////////
// The messages structures
typedef struct _VIN_MSG_T
{
	char VIN[18];
} VIN_MSG_T;

typedef struct _TIME_MSG_T
{
	uint8 hour;
	uint8 min;
} TIME_MSG_T;

typedef struct _ENGINE_MSG_T
{
	uint8 brightness;
	
	uint8 contact;
	uint8 engine;
	uint8 economy;

	int8 water_temp;
	uint8 oil_level;
	uint8 fuel_level;
	int8 oil_temp;

	int8 ext_temp; // external sensor temperature

} ENGINE_MSG_T;

typedef struct _REMOTE_CONTROL_MSG_T
{
	uint8 controls; // bitfield -> bits are defined below, CONTROL_xxxx
	uint8 wheel;
} REMOTE_CONTROL_MSG_T;

typedef struct _LIGHTS_STATUS_MSG_T
{
	uint8 lights_status; // bitfield, bit values below, LIGHT_xxxxx

} LIGHTS_STATUS_MSG_T;

typedef struct _RADIO_BUTTONS_MSG_T
{
	uint8 buttons; // values defined in RADIO_BUTTON_xxxx

	uint8 released; // set to ? if the button was released.
} RADIO_BUTTONS_MSG_T;

typedef struct _CAR_STATUS1_MSG_T
{
	uint8 doors_status;// bitfield, bits below, DOOR_xxxxx

	uint8	right_stick_button;
	uint8	avg_speed;//in km/h
	uint16	range; //total number of km
	float   avg_consumption; // in l/100km
	float	inst_consumption; // in l/100km

	uint16 mileage; //in km

} CAR_STATUS1_MSG_T;

typedef struct _CAR_STATUS2_MSG_T
{
	uint8 handbrake;
	uint8 left_stick_button;
	uint8 seatbelt;

	uint8 esp_asr_disabled;
	uint8 car_locked;
	uint8 doors_open;
	uint8 asp_esr_disabled;

	uint8 auto_light;
	uint8 auto_wiping;
} CAR_STATUS2_MSG_T;

typedef struct _DASHBOARD_MSG_T
{
	uint16 rpm;// engine rpm
	uint16 speed;// car speed

	uint32 seq_num; // sequence number...
} DASHBOARD_MSG_T;

typedef struct _DASHBOARD_BUTTONS_MSG_T
{
	uint8 button;// bitfield, bit defined by DASHBOARD_BUTTON_xxxx
	uint8 brightness;
} DASHBOARD_BUTTONS_MSG_T;

//Message structure when the HeadUnit is in radio mode
typedef struct _RADIO_INFOS_MSG_T
{
	uint8 seq; //sequence number
	
	float freq;
	char rds_txt[9];

} RADIO_INFOS_MSG_T;

//Message structure when the HeadUnit is in CD mode
typedef struct _CD_INFOS_MSG_T
{
	uint8 seq; //sequence number

	uint8 minutes;
	uint8 seconds;
	uint8 track_num;
	uint8 track_count;

	uint8 total_cd_minutes;
	uint8 total_cd_seconds;

} CD_INFOS_MSG_T;

typedef struct _PRESET_INFOS_MSG_T
{
	uint8 seq; //sequence number

	uint8 preset_txt_type; // type of the txt field. Values are PRESET_TXT_TYPE_XXXX
	uint8 preset_num;

	char txt[9];
} PRESET_INFOS_MSG_T;

typedef struct _CDC_INFOS_MSG_T
{
} CDC_INFOS_MSG_T;

typedef struct _AUDIO_SETTINGS_MSG_T
{
} AUDIO_SETTINGS_MSG_T;

typedef struct _DISPLAY_STATUS_MSG_T
{
} DISPLAY_STATUS_MSG_T;
//////////////////////////////////////////////////


// Remote Control (steering wheel) defines
#define CONTROL_NEXT		0x80
#define CONTROL_PREV		0x40
#define CONTROL_VOL_UP		0x08
#define CONTROL_VOL_DOWN	0x04
#define CONTROL_SOURCE		0x02


// Light bits
#define LIGHT_HEAD_LOW					0x80
#define LIGHT_HEAD_HIGH					0x40
#define LIGHT_FOG_FRONT					0x20
#define LIGHT_FOG_REAR					0x10
#define LIGHT_RIGHT_SIDE_INDICATOR		0x08
#define LIGHT_LEFT_SIDE_INDICATOR		0x04


// Doors bits
#define DOOR_FRONT_RIGHT			0x80
#define DOOR_FRONT_LEFT				0x40
#define DOOR_REAR_RIGHT				0x20
#define DOOR_REAR_LEFT				0x10
#define DOOR_BOOT					0x08


//Dashboard bitton bits
#define DASHBOARD_BUTTON_LOCK		0x40
#define DASHBOARD_BUTTON_WARNING	0x02


// Radio buttons values
#define RADIO_BUTTON_1				1
#define RADIO_BUTTON_2				2
#define RADIO_BUTTON_3				3
#define RADIO_BUTTON_4				4
#define RADIO_BUTTON_5				5
#define RADIO_BUTTON_6				6
#define RADIO_BUTTON_AUDIO_UP		0x10
#define RADIO_BUTTON_AUDIO_DOWN		0x11
#define RADIO_BUTTON_AUDIO			0x16
#define RADIO_BUTTON_RADIO			0x1B
#define RADIO_BUTTON_CD				0x1D
#define RADIO_BUTTON_CDC			0x1E

// Head Unit info types
#define INFO_TYPE_RADIO				0xD1
#define INFO_TYPE_CD				0xD6
#define INFO_TYPE_PRESET			0xD3
#define INFO_TYPE_CDC				0xD5 // well, not the real value, but until I have it, I will use this one

// // type of the txt field in PRESET_INFOS_MSG_T structure
#define PRESET_TXT_TYPE_RDS			0x0C
#define PRESET_TXT_TYPE_FREQ		0x04

#endif //_PLUGINS_DEFS_H_
