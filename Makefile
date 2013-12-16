EMSCRIPTEN_ROOT='/Users/hiko/Dropbox/Dev/Emscripten/emsdk_portable/emscripten/1.7.8'
CC=$(EMSCRIPTEN_ROOT)/emcc

SRCS = $(wildcard src/*.cpp)
FT_SRCS = $(wildcard libs/FaceTracker/src/lib/*.cc)
OBJS_PATH = build/objs/
OBJS = $(SRCS:%.cpp=%.o) $(FT_SRCS:%.cc=%.o) 

COPT = -O2 -s OUTLINING_LIMIT=50000 -s EXPORTED_FUNCTIONS="['_hello', '_createTracker', '_update', '_getImagePoints', '_getCalibratedObjectPoints', '_getEulerAngles']"
INCLUDES = -Ilibs/FaceTracker/include -Ilibs/opencv-2.4.7/include
CPPFLAGS = $(COPT) $(INCLUDES)
LIBPATH = -Llibs/opencv-2.4.7/lib -Llibs/opencv-2.4.7/share/OpenCV/3rdparty/lib
LIBS = -lopencv_calib3d -lopencv_objdetect -lopencv_imgproc -lopencv_core -lzlib
# PRELOAD_FILES = $(patsubst %, --preload-file %, $(wildcard libs/FaceTracker/model/*))
LDFLAGS = $(OPTIMIZE_LEVEL) $(LIBPATH) $(LIBS) --preload-file data

all: $(OBJS:%=$(OBJS_PATH)%)
	@echo Linking...
	$(CC) $(COPT) $^ $(LDFLAGS) -o build/libft.js

$(OBJS_PATH)%.o: %.cpp
	@echo Compiling $(basename $<)...
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) -c $< -o $@

$(OBJS_PATH)%.o: %.cc
	@echo Compiling $(basename $<)...
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS_PATH)

hoge:
	@echo $(OBJS)
