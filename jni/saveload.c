#include "inc/saveload.h"
//#include "inc/gz.h"
//#include "inc/zlib.h"


SETTING setting, tmpsetting;
SETTING2 setting2;
int bTurbo=0;

//uint16_t thumb_tmp[160*144];
struct tm * state_tmp_time;

int rom_load(char * rpath)
{
	char tmp[PATH_MAX];
	int romsize, ramsize;

	strcpy(RomPath,rpath);

	strcpy(tmp, RomPath);
	*(strrchr(tmp,'/')+1) = 0;
	strcpy(setting2.lastpath, tmp);
	
	bTurbo = 0;
	

	gb_destroy();
	gb_init();
	
	// wèµœt@Cð[h·éB by ruka
	romsize = load_rom(RomPath);
	if (!romsize){
		renderer_set_msg("ROM Load Failed");
		return -1;
	}

	ramsize = load_sram(SramPath);
	
	if (!gb_load_rom(rom_image, romsize, sram_space, ramsize)){
		renderer_set_msg("ROM Load Failed");
		return -1;
	}

	if(org_gbtype==1)
		renderer_set_msg("ROM TYPE:GB");
	else if(org_gbtype==2)
		renderer_set_msg("ROM TYPE:SGB");
	else if(org_gbtype==3)
		renderer_set_msg("ROM TYPE:GBC");
	


	free_rewind_states();
	allocate_rewind_states();

	set_gb_type();
			
	return romsize;
}
int state_load(char* path)
{	
	/*FILE * fd = fopen(path,"rb");
	if (!fd){
		strcat(path,".gz");
		fd = fopen(path,"rb");
		if (!fd)
			return 0;
	}

	gb_restore_state(fd,NULL);
	fclose(fd);*/
	
	byte * buf;

	
	jfile fd = jfopen(env_game_thread, path, JF_MODE_READ, NULL, 0);
	gb_restore_state(fd, NULL);
	jfclose(env_game_thread, fd);

	
	if(rom_get_info()->gb_type == 1)
		lcd_set_mpal(0);
	else if(rom_get_info()->gb_type == 2 && sgb_mode)
		lcd_set_mpal(PAL_SGB);
	else
		lcd_set_mpal(setting.gb_palette);
		
	if(rom_get_info()->gb_type>=3 && org_gbtype==3)
		now_gb_mode = 3;
	else if(rom_get_info()->gb_type==2 && sgb_mode)
		now_gb_mode = 2;
	else
		now_gb_mode = 1;
	
	border_uploaded = 2;


	free_rewind_states();
	allocate_rewind_states();
	
	return 1;

}
int state_save(char* path)
{
	size_t size = gb_save_state(NULL);
	byte *buf = (byte*)malloc(size);
	if (!buf)
	{
		return 0;
	}
	gb_save_state(buf);

	jfile fd = jfopen(env_game_thread, path, JF_MODE_WRITE | JF_MODE_OVERWRITE | JF_MODE_NEW, NULL, 0);
	
	if(fd)
	{
		jfwrite(env_game_thread, fd, buf, size);
		jfclose(env_game_thread, fd);
	}
	
	free(buf);
	
	char tmp[PATH_MAX];
	if(rom_has_battery())
	{
		sprintf(tmp, "%s.sram.gz", RomPath);
		save_sram(get_sram(), rom_get_info()->ram_size,tmp);
	}
	
	return size;
	
	//char temp[PATH_MAX];
	
	//int ret;

/*	if (0)//setting.compress)
	{
		sprintf(temp,"%s.gz",path);

		FILE * fd = fopen(temp, "w");
		if (fd<0){
			free(buf);
			return 0;
		}

		ret = gzCreate(fd, buf, size);
		fclose(fd);
		free(buf);
		
		if (!ret)
		{
			remove(temp);
			return 0;
		}
	}
	else
	{
		FILE * fd = fopen(path, "w");
		if (fd<0)
		{
			free(buf);
			return 0;
		}

		ret = fwrite(buf, size,1 ,fd);
		fclose(fd);
		free(buf);

		if (ret!=1){
			remove(path);
			return 0;
		}else{
			//strcat(path, ".gz");
			//remove_file(path);
		}
		return ret;
	}*/
}
int save_sram(byte *buf,int size, char * path)
{
	const int sram_tbl[]={1,1,1,4,16,8};
	

	/*char *p = strrchr(SavePath, '.');
	if (!strcmp(p,".gz")){
		if(!setting.compress)
			remove_file(SavePath);
	}else{
		if(setting.compress)*/
			//remove_file(path);
	/*}
	
	sprintf(SavePath, "%sSAVE/%s.sav", RinPath, RomName);
	if (setting.compress) 
	{
		strcat(SavePath, ".gz");
		FILE * fd = fopen(SavePath, "w");
		if (!fd)
			return 0;
	
		int sram_size = 0x2000 * sram_tbl[size];

		if ((rom_get_info()->cart_type>=0x0f) && (rom_get_info()->cart_type<=0x13))
		{
			int tmp = renderer_get_timer_state();
			memcpy(buf+sram_size, &tmp, sizeof(int));
			sram_size += sizeof(int);
		}

		cheat_decreate_cheat_map();
		gzCreate(fd, buf, sram_size);
		fclose(fd);
		cheat_create_cheat_map();
	}else
	{
		FILE * fd = fopen(path, "w");
		if(!fd)
		{
			return 0;
		}

		cheat_decreate_cheat_map();
		fwrite(buf, 0x2000*sram_tbl[size],1,fd);
		if ((rom_get_info()->cart_type>=0x0f)
		 && (rom_get_info()->cart_type<=0x13)){
			int tmp = renderer_get_timer_state();
			fwrite( &tmp, sizeof(int),1,fd);
		}
		fclose(fd);
		cheat_create_cheat_map();

	}*/
	cheat_decreate_cheat_map();
	
	int tmp;
	
	jfile fd = jfopen(env_game_thread, path, JF_MODE_OVERWRITE | JF_MODE_NEW | JF_MODE_WRITE, NULL,  0);
	cheat_decreate_cheat_map();
	
	jfwrite(env_game_thread,fd, buf, 0x2000*sram_tbl[size]);
	
	if ((rom_get_info()->cart_type>=0x0f)
		&& (rom_get_info()->cart_type<=0x13)){
		int tmp = renderer_get_timer_state();
		jfwrite( env_game_thread, fd, &tmp, sizeof(int));
	}
	jfclose(env_game_thread, fd);
	cheat_create_cheat_map();
		
	return 1;
}

