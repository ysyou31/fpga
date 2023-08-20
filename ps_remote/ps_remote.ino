//#define FPGA

#include <Ps3Controller.h>
#include "util.h"

//#define DEBUG1 // show pad inputs.
//#define DEBUG2 // show transmit buffer


#define RF Serial1
uint8_t defPadBuf[] = {0xFF, 0xFF, 0x0, 0x0, 0x64, 0x0, 0x0, 0x64, 0x0, 0x0, 0xC8}; // all sticks middle, no button pressed.

void getPS3data();
GamepadData pad;

void setup()
{
    Serial.begin(115200);
    RF.begin(9600, SERIAL_8N1, 16, 17);
    Ps3.begin("12:34:56:78:9a:bc");
    Serial.println("Ready.");

    pad.data.frame_start=0xffff;
}

uint32_t cur_bk;
void loop()
{
  if(millis()-cur_bk>100){
    if(Ps3.isConnected()){
      getPS3data();
      uint8_t chkSum=0;
      for(int i=0; i<FRAME_DATA_SIZE; i++){
        chkSum+=pad.buf[2+i];
      }
      pad.data.chk_sum=chkSum;
      #ifdef DEBUG2
        Serial.print("["); 
        for(int i=0; i<FRAME_DATA_SIZE+3; i++){
          Serial.print("0x"); 
          Serial.print(pad.buf[i], HEX); 
          Serial.print(", ");
        }
        Serial.println("]"); 
      #endif // DEBUG2
    }else{
      memncpy(defPadBuf, pad.buf, FRAME_DATA_SIZE+3);
    }
#ifdef FPGA  // FPGA에서는 0xff, 0xff, x-stick, accel, check_sum 으로 5byte로 줄여보냄. gamepad struct의 buffer를 수정해서 전송함.
    pad.buf[2] = pad.data.lx; // zero zone 미리 적용되어서 0~199 range임.
    pad.buf[3] = map(pad.data.r_trigger - pad.data.l_trigger, -255, 255, 0, 200); // zero zone없음. 눌린값 그대로 R-L trigger값을 0~199로 매핑하여 전송.
    pad.buf[4] = pad.buf[2]+pad.buf[3]; // add checksum
    RF.write(pad.buf, 5);  // Transmitt
#else
    RF.write(pad.buf, FRAME_DATA_SIZE+3);
#endif
    cur_bk=millis();    
  }
}


void getPS3data(){
  if(Ps3.isConnected()){
    if(abs(Ps3.data.analog.stick.lx)>24){ // left stick X(-128~128) map to range of 1~200. -25~25 is zero zone.
      if(Ps3.data.analog.stick.lx>0){
        pad.data.lx=map(Ps3.data.analog.stick.lx, 25, 128, 101,200);
      }else{
        pad.data.lx=map(Ps3.data.analog.stick.lx, -128, -25, 0,99);
      }
#ifdef DEBUG1
      Serial.print("lx="); Serial.println(Ps3.data.analog.stick.lx, DEC);
#endif
    }else
      pad.data.lx=100; // 0~200
    
    if(abs(Ps3.data.analog.stick.rx)>24 || abs(Ps3.data.analog.stick.ry)>24){
      if(Ps3.data.analog.stick.rx>0){
        pad.data.rx=map(Ps3.data.analog.stick.rx, 25, 128, 101,200);
      }else if(Ps3.data.analog.stick.rx<0){
        pad.data.rx=map(Ps3.data.analog.stick.rx, -128, -25, 0,99);
      }else{
        pad.data.rx=100;
      }
      
      if(Ps3.data.analog.stick.ry>24){
        pad.data.ry=map(Ps3.data.analog.stick.ry, 25, 128, 101,200);
      }else if(Ps3.data.analog.stick.rx<-24){
        pad.data.ry=map(Ps3.data.analog.stick.ry, -128, -25, 0,99);
      }else{
        pad.data.ry=100;
      }

#ifdef DEBUG1
      Serial.print("rx="); Serial.println(Ps3.data.analog.stick.rx, DEC);
      Serial.print("ry="); Serial.println(Ps3.data.analog.stick.ry, DEC);
#endif
    }else
      pad.data.ry=pad.data.ry=100;

#ifdef DEBUG1
      if(Ps3.data.analog.button.l2){  // right trigger
        Serial.print("L-Trig : "); Serial.println(Ps3.data.analog.button.l2);
      }
#endif
      pad.data.l_trigger=Ps3.data.analog.button.l2;//, 11, 255, 0, 100);
    
    
#ifdef DEBUG1
      if(Ps3.data.analog.button.r2) { // right trigger
        Serial.print("R-Trig : "); Serial.println(Ps3.data.analog.button.r2);
      }
#endif
      pad.data.r_trigger=Ps3.data.analog.button.r2; //, 11, 255, 0, 100);

#ifdef DEBUG1
    if(Ps3.data.analog.button.l1) // left bumper
      Serial.println("L-Bump pressing");
    if(Ps3.data.analog.button.r1) // rifht bumper
      Serial.println("R-Bump pressing");
#endif
    pad.data.btns.btn_set.lb = Ps3.data.analog.button.l1? true:false;
    pad.data.btns.btn_set.rb = Ps3.data.analog.button.r1? true:false;

#ifdef DEBUG1
    // D-Pad
    if(Ps3.data.analog.button.up || Ps3.data.button.down
        || Ps3.data.analog.button.right || Ps3.data.analog.button.left){
      Serial.println("D-Pad pressing");
    }
#endif

    pad.data.btns.btn_set.up = Ps3.data.analog.button.up? true:false;
    pad.data.btns.btn_set.dn = Ps3.data.analog.button.down? true:false;
    pad.data.btns.btn_set.right = Ps3.data.analog.button.right? true:false;
    pad.data.btns.btn_set.left = Ps3.data.analog.button.left? true:false;

    pad.data.btns.btn_set.a = Ps3.data.button.cross? true:false;
    pad.data.btns.btn_set.b = Ps3.data.button.circle? true:false;
    pad.data.btns.btn_set.x = Ps3.data.button.square? true:false;
    pad.data.btns.btn_set.y = Ps3.data.button.triangle? true:false;

#ifdef DEBUG1        
    if( Ps3.data.button.cross )   Serial.println("Pressing the cross button");
    if( Ps3.data.button.square )     Serial.println("Pressing the square button");
    if( Ps3.data.button.triangle )   Serial.println("Pressing the triangle button");
    if( Ps3.data.button.circle )     Serial.println("Pressing the circle button");
#endif        
  }
}
