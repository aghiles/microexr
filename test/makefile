
.SUFFIXES: .cpp .h

all: gradient.exr deepgradient.exr

gradient: gradient.cpp ../lib/libmicroexr.a
	g++ -I../src -L../lib -lz -lmicroexr $< -o $@

deepgradient: deepgradient.cpp ../lib/libmicroexr.a
	g++ -I../src -L../lib -lz -lmicroexr $< -o $@

deepgradient.exr: deepgradient
	./deepgradient

gradient.exr: gradient
	./gradient

clean:
	rm -f $(OBJ) $(OUT)
	rm -f gradient deepgradient
	rm *.exr