int load_sram(char * path)
{
	jfile fd = jfopen(env_game_thread, path, JF_MODE_READ, NULL, 0);
	
	/*if(!fd){
		strcat(SavePath, ".gz");
		fd = gzopen(SavePath, "rb");
	}*/
	
	if(!fd)
	{
		return 0;
	}
	
	int ramsize = jfread(env_game_thread, fd, sram_space, 16*0x2000+4);
	jfclose(env_game_thread, fd);
	
	if(ramsize & 4)
	{
		renderer_set_timer_state(*(int*)(sram_space+ramsize-4));
	}
	return ramsize;
}
/*int save_thumb(char * path)
{
	byte *buf = (byte*)malloc(144*160*3);
	if (!buf || !edrawing)
		return 0;

	byte r, g, b;
	unsigned short color;
	unsigned int x,y;
	for(y=0; y<144; y++){
		for(x=0; x<160; x++){
			//pointer arithmetic
			color =  *(gb_get_fp() + (y*GLINE_SIZE+GGUARD_LINE+x));
			r = VREDG_8(color);
			g = VGREENG_8(color);
			b = VBLUEG_8(color);
			buf[(y*160+x)*3  ] = r;
			buf[(y*160+x)*3+1] = g;
			buf[(y*160+x)*3+2] = b;
		}
	}
	saveImage(path,(short*) buf, 160, 144, GLINE_SIZE, 0);
	free(buf);
	return 1;
}*/
/*Image* load_thumb(char * path)
{	
	Image * res =  loadImage(path);
	return res;
}*/
byte *save_state_tmp()
{
	size_t size = gb_save_state(NULL);
	time_t tt;
	uint8_t *buf = (uint8_t*)malloc(size);
	if (!buf)
		return NULL;
	gb_save_state(buf);

	unsigned int x,y;
	/*for(y=0; y<144; y++){
		for(x=0; x<160; x++){
			thumb_tmp[y*160+x] = vframe[y*GLINE_SIZE+GGUARD_LINE+x];
		}
	}*/
	time(&tt);
	state_tmp_time = localtime(&tt);
	
	return buf;
}
int load_state_tmp(byte *buf)
{
	if (!buf)
		return 0;

	gb_restore_state(0,buf);

	if(rom_get_info()->gb_type == 1)
		lcd_set_mpal(0);
	else if(rom_get_info()->gb_type == 2 && sgb_mode)
		lcd_set_mpal(PAL_SGB);
	else
		lcd_set_mpal(setting.gb_palette);
		
	if(rom_get_info()->gb_type>=3 && org_gbtype==3)
		now_gb_mode = 3;
	else if(rom_get_info()->gb_type==2 && sgb_mode)
		now_gb_mode = 2;
	else
		now_gb_mode = 1;
	
	border_uploaded = 2;

	return 1;
}
void init_config_glob()
{
	strcpy(setting2.lastpath, (const char * )RomPath);
	memset(setting2.custom_palette, 0, sizeof(word)*3*4);
	
	setting2.scale_method = SCALE_METHOD_BILINEAR_GRID;
	setting2.show_fps = 0;
	setting2.autocopy_rom = 1;
}
void save_config_glob(char * path)
{
	memcpy(setting2.custom_palette,m_pal16[PAL_CUSTOM_GLOBAL], sizeof(word)*3*4);

	//__android_log_print(ANDROID_LOG_DEBUG, "org.rin", "path in save_config_glob: %s",path);

	jfile  fd = jfopen(env_game_thread, path, JF_MODE_NEW | JF_MODE_OVERWRITE | JF_MODE_WRITE, NULL, 0);
	if(!fd)
	{
		return;
	}


	jfwrite(env_game_thread, fd, &setting2, sizeof(setting2));
	jfclose(env_game_thread, fd);
}
void load_config_glob(char * path)
{
	int i;
	char *p;

	//__android_log_print(ANDROID_LOG_DEBUG, "org.rin", "path in load_config_glob: %s",path);

	jfile fd = jfopen(env_game_thread, path, JF_MODE_READ, NULL, 0);
	
	if(!fd)
	{
		init_config_glob();
		return;
	}
	
	memset(&setting2, 0, sizeof(setting2));
	jfread(env_game_thread, fd, &setting2, sizeof(setting2));
	jfclose(env_game_thread, fd);

	memcpy(m_pal16[PAL_CUSTOM_GLOBAL],setting2.custom_palette, sizeof(short)*4*3);
	check_config_glob();
}

