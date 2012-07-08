//---------------------------------------------
// エミュレーション結果の表現法/インターフェース

#ifndef RENDERER_H
#define RENDERER_H


extern int pad_state;
extern char render_msg[128];
extern int render_msg_mode;
extern long render_msg_time;

void renderer_init();
void renderer_reset();
void renderer_refresh();
void render_screen(void *buf);
byte renderer_get_time(int type);
word renderer_get_sensor(bool x_y);
void renderer_set_time(int type,byte dat);
void renderer_set_bibrate(bool bibrate);
int renderer_get_timer_state();
void renderer_set_msg(const char msg[]);

#endif
