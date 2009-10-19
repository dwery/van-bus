#ifndef _SKIN_H_
#define _SKIN_H_



#define EXIT_BUTTON_INDEX			0
#define ABS_INDEX					1
#define ENGINE_INDEX				2
#define BATTERY_INDEX				3
#define AIR_BAG_INDEX				4
#define BELT_INDEX					5
#define BRAKE_INDEX					6
#define LOW_LIGHT_INDEX				7
#define HIGH_LIGHT_INDEX			8
#define REAR_FOG_LIGHT_INDEX		9
#define FL_DOOR_INDEX				10
#define FR_DOOR_INDEX				11
#define RL_DOOR_INDEX				12
#define RR_DOOR_INDEX				13
#define BOOT_INDEX					14
#define SPEED_INDEX					15
#define RPM_INDEX					16
#define OIL_TEMP_INDEX				17
#define OIL_LEVEL_INDEX				18
#define EXT_TEMP_INDEX				19
#define WATER_TEMP_INDEX			20
#define FUEL_INDEX					21
#define AVG_SPEED_INDEX				22
#define RANGE_INDEX					23
#define AVG_CONSUMPTION_INDEX		24
#define INSTANT_CONSUMPTION_INDEX	25
#define SOURCE_NAME_INDEX			26
#define PRESET_INDEX				27
#define INFO_TXT_INDEX				28

#define MAX_ENTRIES					29 // max elem_t entries

#define SMALL_FONT_SIZE		0
#define MEDIUM_FONT_SIZE	1
#define BIG_FONT_SIZE		2

#define CENTER_ALIGNMENT_OPTION		0
#define TRUNCATE_OPTION				1
#define NEW_LINE_OPTION				2
#define LEFT_ALIGNMENT_OPTION		3



#define MAX_AUTHOR_STRING_LEN  50
#define MAX_COMMENT_STRING_LEN 100
#define MAX_EMAIL_STRING_LEN   50

typedef struct _skin_header_t
{
	char magic[10]; //VDASH-PSA
	float version;
	char author[MAX_AUTHOR_STRING_LEN];
	char email[MAX_EMAIL_STRING_LEN];
	char comment[MAX_COMMENT_STRING_LEN];
} skin_header_t;

#endif //_SKIN_H_

