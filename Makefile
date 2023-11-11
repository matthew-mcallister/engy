BUILDDIR ?= builddir
GLSLC ?= glslc

SHADER_SOURCE_DIR := src/shaders
SHADER_BUILD_DIR := $(BUILDDIR)/shaders
GLSL_FILE_NAMES := triangle.vertex.glsl triangle.fragment.glsl
GLSL_FILES := $(patsubst %,$(SHADER_SOURCE_DIR)/%,$(GLSL_FILE_NAMES))
SPV_FILES := $(patsubst %.glsl,$(SHADER_BUILD_DIR)/%.spv,$(GLSL_FILE_NAMES))

.PHONY: all
all: $(SPV_FILES)

$(SHADER_BUILD_DIR):
	mkdir -p $@

$(SHADER_BUILD_DIR)/%.spv: $(SHADER_SOURCE_DIR)/%.glsl $(SHADER_BUILD_DIR)
	$(GLSLC) -I $(SHADER_SOURCE_DIR) -O -o $@ $<
