#include "hw.h"
#include "input.h"

int key_w = 0, key_a = 0, key_s = 0, key_d = 0, key_z = 0;
int key_q = 0, key_e = 0, key_f = 0, key_x = 0, key_r = 0, key_g = 0;
int break_flag = 0;
int extended_flag = 0;
int fire_handled = 0;
int exit_handled = 0;

int mouse_left = 0;
int mouse_right = 0;
int mouse_dx = 0;

void mouse_init(void) {
    *PS2_MOUSE = 0xF4;

    while (1) {
        unsigned int data = *PS2_MOUSE;
        if ((data & 0x8000u) != 0) {
            unsigned char byte = (unsigned char)(data & 0xFFu);
            if (byte == 0xFA) break;
        }
    }
}

void ps2_poll_update_all(void)
{
    while (1)
    {
        unsigned int data = *PS2_KB;
        if ((data & 0x8000u) == 0) break;

        unsigned char byte = (unsigned char)(data & 0xFFu);

        if (byte == 0xE0) {
            extended_flag = 1; //extended means some special operation in happening
            //for instance: pressing "space" key, although we don't have to worry about it
        } else if (byte == 0xF0) {
            break_flag = 1; //break means key released
        } else {
            int pressed = !break_flag;

            if (!extended_flag) {
                if (byte == 0x1D) key_w = pressed;
                else if (byte == 0x1C) key_a = pressed;
                else if (byte == 0x1B) key_s = pressed;
                else if (byte == 0x23) key_d = pressed;
                else if (byte == 0x15) key_q = pressed;
                else if (byte == 0x24) key_e = pressed;
                else if (byte == 0x2B) key_f = pressed;
                else if (byte == 0x34) key_g = pressed;
                else if (byte == 0x22) key_x = pressed;
                else if (byte == 0x2D) key_r = pressed;
                else if (byte == 0x1A) key_z = pressed;
            }  //check if any of keys are pressed
            //remember we check every game loop, so this is kind of like a polling method
            break_flag = 0;
            extended_flag = 0;
        }   //you should reset variables
    }

    static int m_byte_count = 0;
    static unsigned char m_packet[3];

    while (1) {
        unsigned int m_data = *PS2_MOUSE;
        if ((m_data & 0x8000u) == 0) break;//15bit = Receive Valid

        unsigned char byte = (unsigned char)(m_data & 0xFFu);//front 8bit
        //3-byte packet: left,right, x negative, y negative; dx; dy
        if (m_byte_count == 0) {
            if ((byte & 0x08) == 0x08) {//1th byte's bit3 is 1(safety check)
                m_packet[0] = byte;
                m_byte_count++;
            }
        } else if (m_byte_count == 1) {
            m_packet[1] = byte;
            m_byte_count++;
        } else {
            m_packet[2] = byte;
            m_byte_count = 0;

            mouse_left  = m_packet[0] & 0x01;//whether push left
            mouse_right = (m_packet[0] >> 1) & 0x01;//push right

            int dx = m_packet[1];
            if (m_packet[0] & 0x10) dx |= 0xFFFFFF00;//higher 24bit becomes 1
            mouse_dx -= dx;//rotating direction
        }
    }
}