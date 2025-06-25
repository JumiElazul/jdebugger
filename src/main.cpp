#include <filesystem>
#include <iostream>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/personality.h>
#include "debugger.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Program name not specified\n";
        return -1;
    }

    if (!std::filesystem::exists(argv[1])) {
        std::cerr << "Program name doesn't exist\n";
        return -1;
    }

    auto prog = argv[1];

    auto pid = fork();
    if (pid == 0) {
        //we're in the child process
        //execute debugee
        long result = ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
        if (result == -1) {
            std::cerr << "ptrace failed\n";
            return -1;
        }
        personality(ADDR_NO_RANDOMIZE);
        execl(prog, prog, nullptr);
    }
    else if (pid >= 1) {
        //we're in the parent process
        //execute debugger
        std::cout << "Started debugging process " << pid << '\n';
        debugger dbg{prog, pid};
        dbg.run();
    }
}
