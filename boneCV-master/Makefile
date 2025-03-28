CROSS_TOOL = aarch64-linux-gnu-
CC = $(CROSS_TOOL)gcc
CFLAGS = -Wall -g -std=c99 -D_POSIX_C_SOURCE=200809l -Werror -Wshadow
LFLAGS = -L$(HOME)/cmpt433/public/v4l2_lib_BB -lv4l2

TARGET = capture_exec
SRC = capture.c
DEST_DIR = $(HOME)/cmpt433/public/myApps/

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(FLAGS) $(SRC) -o $(TARGET) $(LFLAGS)

install: $(TARGET)
	cp $(TARGET) $(DEST_DIR)

clean:
	rm -f $(TARGET)

.PHONY: all install clean