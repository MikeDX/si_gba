////////////////////////////////////////////////////////////////////////////////
// POS (Portable Open Source) 8080 Emulator
// v0.3 by D. Finck
// (C) 2001 by Darren Finck (dgfinck@yahoo.com)
// 
// The program is distributed under the terms of the GNU General Public License
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// 
////////////////////////////////////////////////////////////////////////////////
// Status:
//    -'most' opcodes implemented
//    -incomplete support of half-carry flag
//    -no evaluation of parity flag :(
//    -seems to run Space Invaders and SI Part II OK :)
// To Do:
//    -fix bugs
//    -implement remaining opcodes
//    -finish evaluation half-carry flag
//    -evaluate parity flag
//    -test with other 8080 software
// For Someone Else To Do: 
//    -extend it to a Z80 emulator
////////////////////////////////////////////////////////////////////////////////

#include "cpu8080.h"

extern char mem[0x10000];       //memory
word pc=0x0000;         //program counter
word sp=0x0000;         //stack pointer
byte a =0x00;           //accumulator
byte b =0x00;           //b register
byte c =0x00;           //c register
byte d =0x00;           //d register
byte e =0x00;           //e register
byte h =0x00;           //h register
byte l =0x00;           //l register
int fz =0;              //flag, zero
int fc =0;              //flag, carry
int fs =0;              //flag, sign
int fn =0;              //flag, subtract
int fh =0;              //flag, half-carry
int fv =0;              //flag, parity/overflow
int iff=0;              //interrupt flip-flop

void reset_8080() {
  pc=0x0000;sp=0x0000;
  a=0x00;
  b=0x00;c=0x00;
  d=0x00;e=0x00;
  h=0x00;l=0x00;
  fz=0;fc=0;fs=0;fn=0;fh=0;fv=0;
  iff=0;
}

void int_8080() {
  if (!iff) return;
  sp-=2;
  mem[sp]   = pc&0xFF;
  mem[sp+1] = pc>>8;
  pc=0x0008;
}

void nmi_8080() {
  sp-=2;
  mem[sp]   = pc&0xFF;
  mem[sp+1] = pc>>8;
  pc=0x0010;
}

