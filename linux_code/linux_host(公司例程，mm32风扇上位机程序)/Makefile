CFLAGS =  -Wall -O2 -g -DDEBUG -Wl,-Map,mrs485.map
CC = gcc
INCLUDES = -I.
TARGET :=  oam
OBJECTS =  oam.o
	
all: $(OBJECTS)
	@echo "Build $(TARGET)..."
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $(TARGET) -lpthread
	@echo ""
    
$(OBJECTS): %.o : %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@
    
clean:
	rm -f *.o  $(TARGET) *.map

