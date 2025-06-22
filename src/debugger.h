#ifndef JUMI_DEBUGGER_H
#define JUMI_DEBUGGER_H
#include "breakpoint.h"
#include <cstdint>
#include <string>
#include <unordered_map>

class debugger {
public:
    debugger(std::string prog_name, pid_t pid);
    void run();
    void set_breakpoint_at_address(std::intptr_t addr);

private:
    std::string _prog_name;
    pid_t _pid;
    bool _quit;
    std::unordered_map<std::intptr_t, breakpoint> _breakpoints;

    void handle_command(const std::string& line);

    void continue_execution();
    void quit() noexcept;
};

#endif
