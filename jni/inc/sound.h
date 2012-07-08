
extern int wavout_enable;
extern unsigned long cur_play;
extern unsigned int nextbank;

//サウンドバッファ１バンクあたりの容量。４バンクで適当にラウンドロビン
//PGA_SAMPLESの倍数にすること。PGA_SAMPLESと同じだと多分ダメなので注意。 - LCK

//Attention: the requestchunk size must fit into of banks * sound_buff_len
#define SOUND_BUF_LEN 1024

#define BANKS 2
extern short * sound_buf;

void wavoutClear();
int wavoutInit();

