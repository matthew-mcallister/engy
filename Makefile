BUILDDIR ?= builddir
GLSLC ?= glslc

SHADER_SOURCE_DIR := src/shaders
SHADER_BUILD_DIR := $(BUILDDIR)/shaders
GLSL_FILES := $(wildcard $(SHADER_SOURCE_DIR)/*.glsl)
SPV_FILES := $(patsubst $(SHADER_SOURCE_DIR)/%.glsl,$(SHADER_BUILD_DIR)/%.spv,$(GLSL_FILES))

.PHONY: all
all: $(SPV_FILES)

$(SHADER_BUILD_DIR):
	mkdir -p $@

$(SHADER_BUILD_DIR)/%.spv: $(SHADER_SOURCE_DIR)/%.glsl $(SHADER_BUILD_DIR)
	$(GLSLC) -O -o $@ $<
