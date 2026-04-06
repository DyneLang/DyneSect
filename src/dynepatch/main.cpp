#include <iostream>

// dynepatch: pre/post-processes source files to support __asm blocks in the
// Norcroft C++ compiler, bridging the gap between modern C++ constructs and
// the 1995 toolchain.

int main(int argc, char *argv[])
{
    std::cout << "dynepatch: Norcroft __asm patch tool\n";
    return 0;
}
