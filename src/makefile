.PHONY: clean

MICROEXRLIB = ../lib/libmicroexr.a

CFLAGS = -Wall -O2

SOURCES = \
	ImfCFile.cpp \
	channellist.cpp \
	half.cpp \
	scanlineblock.cpp \
	scanlinezipblock.cpp \
	attribute.cpp \
	compressor.cpp \
	header.cpp \
	scanlinedeepblock.cpp \
	channel.cpp \
	exrfile.cpp \
	scanlinedeepzipsblock.cpp

OBJECTS = $(SOURCES:.cpp=.o)

%.o : %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

$(MICROEXRLIB): $(OBJECTS)
	ar rcs $(MICROEXRLIB) $(OBJECTS)

clean:
	rm *.o
	rm ../lib/*.a
