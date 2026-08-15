include ./common.mk

WINDOW_BACKEND := glfw
GRAPH_BACKEND := vulkan

PLATFORM_INCLUDES := -I./vendored/glfw/include/GLFW
# PLATFORM_INCLUDES := -I./vendored/raylib/src

LINKS := ./vendored/redlib/clibshared.a
INCLUDES := $(PLATFORM_INCLUDES) -I./vendored/redlib -I./vendored/three2d -I./src/common

C_SRC := $(shell find ./src/common -name "*.c") $(shell sed ./vendored/redlib/simplemake -e '/^[[:space:]]*#/d' -e 's|^|./vendored/redlib/|')

ifneq ($(WINDOW_BACKEND),)
	INCLUDES += -I./src/$(WINDOW_BACKEND) 
	C_SRC += $(shell find ./src/$(WINDOW_BACKEND) -name "*.c")
	C_SRC += $(shell sed ./vendored/$(WINDOW_BACKEND)/simplemake -e '/^[[:space:]]*#/d' -e 's|^|./vendored/$(WINDOW_BACKEND)/|')

CFLAGS += -fPIC

PLATFORM = linux
ifeq ($(PLATFORM),linux)
	CFLAGS += -D_GLFW_X11 -D_GLFW_WAYLAND -DHAVE_MEMFD_CREATE -D_DEFAULT_SOURCE -I$(BUILD_DIR)/wayland
else ifeq ($(PLATFORM),windows)
	CFLAGS += -D_GLFW_WIN32 -D_CRT_NO_SECURE_WARNINGS
else ifeq ($(PLATFORM),macos)
	CFLAGS += -D_GLFW_COCOA
endif
#  CFLAGS += -DPLATFORM_DESKTOP_GLFW -DGRAPHICS_API_OPENGL_33
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

.PHONY: all clean prepare

all: prepare $(TARGET)
	@echo "C SOURCES $(C_SRC)"

prepare:
	@echo $(C_SRC) 
	mkdir -p $(BUILD_DIR)
	./gen_wayland
	./gen_shaders

$(TARGET): $(OBJ)
	@echo "Finishing build $(ARCH)"
	echo $(addprefix $(BUILD_DIR)/,$(notdir $(OBJ)))
	$(VAR) rcs $@ $(OBJ)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(VCC) $(CFLAGS) -std=gnu99 $(INCLUDES) -DCROSS -c $< -o $@

clean:
	$(RM) ./$(TARGET)
	$(RM) -rf ./.build

-include $(DEP)
