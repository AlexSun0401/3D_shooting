#include "hw.h"
#include "HEX.h"
unsigned char seg7[10] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
}; 
void display_score(int score) {

    int d0 = score % 10;
    int d1 = (score / 10) % 10;
    int d2 = (score / 100) % 10;
    int d3 = (score / 1000) % 10;

    int value = (seg7[d3] << 24) |
                (seg7[d2] << 16) |
                (seg7[d1] << 8)  |
                (seg7[d0]); //shifting each bit

    *HEX_ptr = value;
}  //we only want to keep 4 digits appeared on the hex display since you cannot
//get that many scores
void clear_score(){
    display_score(0); //clear by displaying 0
}
