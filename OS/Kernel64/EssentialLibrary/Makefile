TEMPORARYFOLDER = ../../Temporary
OBJECTSTEMPORARYFOLDER = $(TEMPORARYFOLDER)/EssentialLibrary

CC = x86_64-linux-gnu-g++
AR = x86_64-linux-gnu-ar

CCOPTIONS = -Os -m64 -fpack-struct=1 -fno-stack-protector -fno-builtin -W -Wall -masm=intel -nostdlib # -mno-red-zone -mno-mmx -mno-sse -mno-sse2
MAINSOURCES = string.obj vsprintf.obj
TARGET = $(TEMPORARYFOLDER)/libEssentialLibrary.a

prepare:
	mkdir $(OBJECTSTEMPORARYFOLDER)

all: prepare $(MAINSOURCES)
	cd $(OBJECTSTEMPORARYFOLDER); $(AR) rcs $(TARGET) $(MAINSOURCES)

clean:
	rm -rf $(OBJECTSTEMPORARYFOLDER)
	rm -rf $(TARGET)

%.obj:
	$(CC) -m64 -c $*.cpp -o $(OBJECTSTEMPORARYFOLDER)/$*.obj $(CCOPTIONS)
