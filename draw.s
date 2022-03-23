; gba library drawing routines.
; Pete (dooby@bits.bris.ac.uk)

; Not many of these are optimal, but they are WIP. Sometimes there are many
; routines - either I was testing two approaches or one is fastest depending
; on surrounding context...
	AREA bank_data, DATA

	ALIGN 16

|gba_bank|
	DCD	0x00000001  		; Current write bank (1 or 2).
	DCD	0x06000000		; Bank 1 address.
	DCD	0x0600a000 		; Bank 2 address.

;	END
; IMPORT	|gba_bank|
;	IMPORT	|gba_colour|
;	IMPORT	|gba_udiv|
;	IMPORT	|gba_div|
;	IMPORT	|gba_qdiv|

;	EXPORT	gba_scanline
;	EXPORT	gba_scanlineU
;	EXPORT	gba_drawline
;	EXPORT	gba_drawpixel
	EXPORT	gba_setpixel
	EXPORT  gfxPixel
;	EXPORT	gba_triangle
;	EXPORT	gba_triangleQ
;	EXPORT	gba_triangleF

	AREA	draw, CODE, READONLY




; void gba_setpixel(int x, int y, int c);
; where 0 < x < 240, 0 < y < 160, 0 < c < 256
gba_setpixel
	; Check bounds.
	cmp	a1, #0x00
	movlt	pc, lr
	cmp	a1, #0xf0
	movge	pc, lr
	cmp	a2, #0x00
	movlt	pc, lr
	cmp	a2, #0xa0
	movge	pc, lr
	; Generate screen address
	rsb	a2, a2, #159		; Y increases go up where I'm from :)
        add     a4, a1, a2, lsl #8
        sub     a1, a4, a2, lsl #4      ; Add y x 240. Maybe a quicker way?
	; Get base address.
	ldr	a2, =|gba_bank|
	ldrb	a4, [a2]
	ldr	a2, [a2, a4, lsl #2]
	; Calculate pixel address.
	add	a1, a1, a2
	; Plot colour.
4	strb	a3, [a1]
	; Return.
	mov	pc, lr

; (void) gfxPixel (u8 x, u8 y, u16 color, u32 addr);


gfxPixel
;        stmfd   sp!,{r4-r5}
;
 ;       mov     a4,#480
  ;      mul r5,a4,a1
        ;mul     a4, =0x6000000, a1
        ;mul a2,=|gba_bank|,a1
        ;add     r5,r5,a0,lsl #1
   ;     add r5,r5,r0,lsl #1
    ;    add     a4,r5,a3
        ;strh    a2,[a4]

        ;ldmfd   sp!,{a4-a5}
;        mov pc,      lr
 
         stmfd   sp!,{r4-r5}

        mov     r4,#480
        mul     r5,r4,r1
        add     r5,r5,r0,lsl #1
        add     r4,r5,r3
        strh    r2,[r4]

        ldmfd   sp!,{r4-r5}
        bx      lr
       
       END