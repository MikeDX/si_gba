#include "gba.h"				 // eloist's GBA header with many defines
#include "keypad.h"				 // dovoto's keypad defines
#include "screenmode.h"			 // dovoto's screen mode defines
#define NULL 0

//#include "sprite.h"     // dovoto's sprite defines
//#include "objects.h" //for palette
#include "mz80.h"
#include "cpu8080.h"			 // header for POS8080Emu
#include "rom_e.h"				 // game rom data
#include "rom_f.h"				 // converted with
#include "rom_g.h"				 // bin2c.exe
#include "rom_h.h"				 // (a really handy program)
#include "rom2_d.h"				 //
#include "rom2_e.h"				 //
#include "rom2_f.h"				 //
#include "rom2_g.h"				 //
#include "rom2_h.h"				 //
//#include "i8291105.h"
#include "spaceinv-emu.h"
#include "invdpt2m.h"
#include "e3dtitle.h"
#include"sample0.h"
#include"sample1.h"
#include"sample2.h"
#include"sample3.h"
#include"sample4.h"
#include"sample5.h"
#include"sample6.h"
#include"sample7.h"
#include"sample8.h"
int xloop,yloop;
#define size_t unsigned int
void * memcpy(void * dest,const void *src,size_t count);
void *memset(void *dest,const byte src, size_t count);
void updatescr(void);
int splay=10;
void gfxPixel (u8 x, u8 y, u16 color, u32 addr);
int play_dma(u32* sound_src,int sc);
short ops=12500;
int color=250<<10 | 255<<5 | 250;
u8 mem[0x10000];
byte dips=0;					 //dipswitch
int game=0;						 //0=SI 1=SI Deluxe
int running;
int rot=0;
int squish=1;
int heh;
byte shift1=0;
byte shift2=0;
byte shift3=0;
byte soundon3=0;
byte soundon5=0;
byte ufo=0;
int idle;
#define uint8 unsigned char;

								 // macro to convert 3 values [R, G, B] into a single 16 bit int that is used in mode 3, Each Red, Green and Blue is from 0-31 which gives up 32*32*32 or 32, 768 possible colors
#define RGB(r,g,b) ((r)+((g)<<5)+((b)<<10))

u16* BMPOBJData =(u16*)0x6014000;// a pointer to halfway point in charecter memory [cause bitmap modes take up the first half so you only have half the amount of character memory in bitmap modes

//void PlotPixel(u8 xtemp, u8 ytemp, u16 c); // Plots a pixel at coordonate x,y with color c
//#define PlotPixel(x,y,c) gba_setpixel((u8)x,(u8)y,(u16)c);
#define PlotPixel(x,y,c) gfxPixel(x,y,c,0x6000000);
void WaitForVsync(u8 howmany);	 // Waits for (howmany) Vsyncs to occur. If you do a WaitFotVsync(60) it would wait one second. usefull in some way
void FillBG(u16 c2);			 // Fills the Screen with color c2
void CopyOAM(void);				 // Copys the info in sprites[] to OAM. in this way you first set all your attributes in sprites[X].attributeY and then copy them, DONT FORGET TO COPY THEM AFTER
void ClearOAM(void);			 // Clears OAM [object attribute memory] use only at start or all object attributes will be lost

CONTEXTMZ80 z80cpu;
UINT32 dwResult = 0x80000000;
UINT32 dwIntTotal=0;
UINT32 dwDisplayInterval=0;
UINT32 dwFrames = 0;
void Interrupt (void);
int nmiv=0;

struct MemoryReadByte sInvRead[] =
{
	{0,0x10000,readmem},
	{-1,            -1,     NULL}
};

struct z80PortWrite sInvIOWrite[] =
{
	{0, 6, outport},
	{-1,            -1,     NULL}
};

struct z80PortRead sInvIORead[] =
{
	{0, 3, inport},
	{-1, -1, NULL}
};

struct MemoryWriteByte sInvWrite[] =
{
	//	{0x0000,    0x1fff, romwrite},
	//	{0x2000,	0x23ff, InvRamWrite},
	{0x0000,    0xffff, writemem},
	{-1,            -1,     NULL}
};

