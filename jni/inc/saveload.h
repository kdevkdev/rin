#ifndef SAVE_H
#define SAVE_H

#include "main.h"

int rom_load(char* rpath);

int state_load(char* file);
int state_save(char* file);

void save_config(char * path);
void load_config(char * path);
void init_conifg();

void save_config_glob(char * path);
void load_config_glob(char * path);
void init_config_glob();

int save_sram(byte *buf,int size, char * path);
int load_sram(char * path);

int save_thumb(char * path);
//Image* load_thumb(char * path);

int save_state(char * path);
int load_state(char * path);
byte *save_state_tmp();
int load_state_tmp(byte *buf);

void set_gb_type();

void set_speed(int speed);
int get_speed(void);

#define VERRIN "0.1"
#define VERCNF "ARIN 0.4"

typedef struct
{
	char lastpath[PATH_MAX];
	word custom_palette[3][4];
	uint8_t scale_method;
	uint8_t show_fps;
	uint8_t autocopy_rom;
} SETTING2;

extern SETTING2 setting2;

typedef struct
{
	char vercnf[16];
	uint8_t compress;
	uint8_t thumb;
	uint8_t gb_type;
	uint8_t gb_palette;
	uint8_t bGB_Pals[32];
	uint8_t frameskip;
	uint8_t sound_buffer;
	uint8_t speed;
	uint32_t autosave;
	int sound;
	word custom_palette[3][4];
	uint8_t show_help;
} SETTING;

extern SETTING setting, tmpsetting;
extern int bTurbo;

extern int8_t *state_tmp;
//extern uint16_t thumb_tmp[160*144];
extern struct tm * state_tmp_time;

 

#endif