#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo -e "${GREEN}Building C Compiler...${NC}"

# Create build directory if it doesn't exist
mkdir -p build

# Compilation flags
CFLAGS="-Wall -Wextra"

# Function to compile a source file
compile() {
    echo -e "Compiling ${GREEN}$1${NC}..."
    gcc $CFLAGS -c "$1" -o "build/$(basename "$1" .c).o"
    if [ $? -ne 0 ]; then
        echo -e "${RED}Failed to compile $1${NC}"
        exit 1
    fi
}

# Compile files in order of dependency
compile lexer.c
compile parser.c
compile debug.c
compile semantic.c
compile ir.c
compile ir_optimizer.c
compile optimizer.c
compile codegen.c
compile main.c

# Link all object files
echo -e "${GREEN}Linking...${NC}"
gcc build/*.o -o compiler

if [ $? -eq 0 ]; then
    echo -e "${GREEN}Build successful! Executable created: compiler${NC}"
    
    # Create a test file
    echo -e "${GREEN}Creating test file...${NC}"
    cat > test.c << 'EOF'
int main() {
    int x = 42;
    if (x > 40) {
        return x;
    } else {
        return 0;
    }
}
EOF
    
    echo -e "${GREEN}Test file created: test.c${NC}"
    echo -e "You can now run: ${GREEN}./compiler test.c output.s${NC}"
else
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi 