
.SUFFIXES: .cpp .h

gradient: gradient.cpp ../lib/libmicroexr.a
	g++ -I../src -L../lib -lz -lmicroexr $< -o $@

clean:
	rm -f $(OBJ) $(OUT)
