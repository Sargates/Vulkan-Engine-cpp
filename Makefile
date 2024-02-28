# Primary Directive
all: windows run

# Function that expands to all files recursively
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

# All `.cpp` files in `src` directory
SRC_FILES := $(call rwildcard,src/,*.cpp)
# Directory to store `.o` files
OBJ_DIR := obj

# These two lines compile all object files from updated `.cpp` files, I do not understand this
OBJ_FILES := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(filter src/%.cpp,$(SRC_FILES))) \
			 $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(filter-out src/%.cpp,$(SRC_FILES)))
# Rule for compiling source files
$(OBJ_DIR)/%.o: src/%.cpp
	@mkdir -p $(@D)
	@$(CXX) -c $< -o $@ $(CXXFLAGS)


# Run `make clean` before changing name of program
PROGRAM := main


program: $(OBJ_FILES)
	$(CXX) $^ -o $(PROGRAM) $(CXXFLAGS)


windows:
	@echo "Making game for Windows"
	@$(MAKE) program \
	CXX="x86_64-w64-mingw32-g++" \
	CXXFLAGS="-I include -L lib/x86_64-w64-mingw32 -DGLEW_STATIC -lopengl32 -lglfw3dll -lvulkan -std=c++20 -static-libgcc -static-libstdc++" \
	--no-print-directory

linux:
	@echo "Making game for Linux"
	@$(MAKE) program \
	CXX="g++" \
	CXXFLAGS="-I include -L lib/x86_64-linux-gnu -lglfw3 -lvulkan -lGL -std=c++20" \
	--no-print-directory

run:
	@# Uses wildcard to be platform independant -- might cause unintended behavior
	./$(PROGRAM)*

clean:
	@rm -rf $(PROGRAM) $(PROGRAM).exe $(OBJ_DIR)
	@echo Project cleaned!