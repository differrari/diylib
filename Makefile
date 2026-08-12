include ./common.mk

WINDOW_BACKEND := glfw
GRAPH_BACKEND := opengl

PLATFORM_INCLUDES := -I./vendored/glfw/include/GLFW
# PLATFORM_INCLUDES := -I./vendored/raylib/src
PLATFORM_LINKS := ./vendored/glfw/build/libglfw3.a

LINKS := $(PLATFORM_LINKS) ./vendored/redlib/clibshared.a
INCLUDES := $(PLATFORM_INCLUDES) -I./vendored/redlib -I./vendored/three2d -I./src/common

C_SRC := $(shell find ./src/common -name "*.c")

ifneq ($(WINDOW_BACKEND),)
	INCLUDES += -I./src/$(WINDOW_BACKEND) 
	C_SRC += $(shell find ./src/$(WINDOW_BACKEND) -name "*.c")
endif

ifneq ($(GRAPH_BACKEND),)
	INCLUDES += -I./src/$(GRAPH_BACKEND)
	C_SRC += $(shell find ./src/$(GRAPH_BACKEND) -name "*.c")
endif	
	
CFLAGS += $(CFLAGS_BASE)

UNAME_S := $(shell uname -s)

CLEAN_OBJS := $(shell find $(BUILD_DIR) -name "*.o")
CLEAN_DEPS := $(shell find $(BUILD_DIR) -name "*.d")
OBJ     := $(C_SRC:%.c=$(BUILD_DIR)/%.o)
DEP     := $(C_SRC:%.c=$(BUILD_DIR)/%.d)

TARGET  := ./diylib.a

.PHONY: all clean prepare glfw redlib raylib

all: prepare glfw redlib raylib $(TARGET)
	@echo "C SOURCES $(C_SRC)"

prepare:
	mkdir -p $(BUILD_DIR)

raylib:
	make -C vendored/raylib/src PLATFORM=PLATFORM_DESKTOP

glfw:
	make -C vendored/glfw

redlib:
	make -C vendored/redlib cross

# three2d:

$(TARGET): $(OBJ)
	@echo "Finishing build $(ARCH)"
	echo $(addprefix $(BUILD_DIR)/,$(notdir $(OBJ)))
	$(VAR) rcs $@ $(OBJ)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(VCC) $(CFLAGS) -std=c99 $(INCLUDES) -DCROSS -c -MMD -MP $< -lc -lm -o $@

clean:
	$(RM) ./$(TARGET)
	$(RM) -r ./.build

-include $(DEP)
