#include "keyboard.h"
#include "interrupt.h"
#include "print.h"
#include "io.h"
#include "stdint.h"
#include "stdvar.h"
#define buffer_port 0x60

//转义字符
#define backspace '\b'
#define tab '\t'
#define enter '\r'
#define esc '\033'

//控制字符通码
#define shift_l_down 0x2a
#define shift_r_down 0x36
#define ctrl_l_down 0x1d
#define ctrl_r_down 0xe01d
#define alt_l_down 0x38
#define alt_r_down 0xe038
#define capslk_down 0x3a

//控制键状态
static bool shift_state, ctrl_state, alt_state, capslk_state;  //false.断  true.通

//记录0xe开头
static bool xe_scancode;

//通码转字符
//索引 1:未按下shift 2：按下shift
static char keyboardmap[][2] = {
    {0, 0},
    {esc, esc},
    {'1', '!'},
    {'2', '@'},
    {'3', '#'},
    {'4', '$'},
    {'5', '%'},
    {'6', '^'},
    {'7', '&'},
    {'8', '*'},
    {'9', '('},
    {'0', ')'},
    {'-', '_'},
    {'=', '+'},
    {backspace, backspace},
    {tab, tab},
    {'q', 'Q'},
    {'w', 'W'},
    {'e', 'E'},
    {'r', 'R'},
    {'t', 'T'},
    {'y', 'Y'},
    {'u', 'U'},
    {'i', 'I'},
    {'o', 'O'},
    {'p', 'P'},
    {'[', '{'},
    {']', '}'},
    {enter, enter},
    {0, 0},
    {'a', 'A'},
    {'s', 'S'},
    {'d', 'D'},
    {'f', 'F'},
    {'g', 'G'},
    {'h', 'H'},
    {'j', 'J'},
    {'k', 'K'},
    {'l', 'L'},
    {';', ':'},
    {'\'', '"'},
    {'`', '~'},
    {0, 0},
    {'\\', '|'},
    {'z', 'Z'},
    {'x', 'X'},
    {'c', 'C'},
    {'v', 'V'},
    {'b', 'B'},
    {'n', 'N'},
    {'m', 'M'},
    {',', '<'},
    {'.', '>'},
    {'/', '?'},
    {0, 0},
    {0, 0},
    {0, 0},
    {' ', ' '},
    {0, 0}
};


//键盘中断服务程序
void keyboard_int_handler(){
    //控制键当前状态
    bool shift_cur_state = shift_state;
    bool ctrl_cur_state = ctrl_state;
    bool alt_cur_state = alt_state;
    bool capslk_cur_state = capslk_state;

    //获取 scancode
    uint16_t scancode = inb(buffer_port);

    //判断是否0xe开头的多字节
    if(scancode == 0xe0){
        xe_scancode = true;
        return;
    }

    //合并上次的0xe部分
    if(xe_scancode == true){
        scancode = ((0xe000)|scancode);
        xe_scancode = false;
    }
    
    if((scancode & 0x0080)){//scancode为断码,则更新控制键状态
        uint16_t down_code = (scancode &= 0xff7f); //
        if(down_code == shift_l_down | down_code == shift_r_down){
            shift_state = false;
        }else if(down_code == ctrl_l_down | down_code == ctrl_r_down){
            ctrl_state = false;
        }else if(down_code == alt_l_down | down_code == alt_r_down){
            alt_state = false;
        }
        return;
    }else if((scancode > 0x00 && scancode < 0x3b) || (scancode == ctrl_r_down) || (scancode == alt_r_down)){//处理通码
        bool shift = false;     
        if((scancode < 0x0e) || (scancode == 0x29) || \
           (scancode == 0x1a) || (scancode == 0x1b) || \
           (scancode == 0x2b) || (scancode == 0x27) || \
           (scancode == 0x28) || (scancode == 0x33) || \
           (scancode == 0x34) || (scancode == 0x35)){
            if(shift_cur_state){
                shift = true;
            }
        }else{
            if(shift_cur_state && capslk_cur_state){
                shift = false;
            }else if(shift_cur_state || capslk_cur_state){
                shift = true;
            }else{
                shift = false;
            }
        }
        uint8_t cur_scancode = (scancode &= 0x00ff);
        char cur_char = keyboardmap[cur_scancode][shift];  
        
        //打印可见字符
        if(cur_char){
            put_char(cur_char);
            return;
        }
        //如果是控制键通码则更新控制键状态,用来与下次中断的按键组合
        if(scancode == shift_l_down || scancode == shift_r_down){
            shift_state = true;
        }else if(scancode == ctrl_l_down || scancode == ctrl_r_down){
            ctrl_state = true;
        }else if(scancode == alt_l_down || scancode == alt_r_down){
            alt_state = true;
        }else if(scancode == capslk_down){
            capslk_state = !capslk_cur_state;
        }
    }else{
        put_str("unknown key!\n");
    }
}   //需增加左右方向键

//键盘初始化
void keyboard_init(){
    put_str("keyboard_init start...\n");
    int_handler_reg(0x21, keyboard_int_handler);
    put_str("keyboard_init done.\n");
}