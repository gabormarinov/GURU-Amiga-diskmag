LC = lc:lc
ASM = lc:asm
CFLAGS = -iINCLUDE: -b0 -d0 -v
ASMFLAGS = -iINCLUDE:
LINK = lc:blink
LIB = lib:amiga.lib+lib:lc.lib
OBJ = printertag.o+init.o+data.o+dospecial.o+render.o+transfer.o+density.o
TARGET = EpsonX

.c.o:
   @$(LC) $(CFLAGS) $*

$(TARGET): printertag.o init.o data.o dospecial.o render.o density.o transfer.o
   @$(LINK) <WITH <
   FROM $(OBJ)
   TO $(TARGET)
   LIBRARY $(LIB)
   NODEBUG SC SD VERBOSE MAP $(TARGET).map H
   <

init.o: init.asm
   @$(ASM) $(ASMFLAGS) init.asm

printertag.o: printertag.asm epsonx_rev.i
   @$(ASM) $(ASMFLAGS) printertag.asm

transfer.o: transfer.asm
   @$(ASM) $(ASMFLAGS) transfer.asm

dospecial.o: dospecial.c

data.o: data.c

density.o: density.c

render.o: render.c

install:
   @copy $(TARGET) to devs:printers
