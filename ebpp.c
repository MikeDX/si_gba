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
#include"sample0.h"
#include"sample1.h"
#include"sample2.h"
#include"sample3.h"
#include"sample4.h"
#include"sample5.h"
#include"sample6.h"
#include"sample7.h"
#include"sample8.h"

#define size_t unsigned int
void * memcpy(void * dest,const void *src,size_t count);
void updatescr(void);
int splay=10;
int play_dma(u32* sound_src);
short ops=13000;
int color=255*255*255;
u8 mem[0x10000];
byte dips=0;		//dipswitch
int game=0;		//0=SI 1=SI Deluxe
int running;
int rot=0;
int squish=1;
byte shift1=0;
byte shift2=0;
byte shift3=0;
byte soundon3=0;
byte soundon5=0;
byte ufo=0;
int idle;	
#define uint8 unsigned char;

#define RGB(r,g,b) ((r)+((g)<<5)+((b)<<10))  // macro to convert 3 values [R, G, B] into a single 16 bit int that is used in mode 3, Each Red, Green and Blue is from 0-31 which gives up 32*32*32 or 32, 768 possible colors

u16* BMPOBJData =(u16*)0x6014000; // a pointer to halfway point in charecter memory [cause bitmap modes take up the first half so you only have half the amount of character memory in bitmap modes

void PlotPixel(u8 xtemp, u8 ytemp, u16 c); // Plots a pixel at coordonate x,y with color c
void WaitForVsync(u8 howmany);  // Waits for (howmany) Vsyncs to occur. If you do a WaitFotVsync(60) it would wait one second. usefull in some way
void FillBG(u16 c2); // Fills the Screen with color c2
void CopyOAM(void); // Copys the info in sprites[] to OAM. in this way you first set all your attributes in sprites[X].attributeY and then copy them, DONT FORGET TO COPY THEM AFTER
void ClearOAM(void); // Clears OAM [object attribute memory] use only at start or all object attributes will be lost
void EditColor(void); // brings up the color editing screen. 

void C_Entry(void) // main entry point
{

	u16 temp;
	
	SetMode(MODE_3 | BG2_ENABLE | OBJ_ENABLE | OBJ_MAP_1D); // Sets the screen to mode 3 [a 240*160 array of 15-bit colors [5 bits per R,G,B = 0-31 each]], turns on background 2 [the only one accessible in bitmap mode], turns on sprites, sets sprites access mode to 1d 
	
	for(temp=0;temp<256;temp++) 
	{// loop for 256 times [0-255] 
		OBJPaletteMem[temp] = objectsPalette[temp]; //copys sprite palette into the correct memory location
	PlotPixel(temp/2,temp/2,color); //plots the pixel!
	}

	FillBG(0); // clear the screen

	memcpy(&mem[0x0000],&rom_h,0x800);
    memcpy(&mem[0x0800],&rom_g,0x800);
    memcpy(&mem[0x1000],&rom_f,0x800);
    memcpy(&mem[0x1800],&rom_e,0x800);
    
    
    
    if(mem[0x1800]==0x20){  // test for our rom in the right location
		running=1;
		reset_8080();
		
		while(running) {
			//idle++;
			//PlotPixel(0,0,idle);
		    run_8080(ops);			// = 2MHz / 60Hz / 2
        	int_8080();				// not sure if this is 
        	run_8080(ops);			// accurate or not
        	nmi_8080();				//
        	if ( splay ) {
        		splay--;
        		if ( splay==0 ) {
        			*(u16*)0x4000084 = 0x0000;
        			*(u16*)0x4000102 = 0x0080; 
        		}
        		
        	}
        	
        }
    }
}

void writemem(word addr,byte val) {
	if (addr<0x2000) return;
	mem[addr]=val;
//	PlotPixel(5,5,val);
	if(addr>=0x2400 && addr<0x4000) {

      int xpos,ypos;
 
      addr-=0x2400;
      xpos=addr>>5;
      ypos=240-((addr%32)<<3);
      //ypos=(ypos*19)/20;
      if(ypos<0) ypos=0;
	if(rot) xpos=(xpos*160)/224;
	else ypos=(ypos*160)/240;
if ( rot ) {
      PlotPixel(xpos,ypos, ((val>>7)&1) ? color : 0);
      PlotPixel(xpos,ypos+1, ((val>>6)&1) ? color : 0);
      PlotPixel(xpos,ypos+2, ((val>>5)&1) ? color : 0);
      PlotPixel(xpos,ypos+3, ((val>>4)&1) ? color : 0);
      PlotPixel(xpos,ypos+4, ((val>>3)&1) ? color : 0);
      PlotPixel(xpos,ypos+5, ((val>>2)&1) ? color : 0);
      PlotPixel(xpos,ypos+6, ((val>>1)&1) ? color : 0);
      PlotPixel(xpos,ypos+7, ((val>>0)&1) ? color : 0);
}
else {
	      PlotPixel(xpos,ypos, ((val>>7)&1) ? color : 0);
      PlotPixel(xpos,ypos+1, ((val>>6)&1) ? color : 0);
//      PlotPixel(xpos,ypos+2, ((val>>5)&1) ? color : 0);
      PlotPixel(xpos,ypos+2, ((val>>4)&1) ? color : 0);
      PlotPixel(xpos,ypos+3, ((val>>3)&1) ? color : 0);
      //PlotPixel(xpos,ypos+5, ((val>>2)&1) ? color : 0);
      PlotPixel(xpos,ypos+4, ((val>>1)&1) ? color : 0);
      PlotPixel(xpos,ypos+5, ((val>>0)&1) ? color : 0);
}
  }
}

