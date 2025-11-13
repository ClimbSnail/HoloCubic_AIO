#include "LHLXW_GUI.h"
#include "LHLXW_StartAnim.h"
#include "arduino.h"

LV_FONT_DECLARE(APP_OPTION_ico);  // Define option characters

LHLXW_RUN *lhlxw_run = NULL;

/* Gradient color style for function options */
static const int option_color[5][2] = {
    {0x7303c0,0xec38bc},
    {0xdcffbd,0xcc86d1},
    {0xfc466b,0x3f5efb},
    {0xfbd786,0x1cefff},
    {0x00f260,0x0575e6},
};

/* X-coordinate constants for function options */
static const uint8_t x_zu[5]      = {24,46,74,130,168};
/* Y-coordinate constants for function options */
static const uint8_t y_zu[5]      = {88,72,54,72,88};
/* Width constants for function options */
static const uint8_t width_zu[5]  = {50,68,100,68,50};
/* Height constants for function options */
static const uint8_t height_zu[5] = {92,124,160,124,92};

/* The above four arrays are sufficient, but the following array is added for programming convenience */
static const uint8_t obj_var[5][4][2]={
  //   x        y       w       h
    {{168,24},{88,88},{48,48},{92,92}},   // 4 transitions to 0
    {{24,46},{88,72},{48,64},{92,124}},   // 0 transitions to 1
    {{46,74},{72,54},{64,92},{124,160}},  // 1 transitions to 2
    {{74,130},{54,72},{92,64},{160,124}}, // 2 transitions to 3
    {{130,168},{72,88},{64,48},{124,92}}, // 3 transitions to 4
};

/* Relationship between layers and option_num */
struct tierMode{
    uint8_t tierButton[3];  // Button layer
    char tierText[12];      // Label layer
};
#define OPTION_1  "\xee\x99\xa1"  // Eye
#define OPTION_2  "\xee\x98\xab"  // Dice
#define OPTION_3  "\xee\x98\x8d"  // Heartbeat
#define OPTION_4  "\xee\x98\x9d"  // Code
#define OPTION_5  "\xee\x98\xb0"  // Emoji
static const struct tierMode tier_mode[5] = {
    {{2,1,3},OPTION_1},
    {{1,2,0},OPTION_2},
    {{0,1,4},OPTION_3},
    {{4,0,3},OPTION_4},
    {{3,4,2},OPTION_5}
};

static void x_cb(lv_obj_t *var,int32_t v){
    lv_obj_set_x(var,v);
}
static void y_cb(lv_obj_t *var,int32_t v){
    lv_obj_set_y(var,v);
}
static void w_cb(lv_obj_t *var,int32_t v){
    lv_obj_set_width(var,v);
}
static void h_cb(lv_obj_t *var,int32_t v){
    lv_obj_set_height(var,v);
}


void LHLXW_GUI_Init(void){
    lhlxw_run->LV_BACKUP_OBJ = lv_scr_act();  // Backup previous screen to restore when exiting APP (tested that issues occur if not switching to backup screen when exiting APP)
    lhlxw_run->LV_LHLXW_GUI_OBJ = lv_obj_create(NULL);  // Create an empty active page as app main page
    /* Placing the startup animation here avoids consuming a lot of memory at once, but the emoji menu appears very abruptly after the animation ends */
    //startLog(LV_LHLXW_GUI_OBJ);  // Start log animation, after animation ends will switch to LV_LHLXW_GUI_OBJ page which is the APP main page
    
    lv_obj_set_style_bg_color(lhlxw_run->LV_LHLXW_GUI_OBJ,lv_color_hex(0),LV_STATE_DEFAULT);
   
    for(uint8_t i=0;i<5;i++){
        lhlxw_run->option_fun[i] = lv_btn_create(lhlxw_run->LV_LHLXW_GUI_OBJ);
        lv_obj_set_pos(lhlxw_run->option_fun[i],x_zu[i],y_zu[i]);
        lv_obj_set_size(lhlxw_run->option_fun[i],width_zu[i],height_zu[i]);
        lv_obj_set_style_bg_color(lhlxw_run->option_fun[i], lv_color_hex(option_color[i][0]),LV_STATE_DEFAULT);  // Start color
        lv_obj_set_style_bg_grad_color(lhlxw_run->option_fun[i],lv_color_hex(option_color[i][1]),LV_STATE_DEFAULT);  // End color
        lv_obj_set_style_bg_grad_dir(lhlxw_run->option_fun[i],LV_GRAD_DIR_VER,LV_STATE_DEFAULT);  // Enable gradient color
    }

    lv_obj_move_foreground(lhlxw_run->option_fun[tier_mode[0].tierButton[2]]);
    lv_obj_move_foreground(lhlxw_run->option_fun[tier_mode[0].tierButton[1]]);
    lv_obj_move_foreground(lhlxw_run->option_fun[tier_mode[0].tierButton[0]]);

    lhlxw_run->option_label = lv_label_create(lhlxw_run->LV_LHLXW_GUI_OBJ);  // Create label based on button
    lv_obj_set_align(lhlxw_run->option_label,LV_ALIGN_CENTER);
    lv_obj_set_style_text_font(lhlxw_run->option_label,&APP_OPTION_ico,LV_STATE_DEFAULT );
    lv_label_set_text(lhlxw_run->option_label,tier_mode[0].tierText);
    /* Placing it here will cause the emoji page and animation page to exist simultaneously, requiring a lot of RAM */
    startLog(lhlxw_run->LV_LHLXW_GUI_OBJ);  // Start log animation, after animation ends will switch to LV_LHLXW_GUI_OBJ page which is the APP main page
}

