#include "gba.h"        // eloist's GBA header with many defines
#include "keypad.h"     // dovoto's keypad defines
#include "screenmode.h" // dovoto's screen mode defines
#include "sprite.h"     // dovoto's sprite defines
#include "objects.h" //for palette
#include "cpu8080.h"	// header for POS8080Emu
#include "rom_e.h"	// game rom data	
#include "rom_f.h"	// converted with
#include "rom_g.h"	// bin2c.exe
#include "rom_h.h"	// (a really handy program)
void * memcpy(void * dest,const void *src,size_t count);
#define size_t unsigned int
char mem[0x8000];
byte dips=0;		//dipswitch
int game=0;		//0=SI 1=SI Deluxe
int running;
int squish=1;
byte shift1=0;
byte shift2=0;
byte shift3=0;
byte soundon3=0;
byte soundon5=0;
byte ufo=0;

#define uint8 unsigned char;

#define RGB(r,g,b) ((r)+((g)<<5)+((b)<<10))  // macro to convert 3 values [R, G, B] into a single 16 bit int that is used in mode 3, Each Red, Green and Blue is from 0-31 which gives up 32*32*32 or 32, 768 possible colors

u16* BMPOBJData =(u16*)0x6014000; // a pointer to halfway point in charecter memory [cause bitmap modes take up the first half so you only have half the amount of character memory in bitmap modes

void PlotPixel(u8 xtemp, u8 ytemp, u16 c); // Plots a pixel at coordonate x,y with color c
void WaitForVsync(u8 howmany);  // Waits for (howmany) Vsyncs to occur. If you do a WaitFotVsync(60) it would wait one second. usefull in some way
void FillBG(u16 c2); // Fills the Screen with color c2
void CopyOAM(void); // Copys the info in sprites[] to OAM. in this way you first set all your attributes in sprites[X].attributeY and then copy them, DONT FORGET TO COPY THEM AFTER
void ClearOAM(void); // Clears OAM [object attribute memory] use only at start or all object attributes will be lost
void EditColor(void); // brings up the color editing screen. 

/*void writemem(word w,byte b){}
	byte readmem(word w){return 1;}
byte inport(byte b){return 1;}
void outport(byte b1,byte b2){}
u8 mem[0x8000];*/

void C_Entry(void)
{
	
	u16 temp;
	SetMode(MODE_3 | BG2_ENABLE | OBJ_ENABLE | OBJ_MAP_1D); // Sets the screen to mode 3 [a 240*160 array of 15-bit colors [5 bits per R,G,B = 0-31 each]], turns on background 2 [the only one accessible in bitmap mode], turns on sprites, sets sprites access mode to 1d 
	for(temp=0;temp<256;temp++) {// loop for 256 times [0-255] 
		OBJPaletteMem[temp] = objectsPalette[temp]; //copys sprite palette into the correct memory location
PlotPixel(temp/2,temp/2,512); //plots the pixel!

}
	memcpy(0x0000,&rom_h,0x800);
    memcpy(0x0800,&rom_g,0x800);
    memcpy(0x1000,&rom_f,0x800);
    memcpy(0x1800,&rom_e,0x800);
  
    
}

void writemem(word addr,byte val) {
  if (addr<0x2000) return;
  mem[addr]=val;
  if(addr>=0x2400 && addr<0x4000) {

      int xpos,ypos;
      int color=0xFFFF;						//white
      addr-=0x2400;
      xpos=addr>>5;
      ypos=240-((addr%32)<<3);
 //     if (squish) ypos=(ypos*19)/20;
      if ((addr%32)<8) color=(0x47E8);				//greenish
      else if ((addr%32)>25 && (addr%32)<28) color=(0xFA04);	//redish
      PlotPixel(xpos,ypos, ((val>>7)&1) ? 255 : 0);
      PlotPixel(xpos,ypos+1, ((val>>6)&1) ? 255 : 0);
      PlotPixel(xpos,ypos+2, ((val>>5)&1) ? 255 : 0);
      PlotPixel(xpos,ypos+3, ((val>>4)&1) ? 255 : 0);
      PlotPixel(xpos,ypos+4, ((val>>3)&1) ? 255 : 0);
      PlotPixel(xpos,ypos+5, ((val>>2)&1) ? 255 : 0);
      PlotPixel(xpos,ypos+6, ((val>>1)&1) ? 255 : 0);
      PlotPixel(xpos,ypos+7, ((val>>0)&1) ? 255 : 0);

  }
}

