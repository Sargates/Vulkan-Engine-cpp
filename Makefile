# Primary Directive
all: windows run

# Function that expands to all files recursively
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

# All `.cpp` files in `src` directory
SRC_DIR := src
SRC_FILES := $(call rwildcard,$(SRC_DIR)/,*.cpp)
# Directory to store `.o` files
OBJ_DIR := obj
# These two lines compile all object files from updated `.cpp` files, I do not understand this
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(filter $(SRC_DIR)/%.cpp,$(SRC_FILES))) \
			 $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(filter-out $(SRC_DIR)/%.cpp,$(SRC_FILES)))
INCLUDE_DIR := include
HEADER_FILES := $(call rwildcard,$(INCLUDE_DIR)/,*.hpp)

# Rule for compiling source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADER_FILES)
	@echo -n "$@ "
	@mkdir -p $(@D)
	@$(CXX) -c $< -o $@ $(CXXFLAGS)


SHADER_DIR := shaders
SHADER_OUT_DIR := obj
SHADER_VERT_FILES := $(call rwildcard,$(SHADER_DIR)/,*.vert)
SHADER_FRAG_FILES := $(call rwildcard,$(SHADER_DIR)/,*.frag)

SHADER_VERT_SPIRV_FILES := $(patsubst $(SHADER_DIR)/%.vert,$(SHADER_OUT_DIR)/%.vert.spv,$(filter $(SHADER_DIR)/%.vert,$(SHADER_VERT_FILES))) \
			 $(patsubst %.vert,$(SHADER_OUT_DIR)/%.vert.spv,$(filter-out $(SHADER_DIR)/%.vert,$(SHADER_VERT_FILES)))
$(SHADER_OUT_DIR)/%.vert.spv: $(SHADER_DIR)/%.vert
	@echo "Compiling Vertex Shaders"
	@mkdir -p $(@D)
	@$(GLSLC) -c $< -o $@

SHADER_FRAG_SPIRV_FILES := $(patsubst $(SHADER_DIR)/%.frag,$(SHADER_OUT_DIR)/%.frag.spv,$(filter $(SHADER_DIR)/%.frag,$(SHADER_FRAG_FILES))) \
			 $(patsubst %.frag,$(SHADER_OUT_DIR)/%.frag.spv,$(filter-out $(SHADER_DIR)/%.frag,$(SHADER_FRAG_FILES)))
$(SHADER_OUT_DIR)/%.frag.spv: $(SHADER_DIR)/%.frag
	@echo "Compiling Fragment Shaders"
	@mkdir -p $(@D)
	@$(GLSLC) -c $< -o $@


# Run `make clean` before changing name of program
PROGRAM := main



program: $(OBJ_FILES)
	@echo ""
	@echo "Object Files Compiled!"
	@echo "Compiling Executable..."
	$(CXX) $^ -o $(PROGRAM) $(CXXFLAGS)


windows:
	@echo "Making game for Windows"
	@$(MAKE) shaders \
	GLSLC="./bin/glslc.exe" \
	--no-print-directory
	@echo "Shaders Compiled!"

	@echo -n "Compiling Object Files: "
	@$(MAKE) program \
	CXX="x86_64-w64-mingw32-g++" \
	CXXFLAGS="-I include -L lib/x86_64-w64-mingw32 -lopengl32 -lglfw3dll -lvulkan -std=c++20" \
	--no-print-directory

linux:
	@echo "Making game for Linux"
	@$(MAKE) shaders \
	GLSLC="./bin/glslc" \
	--no-print-directory
	@echo "Shaders Compiled!"

	@echo -n "Compiling Object Files: "
	@$(MAKE) program \
	CXX="g++" \
	CXXFLAGS="-I include -L lib/x86_64-linux-gnu -lglfw3 -lvulkan -lGL -std=c++20" \
	--no-print-directory


shaders: $(SHADER_VERT_SPIRV_FILES) $(SHADER_FRAG_SPIRV_FILES)
	@echo "Compiling Shaders: GLSLC = $(GLSLC)"

# shaders-linux:

test:
	@echo $(SRC_FILES)
	@echo $(OBJ_FILES)
	@echo $(SHADER_VERT_FILES)
	@echo $(SHADER_FRAG_FILES)
	@echo $(SHADER_VERT_SPIRV_FILES)
	@echo $(SHADER_FRAG_SPIRV_FILES)


run:
	@# Uses wildcard to be platform independant -- might cause unintended behavior
	./$(PROGRAM)*

clean:
	@rm -rf $(PROGRAM) $(PROGRAM).exe $(OBJ_DIR)
	@echo Project cleaned!