void C_Entry(void)				 // main entry point
{

	//	u16 temp;
	if(!((*KEYS) & KEY_A)) game=1;
								 // Sets the screen to mode 3 [a 240*160 array of 15-bit colors [5 bits per R,G,B = 0-31 each]], turns on background 2 [the only one accessible in bitmap mode], turns on sprites, sets sprites access mode to 1d
	SetMode(MODE_3 | BG2_ENABLE | OBJ_ENABLE | OBJ_MAP_1D);
	heh=0;
								 //loads the data from coloredit.h onto the screen
	for(xloop=0;xloop<240;xloop++)
		for(yloop=0;yloop<160;yloop++)
	{
		heh++;
		PlotPixel(xloop,yloop,titleBitmap[yloop*240+xloop]);
	}							 //VideoBuffer[yloop*240+xloop] = titleBitmap[yloop*240+xloop];

	game=2;
	while(game==2)
	{
		if(!((*KEYS) & KEY_A)) game=1;
		if(!((*KEYS) & KEY_B)) game=0;
	}
	z80cpu.z80Base = mem;		 // This allocates our virtual 64k of workspace for our z80 cpu
								 // And this clears it to zero, as it might contain garbage
	//memset(z80cpu.z80Base, 0, 0x10000);

	/*	for(temp=0;temp<256;temp++)
		{// loop for 256 times [0-255]
			OBJPaletteMem[temp] = objectsPalette[temp]; //copys sprite palette into the correct memory location
	//	PlotPixel(temp/2,temp/2,color); //plots the pixel!
		} */

	//FillBG(0); // clear the screen
								 //loads the data from coloredit.h onto the screen
	for(xloop=0;xloop<240;xloop++)
	{
		for(yloop=0;yloop<160;yloop++)
		{
			if ( game==1) invadersBitmap[yloop*240+xloop]=invdpt2mBitmap[yloop*240+xloop];
			VideoBuffer[yloop*240+xloop] = invadersBitmap[yloop*240+xloop];
		}
	}
	for(xloop=0x2000;xloop<0x4000;xloop++) mem[xloop]=255;
	if ( game==0)
	{
		memcpy(&mem[0x0000],&rom_h,0x800);
		memcpy(&mem[0x0800],&rom_g,0x800);
		memcpy(&mem[0x1000],&rom_f,0x800);
		memcpy(&mem[0x1800],&rom_e,0x800);
	}
	else
	{
		memcpy(&mem[0x0000],&rom2_h,0x800);
		memcpy(&mem[0x0800],&rom2_g,0x800);
		memcpy(&mem[0x1000],&rom2_f,0x800);
		memcpy(&mem[0x1800],&rom2_e,0x800);
		memcpy(&mem[0x4000],&rom2_d,0x800);
	}

	//   if(mem[0x1800]==0x20){  // test for our rom in the right location
	running=1;
	mz80reset();
	z80cpu.z80MemRead = sInvRead;
	z80cpu.z80MemWrite = sInvWrite;
	z80cpu.z80IoRead = sInvIORead;
	z80cpu.z80IoWrite = sInvIOWrite;

	z80cpu.z80intAddr = 0x08;	 // Interrupt address
	z80cpu.z80nmiAddr = 0x10;	 // NMI Address
	mz80SetContext(&z80cpu);
	while (0x80000000 == dwResult && running == 1)
	{
		//idle++;
		//PlotPixel(0,0,idle);
		dwResult = mz80exec(8000);
								 // How much emulated time passed?
		dwIntTotal += mz80GetElapsedTicks(0);
								 // Reset our internal counter
		dwDisplayInterval += mz80GetElapsedTicks(1);
								 // Roughly 60 times a second at 2MHZ
		if (dwDisplayInterval >= 34133)
		{
			// Throttle speed here for screen update..

			++dwFrames;

			dwDisplayInterval -= 34133;
		}

		if (dwIntTotal > 17067)
		{

			// We toggle between an NMI and an Interrupt
			//PlotPixel(0,0,color);
			dwIntTotal -= 17067;
			Interrupt();

		}
		//PlotPixel(0,0,0);
		if(!((*KEYS) & KEY_START))
		{
			play_dma((u32*)&sample0,10);
			splay=10;
		}
		// int_8080();				 // not sure if this is
		if ( splay )
		{
			splay--;
			if ( splay==0 )
			{
				*(u16*)0x4000084 = 0;
				*(u16*)0x4000102 = 0;
				*(u16*)0x40000C6 = 0;
			}

		}
		/* run_8080(ops);			 // accurate or not
		nmi_8080();				 //
		if ( splay )
		{
			splay--;
			if ( splay==0 )
			{
				*(u16*)0x4000084 = 0;
				*(u16*)0x4000102 = 0;
				*(u16*)0x40000C6 = 0;
			}

		}*/

	}
	// }
	FillBG(0);					 // clear the screen
}