byte readmem(word addr) {
  return mem[addr];
}

void outport(byte port,byte val) {
  if (port==2) {
    shift1=val&0x07;
  }
  else if (port==3) {
    if (val&01) {
      if (ufo++ == 100) {	// this ugly counter thingy
 //       sound_effect(0);	// seems to kind-of work for the
        ufo=0;			// repeating UFO sound...
      }				// (someone please improve it)
    }
//    if (val&0x02 && ~soundon3&0x02) sound_effect(1);
    //if (val&0x04 && ~soundon3&0x04) sound_effect(2);
    //if (val&0x08 && ~soundon3&0x08) sound_effect(3);
    soundon3=val;
  }
  else if (port==4) {
    shift2=shift3;
    shift3=val;
  }    
  else if (port==5) {
//    if (val&0x01 && ~soundon5&0x01) sound_effect(4);
    //if (val&0x02 && ~soundon5&0x02) sound_effect(5);
    //if (val&0x04 && ~soundon5&0x04) sound_effect(6);
    //if (val&0x08 && ~soundon5&0x08) sound_effect(7);
    //if (val&0x10 && ~soundon5&0x10) sound_effect(8);
    soundon5=val;
  }
}

byte inport(byte port) {
  if (port==0)  return (0x74);  //(makes SI2 work right; protection?)
  if (port==1) {
    byte tmp=0;
//    unsigned char mcont;
//    cont_cond_t cond;
    //mcont = maple_controller_addr();
    //cont_get_cond(mcont, &cond);
    
    
    if(!((*KEYS) & KEY_SELECT))          tmp|=(1<<0);	//coin
    
if(!((*KEYS) & KEY_RIGHT))                      tmp|=(1<<1);	//2pl
//    if(cond.ltrig)                       tmp|=(1<<2);	//1pl
//    if(!(cond.buttons & CONT_A))         tmp|=(1<<4);	//fire
//    if(!(cond.buttons & CONT_DPAD_LEFT)) tmp|=(1<<5);	//left
//    if(!(cond.buttons & CONT_DPAD_RIGHT))tmp|=(1<<6);	//right
//    if(!(cond.buttons & CONT_B))         tmp|=(1<<6);	//right
    //if(!(cond.buttons & CONT_START))     running=0;
    return tmp;
  }
  else if (port==2) {
    byte tmp=dips;
/*    uint8 mcont;
    cont_cond_t cond;
    mcont = maple_controller_addr();
    cont_get_cond(mcont, &cond);
    if(!(cond.buttons & CONT_Y))         tmp|=(1<<2);	//tilt
    if(!(cond.buttons & CONT_A))         tmp|=(1<<4);	//fire2
    if(!(cond.buttons & CONT_DPAD_LEFT)) tmp|=(1<<5);	//left2
    if(!(cond.buttons & CONT_DPAD_RIGHT))tmp|=(1<<6);	//right2
    if(!(cond.buttons & CONT_START))     running=0;*/
    return tmp;
  }
  else if (port==3)
    return(((((shift3<<8)+shift2)<<shift1)>>8)&0xFF);
  return(0);
}
		
void PlotPixel(u8 xtemp, u8 ytemp, u16 c)
{
	VideoBuffer[((ytemp)*240)+(xtemp)] = c;
}

	void * memcpy(void * dest,const void *src,size_t count)
 {
 	char *tmp = (char *) dest, *s = (char *) src;
 	while (count--)
 		*tmp++ = *s++;
 	return dest;
 }