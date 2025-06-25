#ifndef JUMI_DEBUGGER_H
#define JUMI_DEBUGGER_H
#include "common.h"
#include "breakpoint.h"
#include "registers.h"
#include <cstdint>
#include <string>
#include <unordered_map>

class debugger {
public:
    debugger(std::string prog_name, pid_t pid);
    void run();
    void set_breakpoint_at_address(std::intptr_t addr);
    void delete_breakpoint_at_address(std::intptr_t addr);
    uint64_t get_register_value(pid_t pid, reg r);
    void set_register_value(pid_t pid, reg r, uint64_t value);
    uint64_t get_register_value_from_dwarf_register(pid_t pid, int regnum);
    std::string get_register_name(reg r);
    reg get_register_from_name(const std::string& name);
    void dump_registers();

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
