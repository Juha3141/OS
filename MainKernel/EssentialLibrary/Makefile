TEMPRORYFOLDER = ../../Temprory
OBJECTSTEMPRORYFOLDER = $(TEMPRORYFOLDER)/EssentialLibrary

CC = g++
AR = ar

CCOPTIONS = -Os -m64 -fpack-struct=1 -fno-stack-protector -fno-builtin -W -Wall -masm=intel -nostdlib # -mno-red-zone -mno-mmx -mno-sse -mno-sse2
MAINSOURCES = string.obj vsprintf.obj
TARGET = $(TEMPRORYFOLDER)/libEssentialLibrary.a

prepare:
	mkdir $(OBJECTSTEMPRORYFOLDER)

all: prepare $(MAINSOURCES)
	cd $(OBJECTSTEMPRORYFOLDER); $(AR) rcs $(TARGET) $(MAINSOURCES)

clean:
	rm -rf $(OBJECTSTEMPRORYFOLDER)
	rm -rf $(TARGET)

%.obj:
	$(CC) -m64 -c $*.cpp -o $(OBJECTSTEMPRORYFOLDER)/$*.obj $(CCOPTIONS)