void run_8080(int cycles) {
  int clk=0;			//clock cycle counter
  while(clk<cycles)  {  	
   byte opcode=mem[pc];		//fetch opcode

   int temp;			//variable for temporary use

   switch(opcode) {		//execute based on opcode

    case 0x00:                  //nop
      pc++;
      clk+=4;
    break;

    case 0x01:                  //ld bc,word
      b=mem[pc+2];
      c=mem[pc+1];
      pc+=3;
      clk+=10;
    break;

    case 0x02:                  //ld (bc),a
      writemem((b<<8)+c,a);
      pc++;
      clk+=7;
    break;

    case 0x03:                  //inc bc
      c++;
      if(c==0) b++;
      pc++;
      clk+=6;
    break;

    case 0x04:                  //inc b
      b++;
      fz=0;fn=0;
      if (b==0) fz=1;
      fs=(b>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0x05:                  //dec b
      b--;
      if(b==0) fz=1;
      else fz=0;
      fn=1;
      fs=(b>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0x06:                  //ld b,byte
      b=mem[pc+1];
      pc+=2;
      clk+=7;
    break;

    case 0x07:                  //rlca
      fc=(a>>7)&1;
      a=(a<<1)+fc;
      fn=0;fh=0;
      pc++;
      clk+=4;
    break;

//  case 0x08: invalid 8080 opcode

    case 0x09:                  //add hl,bc
      temp=(h<<8)+l+(b<<8)+c;
      if (temp>0xFFFF) fc=1;
      else fc=0;
      l=temp&0xFF;
      h=(temp>>8)&0xFF;
      fn=0;
      pc++;
      clk+=11;
    break;

    case 0x0A:                  //ld a,(bc)
      a=readmem((b<<8)+c);
      pc++;
      clk+=7;
    break;

    case 0x0B:                  //dec bc
      c--;
      if (c==0xFF) b--;
      pc++;
      clk+=6;
    break;

    case 0x0C:                  //inc c
      c++;
      fz=0;fn=0;
      if (c==0) fz=1;
      fs=(c>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0x0D:                  //dec c
      c--;
      fz=0;fn=1;
      if(c==0) fz=1;
      fs=(c>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0x0E:                  //ld c,byte
      c=mem[pc+1];
      pc+=2;
      clk+=7;
    break;

    case 0x0F:                  //rrca
      fc=a&1;
      a=(a>>1)+(fc<<7);
      fn=0;fh=0;
      pc++;
      clk+=4;
    break;

//  case 0x10: invalid 8080 opcode

    case 0x11:                  //ld de,word
      d=mem[pc+2];
      e=mem[pc+1];
      pc+=3;
      clk+=10;
    break;

    case 0x12:                  //ld (de),a
      writemem((d<<8)+e,a);
      pc++;
      clk+=7;
    break;

    case 0x13:                  //inc de
      e++;
      if(e==0) d++;
      pc++;
      clk+=6;
    break;

    case 0x14:                  //inc d
      d++;
      fz=0;fn=0;
      if (d==0) fz=1;
      fs=(d>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0x15:                  //dec d
      d--;
      fz=0;fn=1;
      if(d==0) fz=1;
      fs=(d>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0x16:                  //ld d,byte
      d=mem[pc+1];
      pc+=2;
      clk+=7;
    break;

    case 0x17:                  //rla
      temp=fc;
      fc=(a>>7)&1;
      a=(a<<1)|temp;
      fn=0;fh=0;
      pc++;
      clk+=4;
    break;

//  case 0x18: invalid 8080 opcode

    case 0x19:                  //add hl,de
      temp=((h<<8)+l)+((d<<8)+e);
      if (temp>0xFFFF) fc=1;
      else fc=0;
      l=temp & 0xFF;
      h=(temp>>8) & 0xFF;
      fn=0;
      pc++;
      clk+=11;
    break;

    case 0x1A:                  //ld a,(de)
      a=readmem((d<<8)+e);
      pc++;
      clk+=7;
    break;

    case 0x1B:                  //dec de
      e--;
      if (e==0xFF) d--;
      pc++;
      clk+=6;
    break;

    case 0x1C:                  //inc e
      e++;
      fz=0;fn=0;
      if (e==0) fz=1;
      fs=(e>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0x1D:                  //dec e
      e--;
      fz=0;fn=1;
      if (e==0) fz=1;
      fs=(e>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0x1E:                  //ld e,byte
      e=mem[pc+1];              
      pc+=2;
      clk+=7;
    break;

    case 0x1F:                  //rra
      temp=fc;
      fc=a&1;
      a=(a>>1)|(temp<<7);
      fn=0;fh=0;
      pc++;
      clk+=4;
    break;

//  case 0x20:                  //rim
//    not implemented yet
//    don't know what it does...
//  break;

    case 0x21:                  //ld hl,word
      h=mem[pc+2];
      l=mem[pc+1];
      pc+=3;
      clk+=10;
    break;

    case 0x22:                  //ld (word),hl
      writemem((mem[pc+2]<<8)+mem[pc+1],l);
      writemem((mem[pc+2]<<8)+mem[pc+1]+1,h);
      pc+=3;
      clk+=16;
    break;

    case 0x23:                  //inc hl
      l++;
      if (l==0) h++;
      pc++;
      clk+=6;
    break;

    case 0x24:                  //inc h
      h++;
      fz=0;fn=0;
      if (h==0) fz=1;
      fs=(h>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0x25:                  //dec h
      h--;
      fz=0;fn=1;
      if(h==0) fz=1;
      fs=(h>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0x26:                  //ld h,byte
      h=mem[pc+1];              
      pc+=2;
      clk+=7;
    break;

    case 0x27:                  //daa
				//only partially implemented...
      if (fn) {
	if (fc) {
	  if (fh) {
	  }
	  else {
	  }
	}
	else {
	  if (fh) {
	  }
	  else {
	  }
	}
      }
      else {
	if (fc) {
	  if (fh) {
	    if ((a&15)<4 && (a>>4)<4) a+=0x66;
	  }
	  else {
	    if ((a>>4)<4) {
	      if ((a&15)>9) a+=0x66;
	      else a+=0x60;
	    }
	  }
	}
	else {
	  if (fh) {
	    if ((a&15) < 4) {
	      if ((a>>4) > 9) {
		a+=0x66;
		fc=1;
	      }
	      else a+=0x06;
	    }
	  }
	  else {
	    if ((a&15) > 9) {
	      if ((a>>4) > 8) {
		a+=0x66;
		fc=1;
	      }
	      else a+=0x06;
	    }
	    else {
	      if ((a>>4) > 9) {
		a+=0x60;
		fc=1;
	      }
	    }
	  }
	} 
      }
      pc++;
      clk+=4;
    break;

//  case 0x28: invalid 8080 opcode

    case 0x29:                  //add hl,hl
      temp=((h<<8)+l)*2;
      if (temp>0xFFFF) fc=1;
      else fc=0;
      l= temp & 0xFF;
      h=(temp>>8) & 0xFF;
      fn=0;
      pc++;
      clk+=11;
    break;

    case 0x2A:                  //ld hl,(word)
      l=readmem((mem[pc+2]<<8)+mem[pc+1]);
      h=readmem((mem[pc+2]<<8)+mem[pc+1]+1);
      pc+=3;
      clk+=16;
    break;

    case 0x2B:                  //dec hl
      l--;
      if (l==0xFF) h--;
      pc++;
      clk+=6;
    break;

    case 0x2C:                  //inc l
      l++;
      fz=0;fn=0;
      if (l==0) fz=1;
      fs=(l>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0x2D:                  //dec l
      l--;
      fz=0;fn=1;
      if(l==0) fz=1;
      fs=(l>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0x2E:                  //ld l,byte
      l=mem[pc+1];
      pc+=2;
      clk+=7;
    break;

    case 0x2F:                  //cpl
      a=~a;
      fh=1;fn=1;
      pc++;
      clk+=4;
    break;

//  case 0x30:                  //sim
//    not implemented yet
//    don't know what it does...
//  break;

    case 0x31:                  //ld sp,word
      sp=(mem[pc+2]<<8)+mem[pc+1];
      pc+=3;
      clk+=10;
    break;

    case 0x32:                  //ld (word),a
      writemem((mem[pc+2]<<8)+mem[pc+1],a);
      pc+=3;
      clk+=13;
    break;

    case 0x33:                  //inc sp
      sp++;
      pc++;
      clk+=6;
    break;

    case 0x34:                  //inc (hl)
      temp=readmem((h<<8)+l);
      temp++;
      if (temp>0xFF)
	{temp=0;fz=1;}
      else fz=0;
      writemem((h<<8)+l,temp);
      fs=(temp>>7)&1;
      pc++;
      clk+=11;
    break;

    case 0x35:                  //dec (hl)
      temp=readmem((h<<8)+l);
      temp--;
      fz=0;
      if (temp==0) fz=1;
      else if (temp<0) temp=0xFF;
      writemem((h<<8)+l,temp);
      pc++;
      clk+=11;
    break;

    case 0x36:                  //ld (hl),byte
      writemem((h<<8)+l,mem[pc+1]);
      pc+=2;
      clk+=10;
    break;

    case 0x37:                  //scf
      fc=1;fn=0;fh=0;
      pc++;
      clk+=4;
    break;

//  case 0x38: invalid 8080 opcode

//  case 0x39:                  //add hl,sp
//    not implemented yet     
//    pc++;
//  break;

    case 0x3A:                  //ld a,(word)
      a=readmem((mem[pc+2]<<8)+mem[pc+1]);
      pc+=3;
      clk+=13;
    break;

    case 0x3B:                  //dec sp
      sp--;
      pc++;
      clk+=6;
    break;

    case 0x3C:                  //inc a
      a++;
      fz=0;
      if (a==0) fz=1;
      pc++;
      clk+=4;
    break;

    case 0x3D:                  //dec a
      a--;
      fz=0;
      if (a==0) fz=1;
      pc++;
      clk+=4;
    break;

    case 0x3E:                  //ld a,byte
      a=mem[pc+1];
      pc+=2;
      clk+=7;
    break;

    case 0x3F:                  //ccf
      fc=~fc;
      fn=0;
      pc++;
      clk+=4;
    break;

//  case 0x40:                  //ld b,b
//  not implemented, opcode seems silly
//    pc++;
//  break;

    case 0x41:                  //ld b,c
      b=c;
      pc++;
      clk+=4;
    break;

    case 0x42:                  //ld b,d
      b=d;
      pc++;
      clk+=4;
    break;

    case 0x43:                  //ld b,e
      b=e;
      pc++;
      clk+=4;
    break;

    case 0x44:                  //ld b,h
      b=h;
      pc++;
      clk+=4;
    break;

    case 0x45:                  //ld b,l
      b=l;
      pc++;
      clk+=4;
    break;

    case 0x46:                  //ld b,(hl)
      b=readmem((h<<8)+l);
      pc++;
      clk+=7;
    break;

    case 0x47:                  //ld b,a
      b=a;
      pc++;
      clk+=4;
    break;

    case 0x48:                  //ld c,b
      c=b;
      pc++;
      clk+=4;
    break;

//  case 0x49:                  //ld c,c
//    not implemented
//    opcode seems silly  
//    pc++;
//  break;  

    case 0x4A:                  //ld c,d
      c=d;
      pc++;
      clk+=4;
    break;

    case 0x4B:                  //ld c,e
      c=e;
      pc++;
      clk+=4;
    break;

    case 0x4C:                  //ld c,h
      c=h;
      pc++;
      clk+=4;
    break;

    case 0x4D:                  //ld c,l
      c=l;
      pc++;
      clk+=4;
    break;

    case 0x4E:                  //ld c,(hl)
      c=readmem((h<<8)+l);
      pc++;
      clk+=7;
    break;

    case 0x4F:                  //ld c,a
      c=a;
      pc++;
      clk+=4;
    break;

    case 0x50:                  //ld d,b
      d=b;
      pc++;
      clk+=4;
    break;

    case 0x51:                  //ld d,c
      d=c;
      pc++;
      clk+=4;
    break;

//  case 0x52:                  //ld d,d
//    not implemented
//    opcode seems silly
//    pc++;
//  break;

    case 0x53:                  //ld d,e
      d=e;
      pc++;
      clk+=4;
    break;

    case 0x54:                  //ld d,h
      d=h;
      pc++;
      clk+=4;
    break;

    case 0x55:                  //ld d,l
      d=l;
      pc++;
      clk+=4;
    break;

    case 0x56:                  //ld d,(hl)
      d=readmem((h<<8)+l);
      pc++;
      clk+=7;
    break;

    case 0x57:                  //ld d,a
      d=a;
      pc++;
      clk+=4;
    break;

    case 0x58:                  //ld e,b
      e=b;
      pc++;
      clk+=4;
    break;

    case 0x59:                  //ld e,c
      e=c;
      pc++;
      clk+=4;
    break;

    case 0x5A:                  //ld e,d
      e=d;
      pc++;
      clk+=4;
    break;

//  case 0x5B:                  //ld e,e
//    not implemented
//    opcode seems silly
//    pc++;
//  break;

    case 0x5C:                  //ld e,h
      e=h;
      pc++;
      clk+=4;
    break;

    case 0x5D:                  //ld e,l
      e=l;
      pc++;
      clk+=4;
    break;

    case 0x5E:                  //ld e,(hl)
      e=readmem((h<<8)+l);
      pc++;
      clk+=7;
    break;

    case 0x5F:                  //ld e,a
      e=a;
      pc++;
      clk+=4;
    break;

    case 0x60:                  //ld h,b
      h=b;
      pc++;
      clk+=4;
    break;

    case 0x61:                  //ld h,c
      h=c;
      pc++;
      clk+=4;
    break;

    case 0x62:                  //ld h,d
      h=d;
      pc++;
      clk+=4;
    break;

    case 0x63:                  //ld h,e
      h=e;
      pc++;
      clk+=4;
    break;

//  case 0x64:                  //ld h,h
//    not implemented
//    opcode seems silly
//    pc++;
//  break;

    case 0x65:                  //ld h,l
      h=l;
      pc++;
      clk+=4;
    break;

    case 0x66:                  //ld h,(hl)
      h=readmem((h<<8)+l);
      pc++;
      clk+=7;
    break;

    case 0x67:                  //ld h,a
      h=a;
      pc++;
      clk+=4;
    break;

    case 0x68:                  //ld l,b
      l=b;
      pc++;
      clk+=4;
    break;

    case 0x69:                  //ld l,c
      l=c;
      pc++;
      clk+=4;
    break;

    case 0x6A:                  //ld l,d
      l=d;
      pc++;
      clk+=4;
    break;

    case 0x6B:                  //ld l,e
      l=e;
      pc++;
      clk+=4;
    break;

    case 0x6C:                  //ld l,h
      l=h;
      pc++;
      clk+=4;
    break;

//  case 0x6D:                  //ld l,l
//    not implemented
//    opcode seems silly
//    pc++;
//  break;

    case 0x6E:                  //ld l,(hl)
      l=readmem((h<<8)+l);
      pc++;
      clk+=7;
    break;

    case 0x6F:                  //ld l,a
      l=a;
      pc++;
      clk+=4;
    break;

    case 0x70:                  //ld (hl),b
      writemem((h<<8)+l,b);
      pc++;
      clk+=7;
    break;

    case 0x71:                  //ld (hl),c
      writemem((h<<8)+l,c);
      pc++;
      clk+=7;
    break;

    case 0x72:                  //ld (hl),d
      writemem((h<<8)+l,d);
      pc++;
      clk+=7;
    break;

    case 0x73:                  //ld (hl),e
      writemem((h<<8)+l,e);
      pc++;
      clk+=7;
    break;

    case 0x74:                  //ld (hl),h
      writemem((h<<8)+l,h);
      pc++;
      clk+=7;
    break;

    case 0x75:                  //ld (hl),l
      writemem((h<<8)+l,l);
      pc++;
      clk+=7;
    break;

//  case 0x76:                  //halt
//    not implemented yet
//  break;

    case 0x77:                  //ld (hl),a
      writemem((h<<8)+l,a);
      pc++;
      clk+=7;
    break;

    case 0x78:                  //ld a,b
      a=b;
      pc++;
      clk+=4;
    break;

    case 0x79:                  //ld a,c
      a=c;
      pc++;
      clk+=4;
    break;

    case 0x7A:                  //ld a,d
      a=d;
      pc++;
      clk+=4;
    break;

    case 0x7B:                  //ld a,e
      a=e;
      pc++;
      clk+=4;
    break;

    case 0x7C:                  //ld a,h
      a=h;
      pc++;
      clk+=4;
    break;

    case 0x7D:                  //ld a,l
      a=l;
      pc++;
      clk+=4;
    break;

    case 0x7E:                  //ld a,(hl)
      a=readmem((h<<8)+l);
      pc++;
      clk+=7;
    break;

//  case 0x7F:                  //ld a,a
//    not implemented
//    opcode seems silly
//    pc++
//  break;  

    case 0x80:                  //add a,b
      temp=a+b;
      if ((a&0x0F)+(b&0x0F)>0x0F) fh=1;
      else fh=0;
      if (temp>0xFF) fc=1;
      else fc=0;
      a=temp&0xFF;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      fn=0;
      pc++;
      clk+=4;
    break;

    case 0x81:                  //add a,c
      temp=a+c;
      if ((a&0x0F)+(c&0x0F)>0x0F) fh=1;
      else fh=0;
      if (temp>0xFF) fc=1;
      else fc=0;
      a=temp&0xFF;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0x82:                  //add a,d
      temp=a+d;
      if ((a&0x0F)+(d&0x0F)>0x0F) fh=1;
      else fh=0;
      if (temp>0xFF) fc=1;
      else fc=0;
      a=temp&0xFF;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0x83:                  //add a,e
      temp=a+e;
      if ((a&0x0F)+(e&0x0F)>0x0F) fh=1;
      else fh=0;
      if (temp>0xFF) fc=1;
      else fc=0;
      a=temp&0xFF;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      fn=0;
      pc++;
      clk+=4;
    break;

    case 0x84:                  //add a,h
      temp=a+h;
      if ((a&0x0F)+(h&0x0F)>0x0F) fh=1;
      else fh=0;
      if (temp>0xFF) fc=1;
      else fc=0;
      a=temp&0xFF;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0x85:                  //add a,l
      temp=a+l;
      if ((a&0x0F)+(l&0x0F)>0x0F) fh=1;
      else fh=0;
      if (temp>0xFF) fc=1;
      else fc=0;
      a=temp&0xFF;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0x86:                  //add a,(hl)
      temp=a+readmem((h<<8)+l);
      if ((a&0x0F)+(readmem((h<<8)+l)&0x0F)>0x0F) fh=1;
      else fh=0;
      if (temp>0xFF) fc=1;
      else fc=0;
      a=temp&0xFF;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      pc++;
      clk+=7;
    break;

    case 0x87:                  //add a,a
      temp=a+a;
      if (a&0x08) fh=1;
      else fh=0;
      if (temp>0xFF) fc=1;
      else fc=0;
      a=temp&0xFF;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0x88:                  //adc a,b
      temp=a+b+fc;
      if ((a&0x0F)+((b+fc)&0x0F)>0x0F) fh=1;
      else fh=0;
      if (temp>0xFF) fc=1;
      else fc=0;
      a=temp&0xFF;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0x89:                  //adc a,c
      temp=a+c+fc;
      if ((a&0x0F)+((c+fc)&0x0F)>0x0F) fh=1;
      else fh=0;
      if (temp>0xFF) fc=1;
      else fc=0;
      a=temp&0xFF;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0x8A:                  //adc a,d
      temp=a+d+fc;
      if ((a&0x0F)+((d+fc)&0x0F)>0x0F) fh=1;
      else fh=0;
      if (temp>0xFF) fc=1;
      else fc=0;
      a=temp&0xFF;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      fn=0;
      pc++;
      clk+=4;
    break;

    case 0x8B:                  //adc a,e
      temp=a+e+fc;
      if ((a&0x0F)+((e+fc)&0x0F)>0x0F) fh=1;
      else fh=0;
      if (temp>0xFF) fc=1;
      else fc=0;
      a=temp&0xFF;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0x8C:                  //adc a,h
      temp=a+h+fc;
      if ((a&0x0F)+((h+fc)&0x0F)>0x0F) fh=1;
      else fh=0;
      if (temp>0xFF) fc=1;
      else fc=0;
      a=temp&0xFF;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0x8D:                  //adc a,l
      temp=a+l+fc;
      if ((a&0x0F)+((l+fc)&0x0F)>0x0F) fh=1;
      else fh=0;
      if (temp>0xFF) fc=1;
      else fc=0;
      a=temp&0xFF;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0x8E:                  //adc a,(hl)
      temp=a+readmem((h<<8)+l)+fc;
      if ((a&0x0F)+((readmem((h<<8)+l)+fc)&0x0F)>0x0F) fh=1;
      else fh=0;
      if (temp>0xFF) fc=1;
      else fc=0;
      a=temp&0xFF;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      pc++;
      clk+=7;
    break;

    case 0x8F:                  //adc a,a
      temp=a+a+fc;
      if ((a&0x0F)+((a+fc)&0x0F)>0x0F) fh=1;
      else fh=0;
      if (temp>0xFF) fc=1;
      else fc=0;
      a=temp&0xFF;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0x90:                  //sub b
      temp=a-b;
      if (temp<0) fc=1;
      else fc=0;
      a=a-b;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      fn=1;
      pc++;
      clk+=4;
    break;

    case 0x91:                  //sub c
      temp=a-c;
      if (temp<0) fc=1;
      else fc=0;
      a=a-c;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      fn=1;
      pc++;
      clk+=4;
    break;

    case 0x92:                  //sub d
      temp=a-d;
      if (temp<0) fc=1;
      else fc=0;
      a=a-d;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      fn=1;
      pc++;
      clk+=4;
    break;

    case 0x93:                  //sub e
      temp=a-e;
      if (temp<0) fc=1;
      else fc=0;
      a=a-e;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      fn=1;
      pc++;
      clk+=4;
    break;

    case 0x94:                  //sub h
      temp=a-h;
      if (temp<0) fc=1;
      else fc=0;
      a=a-h;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      fn=1;
      pc++;
      clk+=4;
    break;

    case 0x95:                  //sub l
      temp=a-l;
      if (temp<0) fc=1;
      else fc=0;
      a=a-l;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      fn=1;
      pc++;
      clk+=4;
    break;

    case 0x96:                  //sub (hl)
      temp=a-readmem((h<<8)+l);
      if (temp<0) fc=1;
      else fc=0;
      a=a-readmem((h<<8)+l);
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      fn=1;
      pc++;
      clk+=7;
    break;

    case 0x97:                  //sub a
      a=0;
      fc=0;fz=1;fs=0;fn=1;
      pc++;
      clk+=4;
    break;

    case 0x98:                  //sbc b
      temp=a-b-fc;
      a=a-b-fc;
      if (temp<0) fc=1;
      else fc=0;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      fn=1;
      pc++;
      clk+=4;
    break;

    case 0x99:                  //sbc c
      temp=a-c-fc;
      a=a-c-fc;
      if (temp<0) fc=1;
      else fc=0;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      fn=1;
      pc++;
      clk+=4;
    break;

    case 0x9A:                  //sbc d
      temp=a-d-fc;
      a=a-d-fc;
      if (temp<0) fc=1;
      else fc=0;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      fn=1;
      pc++;
      clk+=4;
    break;

    case 0x9B:                  //sbc e
      temp=a-e-fc;
      a=a-e-fc;
      if (temp<0) fc=1;
      else fc=0;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      fn=1;
      pc++;
      clk+=4;
    break;

    case 0x9C:                  //sbc h
      temp=a-h-fc;
      a=a-h-fc;
      if (temp<0) fc=1;
      else fc=0;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      fn=1;
      pc++;
      clk+=4;
    break;

    case 0x9D:                  //sbc l
      temp=a-l-fc;
      a=a-l-fc;
      if (temp<0) fc=1;
      else fc=0;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      fn=1;
      pc++;
      clk+=4;
    break;

    case 0x9E:                  //sbc (hl)
      temp=a-readmem((h<<8)+l)-fc;
      a=a-readmem((h<<8)+l)-fc;
      if (temp<0) fc=1;
      else fc=0;
      if (a==0) fz=1;
      else fz=0;
      fs=(a>>7)&1;
      fn=1;
      pc++;
      clk+=7;
    break;

    case 0x9F:                  //sbc a
      if (fc)
	{a=0xFF;fz=0;}
      else
	{a=0;fz=1;}
      fs=(a>>7)&1;
      fn=1;
      pc++;
      clk+=4;
    break;

    case 0xA0:                  //and b
      a=a&b;
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=1;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xA1:                  //and c
      a=a&c;
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=1;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xA2:                  //and d
      a=a&d;
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=1;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xA3:                  //and e
      a=a&e;
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=1;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xA4:                  //and h
      a=a&h;
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=1;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xA5:                  //and l
      a=a&l;
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=1;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xA6:                  //and (hl)
      a=a&readmem((h<<8)+l);
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=1;
      fs=(a>>7)&1;
      pc++;
      clk+=7;
    break;

    case 0xA7:                  //and a
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=1;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xA8:                  //xor b
      a=a^b;
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=0;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xA9:                  //xor c
      a=a^c;
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=0;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xAA:                  //xor d
      a=a^d;
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=0;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xAB:                  //xor e
      a=a^e;
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=0;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xAC:                  //xor h
      a=a^h;
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=0;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xAD:                  //xor l
      a=a^l;
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=0;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xAE:                  //xor (hl)
      a=a^readmem((h<<8)+l);
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=0;
      fs=(a>>7)&1;
      pc++;
      clk+=7;
    break;

    case 0xAF:                  //xor a
      a=0;
      fz=1;fc=0;fs=0;fn=0;fh=0;
      pc++;
      clk+=4;
    break;

    case 0xB0:                  //or b
      a=a|b;
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=0;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xB1:                  //or c
      a=a|c;
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=0;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xB2:                  //or d
      a=a|d;
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=0;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xB3:                  //or e
      a=a|e;
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=0;
      pc++;
      clk+=4;
    break;

    case 0xB4:                  //or h
      a=a|h;
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=0;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xB5:                  //or l
      a=a|l;
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=0;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xB6:                  //or (hl)
      a=a|readmem((h<<8)+l);     
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=0;
      fs=(a>>7)&1;
      pc++;
      clk+=7;
    break;

    case 0xB7:                  //or a
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=0;
      fs=(a>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xB8:                  //cp b
      fz=0;fc=0;fn=1;
      if(a==b) fz=1;
      else if(a<b) fc=1;
      fs=((a-b)>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xB9:                  //cp c
      fc=0;fz=0;fn=1;
      if(a<c) fc=1;
      else if(a==c) fz=1;
      fs=((a-c)>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xBA:                  //cp d
      fc=0;fz=0;fn=1;
      if(a<d) fc=1;
      else if(a==d) fz=1;
      fs=((a-d)>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xBB:                  //cp e
      fc=0;fz=0;fn=1;
      if(a<e) fc=1;
      else if(a==e) fz=1;
      fs=((a-e)>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xBC:                  //cp h
      fc=0;fz=0;fn=1;
      if(a<h) fc=1;
      else if(a==h) fz=1;
      fs=((a-h)>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xBD:                  //cp l
      fc=0;fz=0;fn=1;
      if(a<l) fc=1;
      else if(a==l) fz=1;
      fs=((a-l)>>7)&1;
      pc++;
      clk+=4;
    break;

    case 0xBE:                  //cp (hl)
      fn=1;
      if(a==readmem((h<<8)+l)) fz=1;
      else fz=0;
      if(a<readmem((h<<8)+l)) fc=1;
      else fc=0;
      fs=((a-readmem((h<<8)+l))>>7)&1;
      pc++;
      clk+=7;
    break;

//  case 0xBF:                  //cp a
//    this opcode seems silly
//    if(a==a) fz=1;
//    else fz=0;
//    if(a<a) fc=1;
//    else fc=0;
//    pc++;
//  break;      

    case 0xC0:                  //ret nz
      if (!fz) {
	pc=(mem[sp+1]<<8)+mem[sp];
	sp+=2;
	clk+=11;
      }
      else {
	pc++;
	clk+=5;
      }
    break;

    case 0xC1:                  //pop bc
      b=mem[sp+1];
      c=mem[sp];
      sp+=2;
      pc++;
      clk+=10;
    break;

    case 0xC2:                  //jp nz,word
      clk+=10;
      if(!fz)
	pc=(mem[pc+2]<<8)+mem[pc+1];
      else pc+=3;
    break;

    case 0xC3:                  //jp word
      pc=(mem[pc+2]<<8)+mem[pc+1];
      clk+=10;
    break;

    case 0xC4:                  //call nz,word
      pc+=3;
      clk+=10;
      if (!fz) {
	sp-=2;
	mem[sp]   = pc&0xFF;
	mem[sp+1] = pc>>8;
	pc=(mem[pc-1]<<8)+mem[pc-2];
	clk+=7;
      }
    break;

    case 0xC5:                  //push bc
      sp-=2;
      mem[sp+1]=b;
      mem[sp]  =c;
      pc++;
      clk+=11;
    break;    

    case 0xC6:                  //add a,byte
      temp=a+mem[pc+1];
      if (a&(1<<3) && mem[pc+1]&(1<<3)) fh=1; else fh=0;
      if (temp>0xFF) fc=1; else fc=0;
      a=temp&0xFF;
      if (a==0) fz=1; else fz=0;
      fs=(a>>7)&1;
      fn=0;
      pc+=2;
      clk+=8;
    break;

    case 0xC7:                  //rst $00
      pc+=3;
      sp-=2;
      mem[sp]   = pc&0xFF;
      mem[sp+1] = pc>>8;
      pc=0x0000;
      clk+=11;
    break;

    case 0xC8:                  //ret z
      if (fz) {
	pc=(mem[sp+1]<<8)+mem[sp];
	sp+=2;
	clk+=11;
      }
      else {
	pc++;
	clk+=5;
      }
    break;      

    case 0xC9:                  //ret
      pc=(mem[sp+1]<<8)+mem[sp];
      sp+=2;
      clk+=10;
    break;

    case 0xCA:                  //jp z,word
      clk+=10;
      if (fz)
	pc=(mem[pc+2]<<8)+mem[pc+1];
      else
	pc+=3;
      break;

//  case 0xCB: invalid 8080 opcode

    case 0xCC:                  //call z,word
      pc+=3;
      clk+=10;
      if (fz) {
	sp-=2;
	mem[sp]   = pc&0xFF;
	mem[sp+1] = pc>>8;
	pc=(mem[pc-1]<<8)+mem[pc-2];
	clk+=7;
      }
    break;

    case 0xCD:                  //call word
      pc+=3;
      sp-=2;
      mem[sp]   = pc&0xFF;
      mem[sp+1] = pc>>8;
      pc=(mem[pc-1]<<8)+mem[pc-2];
      clk+=17;
    break;

    case 0xCE:                  //adc byte
      temp=a+mem[pc+1]+fc;
      if (temp>255) fc=1;
      else fc=0;
      a=temp&0xFF;
      if (a==0) fz=1;
      else fz=0;
      fn=0;
      pc+=2;
      clk+=8;
    break;

    case 0xCF:                  //rst $08
      pc+=3;
      sp-=2;
      mem[sp]   = pc&0xFF;
      mem[sp+1] = pc>>8;
      pc=0x0008;
      clk+=11;
    break;

    case 0xD0:                  //ret nc
      if (!fc) {
	pc=(mem[sp+1]<<8)+mem[sp];
	sp+=2;
	clk+=11;
      }
      else {
	pc++;
	clk+=5;
      }
    break;

    case 0xD1:                  //pop de
      d=mem[sp+1];
      e=mem[sp];
      sp+=2;
      pc++;
      clk+=10;
    break;

    case 0xD2:                  //jp nc,word
      clk+=10;
      if(!fc)
	pc=(mem[pc+2]<<8)+mem[pc+1];
      else 
	pc+=3;
      break;

    case 0xD3:                  //out (byte),a
      outport(mem[pc+1],a);
      pc+=2;
      clk+=11;
    break;

    case 0xD4:                  //call nc,word
      clk+=10;
      pc+=3;
      if (!fc) {
	sp-=2;
	mem[sp]   = pc&0xFF;
	mem[sp+1] = pc>>8;
	pc=(mem[pc-1]<<8)+mem[pc-2];
	clk+=7;
      }
    break;

    case 0xD5:                  //push de
      sp-=2;
      mem[sp+1]= d;
      mem[sp]  = e;
      pc++;
      clk+=11;
    break;    

    case 0xD6:                  //sub byte
      temp=a-mem[pc+1];
      if (temp<0) fc=1;
      else fc=0;
      a=a-mem[pc+1];
      if (a==0) fz=1;
      else fz=0;
      fn=1;
      pc+=2;
      clk+=8;
    break;

    case 0xD7:                  //rst $10
      pc+=3;
      sp-=2;
      mem[sp]   = pc&0xFF;
      mem[sp+1] = pc>>8;
      pc=0x0010;
      clk+=11;
    break;

    case 0xD8:                  //ret c
      if (fc) {
	pc=(mem[sp+1]<<8)+mem[sp];
	sp+=2;
	clk+=11;
      }
      else {
	pc++;
	clk+=5;
      }
    break;

//  case 0xD9: invalid 8080 opcode

    case 0xDA:                  //jp c,word
      clk+=10;
      if (fc)
	pc=(mem[pc+2]<<8)+mem[pc+1];
      else
	pc+=3;
      break;

    case 0xDB:                  //in a,(byte)
      a=inport(mem[pc+1]);
      pc+=2;
      clk+=11;
    break;

    case 0xDC:                  //call c,word
      pc+=3;
      clk+=10;
      if (fc) {
	sp-=2;
	mem[sp]   = pc&0xFF;
	mem[sp+1] = pc>>8;
	pc=(mem[pc-1]<<8)+mem[pc-2];
	clk+=7;
      }
    break;

//  case 0xDD: invalid 8080 opcode

    case 0xDE:                  //sbc a,byte
      temp=a-mem[pc+1]-fc;
      if (temp<0) fc=1;
      else fc=0;
      a=a-mem[pc+1]-fc;
      if (a==0) fz=1;
      else fz=0;
      fn=1;
      pc+=2;
      clk+=8;
    break;

    case 0xDF:                  //rst $18
      pc+=3;
      sp-=2;
      mem[sp]   = pc&0xFF;
      mem[sp+1] = pc>>8;
      pc=0x0018;
      clk+=11;
    break;

    case 0xE0:                  //ret po
      if (fv) {
      pc=(mem[sp+1]<<8)+mem[sp];
      sp+=2;
        clk+=11;
      }
      else {
        pc++;
        clk+=5;
      }
    break;

    case 0xE1:                  //pop hl
      l=mem[sp];
      h=mem[sp+1];
      sp+=2;
      pc++;
      clk+=10;
    break;

    case 0xE2:                  //jp po,word
      clk+=10;
      if(fv)
      pc=(mem[pc+2]<<8)+mem[pc+1];
      else pc+=3;
    break;

    case 0xE3:                  //ex (sp),hl
      temp=l;l=mem[sp]  ;mem[sp]  =temp;
      temp=h;h=mem[sp+1];mem[sp+1]=temp;
      pc++;
      clk+=19;
    break;

    case 0xE4:                  //call po,word
      pc+=3;
      clk+=10;
      if (fv) {
      sp-=2;
      mem[sp]   = pc&0xFF;
      mem[sp+1] = pc>>8;
      pc=(mem[pc-1]<<8)+mem[pc-2];
        clk+=7;
      }      
    break;

    case 0xE5:                  //push hl
      sp-=2;
      mem[sp]  = l;
      mem[sp+1]= h;
      pc++;
      clk+=11;
    break;    

    case 0xE6:                  //and byte
      a = a & mem[pc+1];
      if (a==0) fz=1; 
      else fz=0;
      fc=0;fn=0;fh=1;
      pc+=2;
      clk+=8;
    break;

    case 0xE7:                  //rst $20
      pc+=3;
      sp-=2;
      mem[sp]   = pc&0xFF;
      mem[sp+1] = pc>>8;
      pc=0x0020;
      clk+=11;
    break;

    case 0xE8:                  //ret pe
      if (!fv) {
      pc=(mem[sp+1]<<8)+mem[sp];
      sp+=2;
        clk+=11;
      }
      else {
        pc++;
        clk+=5;
      }
    break;

    case 0xE9:                  //jp (hl)
      pc=(h<<8)+l;
      clk+=4;
    break;

    case 0xEA:                  //jp pe,word
      clk+=10;
      if(!fv)
      pc=(mem[pc+2]<<8)+mem[pc+1];
      else pc+=3;
    break;

    case 0xEB:                  //ex de,hl
      temp=d;d=h;h=temp;
      temp=e;e=l;l=temp;
      pc++;
      clk+=4;
    break;

    case 0xEC:                  //call pe,word
      pc+=3;
      clk+=10;
      if (!fv) {
      sp-=2;
      mem[sp]   = pc&0xFF;
      mem[sp+1] = pc>>8;
      pc=(mem[pc-1]<<8)+mem[pc-2];
        clk+=7;
      }      
    break;

//  case 0xED: invalid 8080 opcode

    case 0xEE:                  //xor byte
      a=a^mem[pc+1];
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=0;
      pc+=2;
      clk+=8;
    break;

    case 0xEF:                  //rst $28
      pc+=3;
      sp-=2;
      mem[sp]   = pc&0xFF;
      mem[sp+1] = pc>>8;
      pc=0x0028;
      clk+=11;
    break;

    case 0xF0:                  //ret p
      if (!fs) {
      pc=(mem[sp+1]<<8)+mem[sp];
      sp+=2;
        clk+=11;
      }
      else {
        pc++;
        clk+=5;
      }
    break;

    case 0xF1:                  //pop af
      fs=(mem[sp]>>7)&1;
      fz=(mem[sp]>>6)&1;
      fh=(mem[sp]>>4)&1;
      fv=(mem[sp]>>2)&1;
      fn=(mem[sp]>>1)&1;
      fc=(mem[sp]>>0)&1;
      a=mem[sp+1];
      sp+=2;
      pc++;
      clk+=10;
    break;

    case 0xF2:                  //jp p,word
      clk+=10;
      if(!fs)
      pc=(mem[pc+2]<<8)+mem[pc+1];
      else pc+=3;
    break;

    case 0xF3:                  //di
      iff=0;
      pc++;
      clk+=4;
    break;

    case 0xF4:                  //call p, word
      pc+=3;
      clk+=10;
      if (!fs) {
      sp-=2;
      mem[sp]   = pc&0xFF;
      mem[sp+1] = pc>>8;
      pc=(mem[pc-1]<<8)+mem[pc-2];
        clk+=7;
      }
    break;

    case 0xF5:                  //push af
      sp-=2;
      mem[sp]=(fs<<7)+(fz<<6)+(fh<<4)+(fv<<2)+(fn<<1)+(fc<<0);
      mem[sp+1]=a;
      pc++;
      clk+=11;
    break;

    case 0xF6:                  //or byte
      a=a|mem[pc+1];
      if (a==0) fz=1;
      else fz=0;
      fc=0;fn=0;fh=0;
      pc+=2;
      clk+=8;
    break;

    case 0xF7:                  //rst $30
      pc+=3;
      sp-=2;
      mem[sp]   = pc&0xFF;
      mem[sp+1] = pc>>8;
      pc=0x0030;
      clk+=11;
    break;

    case 0xF8:                  //ret m
      if (fs) {
      pc=(mem[sp+1]<<8)+mem[sp];
      sp+=2;
        clk+=11;
      }
      else {
        pc++;
        clk+=5;
      }
    break;

    case 0xF9:                  //ld sp,hl
      sp=(h<<8)+l;
      pc++;
      clk+=6;
    break;

    case 0xFA:                  //jp m,word
      clk+=10;
      if(fs)
	pc=(mem[pc+2]<<8)+mem[pc+1];
      else pc+=3;
    break;

    case 0xFB:                  //ei
      iff=1;
      pc++;
      clk+=4;
    break;

    case 0xFC:                  //call m,word
      pc+=3;
      clk+=10;
      if (fs) {
      sp-=2;
      mem[sp]   = pc&0xFF;
      mem[sp+1] = pc>>8;
      pc=(mem[pc-1]<<8)+mem[pc-2];
        clk+=7;
      }
    break;

//  case 0xFD: invalid 8080 opcode

    case 0xFE:                  //cp byte
      if(a==mem[pc+1]) fz=1;
      else fz=0;
      if(a<mem[pc+1]) fc=1;
      else fc=0;
      fs=(((a-mem[pc+1])&128)>>7)&1;
      pc+=2;
      clk+=8;
    break;

    case 0xFF:                  //rst $38
      pc+=3;
      sp-=2;
      mem[sp]   = pc&0xFF;
      mem[sp+1] = (pc>>8);
      pc=0x0038;
      clk+=11;
    break;

    default:                     //umimplemented
//      printf("unimplemented opcode %d",opcode);
//      printf(" at address %d",pc);
//      printf("\n a = %d",a);
//      printf("\n b = %d",b);
//      printf("\n c = %d",c);
//      printf("\n d = %d",d);
//      printf("\n e = %d",e);
//      printf("\n h = %d",h);
//      printf("\n l = %d",l);
//      printf("\n sp = %d",sp);
//      printf("\n flags = %d",(fs<<7)+(fz<<6)+(fh<<4)+(fv<<2)+(fn<<1)+(fc<<0));
//      exit(0);
    break;
   }     //switch
  }     //while
}
