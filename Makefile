# Primary Directive
all: windows run

# Function that expands to all files recursively
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

# Directory where all source files are stored
SRC_DIR := src
# All source files stored in $SRC_DIR
SRC_FILES := $(shell find $(SRC_DIR) -name '*.cpp')
# Directory to store `.o` files
OBJ_DIR := obj

# Create string repr of object files from source files
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
# Rule for compiling object files based on changes in the source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo -n "$@ "
	@mkdir -p $(@D)
	@$(CXX) -c $< -o $@ $(CXXFLAGS)


SHADER_DIR := shaders
SHADER_OUT_DIR := obj
SHADER_VERT_FILES := $(shell find $(SHADER_DIR) -name '*.vert')
SHADER_FRAG_FILES := $(shell find $(SHADER_DIR) -name '*.frag')
SHADER_VERT_SPIRV_FILES := $(patsubst $(SHADER_DIR)/%.vert,$(SHADER_OUT_DIR)/%.vert.spv,$(SHADER_VERT_FILES))
SHADER_FRAG_SPIRV_FILES := $(patsubst $(SHADER_DIR)/%.frag,$(SHADER_OUT_DIR)/%.frag.spv,$(SHADER_FRAG_FILES))


$(SHADER_OUT_DIR)/%.spv: $(SHADER_DIR)/%
	@echo -n "$@ "
	@mkdir -p $(@D)
	@$(GLSLC) -c $< -o $@



# Run `make clean` before changing name of program
PROGRAM := main



program: $(OBJ_FILES)
	@echo ""
	@echo "Object Files Compiled!"
	@echo ""
	@echo "Compiling Executable..."
	$(CXX) $^ -o $(PROGRAM) $(CXXFLAGS)


windows:
	@echo "Making game for Windows"
	@echo -n "Compiling Shaders: "
	@$(MAKE) shaders \
	GLSLC="./bin/glslc.exe" \
	--no-print-directory

	@echo -n "Compiling Object Files: "
	@$(MAKE) program \
	CXX="x86_64-w64-mingw32-g++" \
	CXXFLAGS="-I include -L lib/x86_64-w64-mingw32 -lopengl32 -lglfw3dll -lvulkan -std=c++20" \
	--no-print-directory
	@echo ""

linux:
	@echo "Making game for Linux"
	@echo -n "Compiling Shaders: "
	@$(MAKE) shaders \
	GLSLC="./bin/glslc" \
	--no-print-directory

	@echo -n "Compiling Object Files: "
	@$(MAKE) program \
	CXX="g++" \
	CXXFLAGS="-I include -L lib/x86_64-linux-gnu -lglfw3 -lvulkan -lGL -std=c++20" \
	--no-print-directory
	@echo ""

shaders: $(SHADER_VERT_SPIRV_FILES) $(SHADER_FRAG_SPIRV_FILES)
	@echo ""
	@echo "Shaders Compiled!"

test:
	@echo "              SRC_FILES: $(SRC_FILES)"
	@echo "              OBJ_FILES: $(OBJ_FILES)"
	@echo "      SHADER_VERT_FILES: $(SHADER_VERT_FILES)"
	@echo "      SHADER_FRAG_FILES: $(SHADER_FRAG_FILES)"
	@echo "SHADER_VERT_SPIRV_FILES: $(SHADER_VERT_SPIRV_FILES)"
	@echo "SHADER_FRAG_SPIRV_FILES: $(SHADER_FRAG_SPIRV_FILES)"


run:
	@# Uses wildcard to be platform independant -- might cause unintended behavior
	@echo ">>> Starting Program"
	@./$(PROGRAM)*

clean:
	@rm -rf $(PROGRAM) $(PROGRAM).exe $(OBJ_DIR)
	@echo Project cleaned!