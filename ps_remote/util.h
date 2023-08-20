#define FRAME_START 0xffff

#define FRAME_DATA_SIZE 8 // not include frmae_start, chksum

struct _btns {    // 순서가 거꾸로임. 주의할것.
    uint8_t a:1; // 0x01
    uint8_t b:1; // 0x02
    uint8_t x:1; // 0x04
    uint8_t y:1; // 0x08
    uint8_t lb:1; // 0x10
    uint8_t rb:1; // 0x20
    uint8_t btns:2;

    uint8_t up:1; // 01
    uint8_t dn:1; 
    uint8_t left:1;
    uint8_t right:1;//08
    uint8_t ltmb:1;//10
    uint8_t rtmb:1;
    uint8_t start:1;
    uint8_t menu:1; //80
};

typedef union {
    uint16_t var;
    struct _btns btn_set;
}Btns;

typedef union{
  uint8_t buf[FRAME_DATA_SIZE+3]; // Start 16bit+8bit checksum
  struct{
    uint16_t frame_start;
    Btns btns;
    uint8_t lx;
    uint8_t ly;
    uint8_t rx;
    uint8_t ry;
    uint8_t l_trigger;
    uint8_t r_trigger;
    uint8_t chk_sum;
  } data;   
}GamepadData;

void memncpy(uint8_t* s, uint8_t* t, uint8_t n)
{
  for(int i=0; i<n; i++)
    *(t+i)=*(s+i);
}
