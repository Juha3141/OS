BINARYFOLDER = ../../../Binary/Library
OBJECTSBINARYFOLDER = $(BINARYFOLDER)/EssentialLibrary

CC = x86_64-linux-gnu-g++
AR = x86_64-linux-gnu-ar

CCOPTIONS = -Os -m64 -fpack-struct=1 -fno-stack-protector -fno-builtin -W -Wall -masm=intel -nostdlib
MAINSOURCES = $(patsubst %.cpp,%.obj,$(wildcard *.cpp))
TARGET = $(BINARYFOLDER)/libEssentialLibrary.a

clean:
	rm -rf $(OBJECTSBINARYFOLDER)/*.*
	rm -rf $(TARGET)

all: $(MAINSOURCES)
	cd $(OBJECTSBINARYFOLDER); $(AR) rcs $(TARGET) $(wildcard $(OBJECTSBINARYFOLDER)/*.obj)

%.obj:
	$(CC) -m64 -c $*.cpp -o $(OBJECTSBINARYFOLDER)/$@ $(CCOPTIONS)