int check_config_glob()
{
  	if(setting2.scale_method>=SCALE_METHOD_END)
		setting2.scale_method = SCALE_METHOD_BILINEAR_GRID;
	
	if(setting2.show_fps >1)
	  setting2.show_fps =1;
	
	if(setting2.autocopy_rom >1)
	  setting2.autocopy_rom =1;
    
	return 1;
}
void init_config()
{
	int i;

	strcpy(setting.vercnf, VERCNF);
	
	setting.gb_type = 0;
	setting.gb_palette = PAL_DARK_GREEN;
	setting.frameskip = 2;
	setting.sound = 1;
	setting.sound_buffer = 0;
	setting.speed = 30;
	setting.autosave = 100;
	memset(setting.custom_palette, 0, sizeof(word)*3*4);

	setting.thumb = 1;

	for(i=0; i<32; i++)
	{
		setting.bGB_Pals[i] = 1;
	}
	
	setting.compress = 0;
	
}
void check_config()
{
	int i;

	if(strcmp(setting.vercnf, VERCNF)){
		init_config();
		return;
	}
	if(setting.speed > 100)
	  setting.speed = 30;
	if(setting.gb_type>4)
		setting.gb_type = 0;
	if(setting.gb_palette<=PAL_MONOCHROME || setting.gb_palette>=PAL_SGB)
		setting.gb_palette = PAL_DARK_GREEN;
	if(setting.frameskip > 9)
		setting.frameskip=0;
	if(setting.sound_buffer>2)
		setting.sound_buffer = 0;
	
	if(setting.autosave < 0 || setting.autosave > 9999)
		setting.autosave = 100;
		
	lcd_set_mpal(setting.gb_palette);
}
void load_config(char * path)
{
	int i;
	char *p;

	//__android_log_print(ANDROID_LOG_DEBUG, "org.rin", "path in load_config: %s",path);

	jfile fd = jfopen(env_game_thread, path, JF_MODE_READ, NULL, 0);

	if(!fd)
	{
		init_config();
		return;
	}
	
	memset(&setting, 0, sizeof(setting));
	jfread(env_game_thread, fd, &setting, sizeof(setting));
	jfclose(env_game_thread, fd);

	tmpsetting = setting;
	memcpy(m_pal16[PAL_CUSTOM_LOCAL],setting.custom_palette, sizeof(short)*4*3);
	check_config();
	set_gb_type();
	set_speed(setting.speed);
	
}
void save_config(char * path)
{
	
	char tmp[PATH_MAX];
	if(!rom_get_loaded())
	{
	    return;
	}
	else
	{
		if(rom_has_battery())
		{
			save_sram(get_sram(), rom_get_info()->ram_size,SramPath);
		}
	}

	
	memcpy(setting.custom_palette,m_pal16[PAL_CUSTOM_LOCAL], sizeof(word)*3*4);

	//__android_log_print(ANDROID_LOG_DEBUG, "org.rin", "path in save_config: %s",path);
	int i;
	for(i=0; i<sizeof(SETTING); i++){
		if ( *((byte*)&setting+i) != *((byte*)&tmpsetting+i) )
			break;
	}
	if (i>=sizeof(SETTING))
		return;


	jfile fd = jfopen(env_game_thread, path, JF_MODE_NEW | JF_MODE_OVERWRITE | JF_MODE_WRITE, NULL, 0);
	
	if(!fd)
	{
		return;
	}

	jfwrite(env_game_thread, fd, &setting, sizeof(setting));
	jfclose(env_game_thread, fd);

	tmpsetting = setting;
}
void set_gb_type()
{
	if (rom_get_loaded()){
		switch(setting.gb_type){
		case 0:
			if(org_gbtype==1){
				rom_get_info()->gb_type = 2;
				lcd_set_mpal(setting.gb_palette);
			}else if(org_gbtype == 2){
				rom_get_info()->gb_type = 2;
				lcd_set_mpal(PAL_SGB);
			}else if(org_gbtype == 3){
				rom_get_info()->gb_type = 3;
				lcd_set_mpal(PAL_SGB);
			}
			break;
		case 1:
			rom_get_info()->gb_type = 1;
			lcd_set_mpal(PAL_MONOCHROME);
			break;
		case 2:
			rom_get_info()->gb_type = 2;
			if(sgb_mode)
				lcd_set_mpal(PAL_SGB);
			else
				lcd_set_mpal(setting.gb_palette);
			break;
		case 3:
			rom_get_info()->gb_type = 3;
			lcd_set_mpal(setting.gb_palette);
			break;
		case 4:
			rom_get_info()->gb_type = 4;
			lcd_set_mpal(setting.gb_palette);
			break;
		}
		
		if(rom_get_info()->gb_type>=3 && org_gbtype==3)
			now_gb_mode = 3;
		else if(rom_get_info()->gb_type==2 && sgb_mode)
			now_gb_mode = 2;
		else
			now_gb_mode = 1;
	}
}
void set_speed(int speed)
{
  	if(speed >= 0 && speed <= 100)
	{
	    setting.speed = speed;
	    
	    setting.frameskip = speed / 10 -1;
	    

	    
	    if(speed > 30)
	    {
	      sync_time = 167 - (speed - 30);
	      slow_mod = 1;
	    }
	    else if(speed < 30)
	    {
	      167 - (speed - 30)*30;
	      slow_mod = (30 - speed)/4;
	    }
	    else
	    {
	      sync_time = 167;
	      slow_mod = 1;
	    }
	    
	    if(setting.frameskip < 0 )
	      setting.frameskip = 0;
	    
	    if(slow_mod < 1)
	      slow_mod = 1;
	}
}
int get_speed(void)
{
	if(setting.speed <=100)
	  return setting.speed;
	else
	  return 30;
}