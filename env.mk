CC := arm-linux-gnueabihf-gcc

# Header file
#INCLUDE_DIR := -I ${HOME}/MYS-SAM9X5/alsa-lib-1.0.29-arm/include
INCLUDE_DIR := -I ${PWD}/library/include
# Library file
#LIB_DIR     := -L ${HOME}/MYS-SAM9X5/alsa-lib-1.0.29-arm/lib/
LIB_DIR     := -L ${PWD}/library/lib

#CFLAGS := $(INCLUDE_DIR) $(LIB_DIR) -lpthread -lasound `pkg-config --cflags --libs libdrm` -Wall -O0 -g 
CFLAGS := $(INCLUDE_DIR) $(LIB_DIR) -lpthread    -Wall -O0 -g 

#-DSHOW_FPS -DSAVE_BMP -lasound #-lbmp

# Create .o by .c
%.o : %.c
	${CC} ${CFLAGS} -c $< -o $(MAKEROOT)/obj/$@