void romwrite(UINT32 addr, UINT8 val, struct MemoryWriteByte *psWrite)
{
	//	play_dma((u32*)&sample0,10);
	//splay=10;
}


void writemem(UINT32 addr, UINT8 val, struct MemoryWriteByte *psWrite)
{
	//float yp;
	int xpos,ypos,mpos;
	byte ov;
	//	play_dma((u32*)&sample0,10);
	//	splay=10;
	ov=mem[addr];

	if (addr<0x2000)
		return;					 // ROM
	mem[addr]=val ;
	if(addr>=0x2400 && addr<0x4000 && ov!=val)
	{

		addr-=0x2400;
		xpos=addr>>5;
		ypos=240-((addr%32)<<3);

		if(ypos<0) ypos=0;
		ypos=(ypos*160)/240;

		xpos=xpos+10;

		PlotPixel(xpos,ypos, ((val>>7)&1) ? color : invadersBitmap[mpos=ypos*240+xpos]);
		PlotPixel(xpos,ypos+1, ((val>>6)&1) ? color : invadersBitmap[mpos+=240]);

		PlotPixel(xpos,ypos+2, ((val>>4)&1) ? color : invadersBitmap[mpos+=240]);
		PlotPixel(xpos,ypos+3, ((val>>3)&1) ? color : invadersBitmap[mpos+=240]);

		PlotPixel(xpos,ypos+4, ((val>>1)&1) ? color : invadersBitmap[mpos+=240]);
		PlotPixel(xpos,ypos+5, ((val>>0)&1) ? color : invadersBitmap[mpos+=240]);
	}

}


UINT8 readmem(UINT32 addr, struct MemoryReadByte *pPW)
//byte readmem(word addr)
{
	//PlotPixel(5,5,idle);
	//	play_dma((u32*)&sample0,10);
	//			splay=10;
	return mem[addr];

}


void outport(UINT16 port, UINT8 val, struct z80PortWrite *pPW )
{
	//void outport(byte port,byte val) {
	if (port==2)
	{
		shift1=val&0x07;
	}
	else if (port==3)
	{
		if (val&01)
		{
			if (ufo++ == 100)	 // this ugly counter thingy
			{
				play_dma((u32*)&sample0,6);
				//       sound_effect(0);	// seems to kind-of work for the
				ufo=0;			 // repeating UFO sound...
			}					 // (someone please improve it)
		}
								 // Player fire
		if (val&0x02 && ~soundon3&0x02) play_dma((u32*)&sample1,10);
								 //sound_effect(2);
		if (val&0x04 && ~soundon3&0x04) play_dma((u32*)&sample2,20);
								 //sound_effect(3);
		if (val&0x08 && ~soundon3&0x08) play_dma((u32*)&sample3,15);
		soundon3=val;
	}
	else if (port==4)
	{
		shift2=shift3;
		shift3=val;
	}
	else if (port==5)
	{
								 //sound_effect(4);
		if (val&0x01 && ~soundon5&0x01) play_dma((u32*)&sample4,5);
								 //sound_effect(5);
		if (val&0x02 && ~soundon5&0x02)play_dma((u32*)&sample5,5) ;
								 //sound_effect(6);
		if (val&0x04 && ~soundon5&0x04) play_dma((u32*)&sample6,5);
								 //sound_effect(7);
		if (val&0x08 && ~soundon5&0x08)play_dma((u32*)&sample7,5) ;
								 //sound_effect(8);
		if (val&0x10 && ~soundon5&0x10) play_dma((u32*)&sample8,40);
		soundon5=val;
	}
}


