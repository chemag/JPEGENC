CFLAGS=-D__LINUX__ -Wall -O2 
ASAN_FLAGS=-fsanitize=address -fno-omit-frame-pointer -O1 -g -Werror=vla
ALIGN_FLAGS=-fsanitize=alignment -fno-sanitize-recover=alignment -fno-omit-frame-pointer -O1 -g
LIBS = 

all: jpegenc

jpegenc: main.o JPEGENC.o jpegenc_core.o
	$(CC) main.o JPEGENC.o jpegenc_core.o $(LIBS) -o jpegenc

c_api_test: c_api_test.o jpegenc_core.o
	$(CC) c_api_test.o jpegenc_core.o $(LIBS) -o c_api_test

edge_mcu_test: edge_mcu_test.o jpegenc_core_asan.o
	$(CC) $(ASAN_FLAGS) edge_mcu_test.o jpegenc_core_asan.o $(LIBS) -o edge_mcu_test

rgb565_endian_test: rgb565_endian_test.o jpegenc_core_asan.o
	$(CC) $(ASAN_FLAGS) rgb565_endian_test.o jpegenc_core_asan.o $(LIBS) -o rgb565_endian_test

yuv420p_test: yuv420p_test.o jpegenc_core_asan.o
	$(CC) $(ASAN_FLAGS) yuv420p_test.o jpegenc_core_asan.o $(LIBS) -o yuv420p_test

nv12_test: nv12_test.o jpegenc_core_asan.o
	$(CC) $(ASAN_FLAGS) nv12_test.o jpegenc_core_asan.o $(LIBS) -o nv12_test

unaligned_test: unaligned_test.o jpegenc_core_align.o
	$(CC) $(ALIGN_FLAGS) unaligned_test.o jpegenc_core_align.o $(LIBS) -o unaligned_test

yuv422_subsample_test: yuv422_subsample_test.o jpegenc_core_asan.o
	$(CC) $(ASAN_FLAGS) yuv422_subsample_test.o jpegenc_core_asan.o $(LIBS) -o yuv422_subsample_test

main.o: main.cpp
	$(CXX) $(CFLAGS) -c main.cpp

c_api_test.o: c_api_test.c
	$(CC) $(CFLAGS) -c c_api_test.c

edge_mcu_test.o: edge_mcu_test.c
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c edge_mcu_test.c

rgb565_endian_test.o: rgb565_endian_test.c
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c rgb565_endian_test.c

yuv420p_test.o: yuv420p_test.c
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c yuv420p_test.c

nv12_test.o: nv12_test.c
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c nv12_test.c

unaligned_test.o: unaligned_test.c
	$(CC) $(CFLAGS) $(ALIGN_FLAGS) -c unaligned_test.c

yuv422_subsample_test.o: yuv422_subsample_test.c
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c yuv422_subsample_test.c

JPEGENC.o: ../src/JPEGENC.cpp ../src/JPEGENC.h
	$(CXX) $(CFLAGS) -c ../src/JPEGENC.cpp

jpegenc_core.o: ../src/jpegenc.c ../src/jpegenc.inl ../src/JPEGENC.h
	$(CC) $(CFLAGS) -c ../src/jpegenc.c -o jpegenc_core.o

jpegenc_core_asan.o: ../src/jpegenc.c ../src/jpegenc.inl ../src/JPEGENC.h
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c ../src/jpegenc.c -o jpegenc_core_asan.o

jpegenc_core_align.o: ../src/jpegenc.c ../src/jpegenc.inl ../src/JPEGENC.h
	$(CC) $(CFLAGS) $(ALIGN_FLAGS) -c ../src/jpegenc.c -o jpegenc_core_align.o

test: jpegenc c_api_test edge_mcu_test rgb565_endian_test yuv420p_test nv12_test unaligned_test yuv422_subsample_test
	./c_api_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./edge_mcu_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./rgb565_endian_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./yuv420p_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./nv12_test
	UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ./unaligned_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./yuv422_subsample_test
	./jpegenc cxx_api_test.jpg
	test -s cxx_api_test.jpg

clean:
	rm -rf *.o jpegenc c_api_test edge_mcu_test rgb565_endian_test yuv420p_test nv12_test unaligned_test yuv422_subsample_test cxx_api_test.jpg
