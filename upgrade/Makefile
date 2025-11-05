
# Variables
CC = gcc
TARGET = bw2bmp
SRC = main.c
HDR = bmp.h

INPUT_BMP = input.bmp
MESSAGE_FILE = message.txt
OUTPUT_BMP_GRAY = output_grayscale.bmp
OUTPUT_BMP_STEGO = output_stego.bmp

DEMO1 = demo1
DEMO2 = demo2


.PHONY: all build
all: $(TARGET)
build: $(TARGET)

# make execute file 
$(TARGET): $(SRC) $(HDR)
	$(CC) $(SRC) -o $(TARGET)
	@echo "'$(TARGET)' executable created."


# Demo 1: -h, -o, -g 
.PHONY: $(DEMO1)
$(DEMO1): $(TARGET)
	@echo "\n--- DEMO 1: -h, -o, -g ---"
	./$(TARGET) -h $(INPUT_BMP)
	@echo "--------------------------------"
	./$(TARGET) -o $(INPUT_BMP)
	@echo "--------------------------------"
	./$(TARGET) -g $(INPUT_BMP) $(OUTPUT_BMP_GRAY)

# Demo 2: -e, -d
.PHONY: $(DEMO2)
$(DEMO2): $(TARGET)
	@echo "\n--- DEMO 2: -e, -d ---"
	./$(TARGET) -e $(INPUT_BMP) $(MESSAGE_FILE) $(OUTPUT_BMP_STEGO) 
	@echo "--------------------------------"
	./$(TARGET) -d $(OUTPUT_BMP_STEGO)


# delete generated files
.PHONY: clean clear
clean clear:
	rm -f $(TARGET) $(OUTPUT_BMP_GRAY) $(OUTPUT_BMP_STEGO) $(TARGET)
