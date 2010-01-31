base    = TextDump DumpHexMem DisplayHexLine FindPrevTextPage hexdump dumphextofile GetToken PlatformSpec ConvTools FindString \
		  DisplayDiskSector BitDump

headers = ConvTools.h look.h
srcdir = ./src/

obfiles = $(addsuffix .o,$(base))
objects = $(addprefix $(srcdir),$(obfiles))

cc = gcc		
cflags = -lstdc++


look:	$(objects) $(srcdir)look.cpp
	$(CC) -o look $(cflags) $(srcdir)look.cpp $(objects)

$(objects):%.o: %.cpp
	echo $@ -- $(*F) -- $< -- $: -- $. -- $& -- $** -- $? --
	$(CC) -c $? -o $@

clean:	
	@rm $(objects)
	@rm look