UINT16 inport(UINT16 port, struct z80PortRead *pPR)
{
	//byte inport(byte port) {
	if (port==0)  return (0x74); //(makes SI2 work right; protection?)
	if (port==1)
	{
		byte tmp=0;
		//    unsigned char mcont;
		//    cont_cond_t cond;
		//mcont = maple_controller_addr();
		//cont_get_cond(mcont, &cond);

								 //coin
		if(!((*KEYS) & KEY_SELECT))          tmp|=(1<<0);

								 //2pl
		if(!((*KEYS) & KEY_R))                      tmp|=(1<<1);
								 //1pl
		if(!((*KEYS) & KEY_L))                       tmp|=(1<<2);
								 //fire
		if(!((*KEYS) & KEY_A))        tmp|=(1<<4);
								 //left
		if(!((*KEYS) & KEY_LEFT)) tmp|=(1<<5);
								 //right
		if(!((*KEYS) & KEY_RIGHT)) tmp|=(1<<6);
		//if(!((*KEYS) & KEY_UP)) {FillBG(0); rot=1;updatescr(); }
		//if(!((*KEYS) & KEY_DOWN)) {FillBG(0);rot=0;updatescr(); }
								 // color++;
		if(!((*KEYS) & KEY_START))
		{
			play_dma((u32*)&sample0,10);
		}
		//if(!(cond.buttons & CONT_START))     running=0;
		return tmp;
	}
	else if (port==2)
	{
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


__inline void sPlotPixel(u8 xtemp, u8 ytemp, u16 c)
{
	VideoBuffer[((ytemp)*240)+(xtemp)] = c;
	return;

	//	gba_setpixel(xtemp,ytemp,c);
	//	return;

	//	u8 t;

	//ytemp=ytemp;
	//swap(xtemp,ytemp);
	//t=xtemp;xtemp=ytemp;ytemp=t;
	if ( rot)
	{
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


void *memset(void *dest, byte src, size_t count)
{
	char *tmp = (char *) dest;
	while(count--)
		*tmp++=src;
	return NULL;

}


void FillBG(u16 c2)
{
	u8 xtemp,ytemp;
	for(xtemp=0;xtemp<240;xtemp++)
		for(ytemp=0;ytemp<160;ytemp++)
			VideoBuffer[((ytemp)*240)+(xtemp)] = c2;
}


/* void updatescr(void){
	int temp;
	for(temp=0x2400;temp<0x4000;temp++)
	writemem(temp,mem[temp]);
} */

int play_dma(u32* sound_src,int sc)
{
	splay=sc;
	*(u16*)0x4000084 = 0;
	*(u16*)0x4000102 = 0;
	*(u16*)0x40000C6 = 0;
	*(u16*)0x4000082 = 0x0b0E;	 // sound control channel ( SGCNT0_H )
	*(u16*)0x4000084 = 0x0080;	 //sound enable ( SGCNT1 )
								 // the address of the source
	*(u32*)0x40000BC =(unsigned long)sound_src;
								 // Dest DMA LOW&HIGH written (0x40000c0= DM1DAD_L )(0x040000a0=SGFIFOA_L)
	*(u32*)0x40000c0 = 0x040000a0;
	*(u16*)0x40000C6 =  0xB640;	 // DMA Control ( DM1CNT_H )
	*(u16*)0x4000100 = 0xFb82;	 //timer  = samplingrate :)
	*(u16*)0x4000102 = 0x0080;	 //timer Control. Ingen IRQ eller nerdelning av CPU clockan.
	return 0;
}


void Interrupt (void)
{
	if (nmiv)
		mz80nmi();
	else
		mz80int(0);
	nmiv ^= 1;

}
