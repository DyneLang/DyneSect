#include <iostream>

// dynesect: reads the Newton AIF and REx images, analyses the ROM content,
// and writes disassembled source files for reassembly with the Norcroft toolchain.

// -DDSECT_TOOLS_PATH=/usr/local/bin
// -DDSECT_DEBUG_IMAGE_PATH=/Users/matt/dev/Newton/ROMs/MP2x00 US
// -DDSECT_DDK_INCLUDE_PATH=/Users/matt/dev/Newton/NewtonDev/NewtonDev.ux/Cpp/NCT_Projects/DDKIncludes
// -DDSECT_WORK_PATH=/Users/matt/dev/DyneSect.git/work
// Senior CirrusNoDebug image
// Senior CirrusNoDebug high

int main(int argc, char *argv[])
{
    std::cout << "dynesect: Newton ROM dissection tool\n";
    std::cout << "DSECT_TOOLS_PATH=" << DSECT_TOOLS_PATH << "\n";
    return 0;
}