/* Switch options */
void SWITCH_OPTION(bool _flg_,uint8_t mode){
    lv_anim_t a1;
    lv_anim_init(&a1);
    lv_anim_set_time(&a1,607);
    lv_anim_set_path_cb(&a1,lv_anim_path_ease_in);
    for(uint8_t i=0;i<5;i++){
        lv_anim_set_var(&a1,lhlxw_run->option_fun[i]);
        for(uint8_t ii=0;ii<4;ii++){
            if(_flg_)  // Left to right
                lv_anim_set_values(&a1,obj_var[(i+mode)%5][ii][0],obj_var[(i+mode)%5][ii][1]);
            else  // Right to left
                lv_anim_set_values(&a1,obj_var[(i+mode+1)%5][ii][1],obj_var[(i+mode+1)%5][ii][0]);
            switch(ii){
                case 0:  // x change
                    lv_anim_set_exec_cb(&a1,(lv_anim_exec_xcb_t)x_cb);
                break;
                case 1:  // y change
                    lv_anim_set_exec_cb(&a1,(lv_anim_exec_xcb_t)y_cb);
                break;
                case 2:  // w change
                    lv_anim_set_exec_cb(&a1,(lv_anim_exec_xcb_t)w_cb);
                break;
                case 3:  // h change
                    lv_anim_set_exec_cb(&a1,(lv_anim_exec_xcb_t)h_cb);
                break;
            }
            lv_anim_start(&a1);
        }
    }
    lv_obj_move_foreground(lhlxw_run->option_fun[tier_mode[mode].tierButton[2]]);
    lv_obj_move_foreground(lhlxw_run->option_fun[tier_mode[mode].tierButton[1]]);
    lv_obj_move_foreground(lhlxw_run->option_fun[tier_mode[mode].tierButton[0]]);
    lv_label_set_text(lhlxw_run->option_label,tier_mode[mode].tierText);
    lv_obj_move_foreground(lhlxw_run->option_label);
}

void LHLXW_GUI_DeInit(void){
    /* Switch page and delete old screen (not using the function's built-in delete, manually delete after execution) */
    lv_scr_load_anim(lhlxw_run->LV_BACKUP_OBJ, LV_SCR_LOAD_ANIM_OUT_BOTTOM, 573, 0, false);  // Must wait for animation to end before calling system exit function, otherwise system will restart

    /* Add delay here to prevent calling system exit function or deleting animation object before animation completes, which would cause system errors */
    for(uint16_t i=0;i<573;i++){
        lv_task_handler();
        delay(1);
    }
    /* If manually deleting objects, must wait until object's animation is complete before deleting, otherwise there will be problems */
    lv_obj_clean(lhlxw_run->LV_LHLXW_GUI_OBJ);  // Delete all child items of object
    lv_obj_del(lhlxw_run->LV_LHLXW_GUI_OBJ);  // Delete object (tested to release memory, no memory leaks)
}