byte readmem(word addr) {
//	  PlotPixel(5,5,idle);
  return mem[addr];
}

void outport(byte port,byte val) {
  if (port==2) {
    shift1=val&0x07;
  }
  else if (port==3) {
    if (val&01) {
      if (ufo++ == 100) {	// this ugly counter thingy
 play_dma((u32*)&sample0);
 //       sound_effect(0);	// seems to kind-of work for the
        ufo=0;			// repeating UFO sound...
      }				// (someone please improve it)
    }
    if (val&0x02 && ~soundon3&0x02) play_dma((u32*)&sample1); //sound_effect(1);
    if (val&0x04 && ~soundon3&0x04) play_dma((u32*)&sample2); //sound_effect(2);
    if (val&0x08 && ~soundon3&0x08) play_dma((u32*)&sample3); //sound_effect(3);
    soundon3=val;
  }
  else if (port==4) {
    shift2=shift3;
    shift3=val;
  }    
  else if (port==5) {
    if (val&0x01 && ~soundon5&0x01) play_dma((u32*)&sample4);//sound_effect(4);
    if (val&0x02 && ~soundon5&0x02)play_dma((u32*)&sample5) ;//sound_effect(5);
    if (val&0x04 && ~soundon5&0x04) play_dma((u32*)&sample6);//sound_effect(6);
    if (val&0x08 && ~soundon5&0x08)play_dma((u32*)&sample7) ;//sound_effect(7);
    if (val&0x10 && ~soundon5&0x10) play_dma((u32*)&sample8);//sound_effect(8);
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
    
if(!((*KEYS) & KEY_R))                      tmp|=(1<<1);	//2pl
if(!((*KEYS) & KEY_L))                       tmp|=(1<<2);	//1pl
if(!((*KEYS) & KEY_A))        tmp|=(1<<4);	//fire
if(!((*KEYS) & KEY_LEFT)) tmp|=(1<<5);	//left
if(!((*KEYS) & KEY_RIGHT)) tmp|=(1<<6);	//right
if(!((*KEYS) & KEY_UP)) {FillBG(0); rot=1;updatescr(); }
if(!((*KEYS) & KEY_DOWN)) {FillBG(0);rot=0;updatescr(); }
if(!((*KEYS) & KEY_START)) {play_dma((u32*)&sample0); // color++; 
}
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
//	u8 t;
	
	//ytemp=ytemp;
	//swap(xtemp,ytemp);
	//t=xtemp;xtemp=ytemp;ytemp=t;
	if ( rot) {
		xtemp=160-xtemp;
		VideoBuffer[((xtemp)*240)+(ytemp)] = c;
		}
	else VideoBuffer[((ytemp)*240)+(xtemp)] = c;
}

	void * memcpy(void * dest,const void *src,size_t count)
 {
 	char *tmp = (char *) dest, *s = (char *) src;
 	while (count--)
 		*tmp++ = *s++;
 	return dest;
 }
 
 void FillBG(u16 c2)
{
	u8 xtemp,ytemp;
	for(xtemp=0;xtemp<240;xtemp++)
		for(ytemp=0;ytemp<160;ytemp++)
			VideoBuffer[((ytemp)*240)+(xtemp)] = c2;
}


void updatescr(void){
	int temp;
	for(temp=0x2400;temp<0x4000;temp++)
	writemem(temp,mem[temp]);
}


int play_dma(u32* sound_src) 
{ 
	splay=10;
 *(u16*)0x4000082 = 0x0b0E;  // sound control channel ( SGCNT0_H )    
*(u16*)0x4000084 = 0x0080; //sound enable ( SGCNT1 )
 *(u32*)0x40000BC =(unsigned long)sound_src;  // the address of the source 
*(u32*)0x40000c0 = 0x040000a0;// Dest DMA LOW&HIGH written (0x40000c0= DM1DAD_L )(0x040000a0=SGFIFOA_L)
*(u16*)0x40000C6 =  0xB640; // DMA Control ( DM1CNT_H )
  *(u16*)0x4000100 = 0xFb82; //timer  = samplingrate :)
    *(u16*)0x4000102 = 0x0080; //timer Control. Ingen IRQ eller nerdelning av CPU clockan.
    return 0;
} 
