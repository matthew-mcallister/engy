BUILDDIR ?= builddir
GLSLC ?= glslc

SHADER_SOURCE_DIR := src/shaders
SHADER_BUILD_DIR := $(BUILDDIR)/shaders
GLSL_FILE_NAMES := \
	chunk.vertex.glsl \
	chunk.fragment.glsl \
	fullscreen.vertex.glsl \
	sky.fragment.glsl
GLSL_INCLUDE_NAMES := \
	common.glsl
GLSL_FILES := $(patsubst %,$(SHADER_SOURCE_DIR)/%,$(GLSL_FILE_NAMES))
GLSL_INCLUDE_FILES := $(patsubst %,$(SHADER_SOURCE_DIR)/%,$(GLSL_INCLUDE_NAMES))
SPV_FILES := $(patsubst %.glsl,$(SHADER_BUILD_DIR)/%.spv,$(GLSL_FILE_NAMES))

.PHONY: all
all: $(SPV_FILES)

.PHONEY: clean
clean:
	rm -r $(SHADER_BUILD_DIR)

$(SHADER_BUILD_DIR):
	mkdir -p $@

$(SHADER_BUILD_DIR)/%.spv: $(SHADER_SOURCE_DIR)/%.glsl $(GLSL_INCLUDE_FILES) $(SHADER_BUILD_DIR)
	$(GLSLC) -I $(SHADER_SOURCE_DIR) -O -o $@ $<
