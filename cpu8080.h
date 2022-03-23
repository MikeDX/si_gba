
typedef unsigned char byte;
typedef unsigned short word;


extern void writemem(UINT32 addr, UINT8 bVal, struct MemoryWriteByte *psWrite);
extern void romwrite(UINT32 addr, UINT8 val, struct MemoryWriteByte *psWrite);
extern UINT8 readmem(UINT32 addr, struct MemoryReadByte *pPW);

extern UINT16 inport(UINT16 port, struct z80PortRead *pPR);
extern void outport(UINT16 port, UINT8 val, struct z80PortWrite *pPW );
extern void run_8080(int);
extern void reset_8080(void);
extern void int_8080(void);
extern void nmi_8080(void);
void PlotPixel(byte xtemp, byte ytemp, word c); // Plots a pixel at coordonate x,y with color c

void InvRomWrite(UINT32 dwAddr, UINT8 bVal, struct MemoryWriteByte *psWrite );
void InvRamWrite(UINT32 dwAddr, UINT8 bVal, struct MemoryWriteByte *psWrite );

void InvVideoWrite(UINT32 dwAddr, UINT8 bVal, struct MemoryWriteByte *psWrite );

UINT16 InvIntRead(UINT16 port, struct z80PortRead *pPR);
void InvIntWrite(UINT16 port, UINT8 bVal, struct z80PortWrite *pPW );
