@cls
ztasm -Littleend -cpu ARM7TDMI boot.asm
ztasm -Littleend -cpu ARM7TDMI draw.s
rem zarmcc -c -Wall -Otime  -fpu none -Littleend -cpu ARM7TDMI -apcs /narrow/noswst cpu8080.c  -o cpu8080.o
rem ztcc -c -Wall -Otime  -fpu none -Littleend -cpu ARM7TDMI -apcs /narrow/noswst mz80.c  -o mz80.o
ztcc -c -Wall -Otime  -fpu none -Littleend -cpu ARM7TDMI -apcs /narrow/noswst main.c  -o main.o
zarmlink -bin -first boot.o -map -ro-base 0x08000000 -rw-base 0x2000000 boot.o main.o draw.o  mz80.o  -o gbasi.gba
rem gbafix gbasi.gba -p
