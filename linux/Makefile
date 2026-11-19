CFLAGS=-D__LINUX__ -Wall -O2 
LIBS = 

all: jpegenc

jpegenc: main.o JPEGENC.o
	$(CC) main.o JPEGENC.o $(LIBS) -o jpegenc 

main.o: main.cpp
	$(CXX) $(CFLAGS) -c main.cpp

JPEGENC.o: ../src/JPEGENC.cpp ../src/jpegenc.inl ../src/JPEGENC.h
	$(CXX) $(CFLAGS) -c ../src/JPEGENC.cpp

# Unit tests: "make test". Each test links against the C core
# (JPEGENC_NO_ARDUINO) built with AddressSanitizer and
# UndefinedBehaviorSanitizer. The sanitizer flags need Clang. The target
# also runs the jpegenc example, which encodes through the C++ class.
TEST_CC ?= clang
TEST_CFLAGS = -DJPEGENC_NO_ARDUINO -I../src -Wall -O1 -g \
	-fno-omit-frame-pointer -fsanitize=address,undefined \
	-fno-sanitize-recover=all
TEST_OBJS = jpegenc_test_core.o jpegenc_test_api.o
TESTS =
TESTS += c_api_test
TESTS += edge_mcu_test
TESTS += yuv420p_test
TESTS += nv12_test
TESTS += rgb565_endian_test

jpegenc_test_core.o: jpegenc_test_core.c ../src/JPEGENC.h ../src/jpegenc.inl
	$(TEST_CC) $(TEST_CFLAGS) -c jpegenc_test_core.c -o $@

jpegenc_test_api.o: jpegenc_test_api.c jpegenc_test.h ../src/JPEGENC.h
	$(TEST_CC) $(TEST_CFLAGS) -c jpegenc_test_api.c -o $@

%_test: %_test.c $(TEST_OBJS) jpegenc_test.h
	$(TEST_CC) $(TEST_CFLAGS) $< $(TEST_OBJS) -lm -o $@

test: jpegenc $(TESTS)
	@for t in $(TESTS); do \
		echo "./$$t"; \
		ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
		UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ./$$t || exit 1; \
	done
	./jpegenc test_example.jpg
	test -s test_example.jpg

.PHONY: all test clean

clean:
	rm -rf *.o jpegenc $(TESTS) test_example.jpg
