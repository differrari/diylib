include ./common.mk

WINDOW_BACKEND ?= glfw
GRAPH_BACKEND ?= opengl

PLATFORM_INCLUDES := -I./vendored/glfw/include/GLFW
# PLATFORM_INCLUDES := -I./vendored/raylib/src

LINKS := ./vendored/redlib/clibshared.a
INCLUDES := $(PLATFORM_INCLUDES) -I./vendored/redlib -I./vendored/three2d -I./src/common

C_SRC := $(shell find ./src/common -name "*.c") $(shell xargs -r -n1 printf 'vendored/redlib/%s\n' < vendored/redlib/simplemake)

M_SRC := 

ifneq ($(WINDOW_BACKEND),)
	INCLUDES += -I./src/$(WINDOW_BACKEND) 
	C_SRC += $(shell find ./src/$(WINDOW_BACKEND) -name "*.c")
	C_SRC += $(shell xargs -r -n1 printf './vendored/$(WINDOW_BACKEND)/%s\n' < ./vendored/$(WINDOW_BACKEND)/simplemake)

CFLAGS += -fPIC

PLATFORM ?= $(shell uname)
ifeq ($(PLATFORM),Linux)
	CFLAGS += -D_GLFW_X11 -D_GLFW_WAYLAND -DHAVE_MEMFD_CREATE -D_DEFAULT_SOURCE -I$(BUILD_DIR)/wayland
else ifeq ($(PLATFORM),windows)
	CFLAGS += -D_GLFW_WIN32 -D_CRT_NO_SECURE_WARNINGS
else ifeq ($(PLATFORM),Darwin)
	CFLAGS += -D_GLFW_COCOA
	M_SRC := vendored/glfw/src/cocoa_init.m vendored/glfw/src/cocoa_window.m vendored/glfw/src/cocoa_monitor.m vendored/glfw/src/cocoa_joystick.m vendored/glfw/src/nsgl_context.m
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
OBJ     := $(C_SRC:%.c=$(BUILD_DIR)/%.o) $(M_SRC:%.m=$(BUILD_DIR)/%.o)
DEP     := $(C_SRC:%.c=$(BUILD_DIR)/%.d)

TARGET  := ./diylib.a

.PHONY: all clean prepare $(C_SRC) $(M_SRC)

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

$(BUILD_DIR)/%.o: %.c prepare
	@mkdir -p $(dir $@)
	$(VCC) $(CFLAGS) -std=gnu99 $(INCLUDES) -DCROSS -c $< -o $@

$(BUILD_DIR)/%.o: %.m prepare
	@mkdir -p $(dir $@)
	$(VCC) $(CFLAGS) -std=gnu99 $(INCLUDES) -DCROSS -c $< -o $@

clean:
	$(RM) ./$(TARGET)
	$(RM) -rf ./.build

-include $(DEP)
