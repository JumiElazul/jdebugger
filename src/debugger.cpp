#include "debugger.h"
#include "linenoise.h"
#include "registers.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>

debugger::debugger(std::string prog_name, pid_t pid)
    : _prog_name{std::move(prog_name)}, _pid{pid}, _quit(false), _breakpoints() {}

static std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> out;
    std::stringstream ss{s};
    std::string item;

    while (std::getline(ss, item, delimiter)) {
        out.push_back(item);
    }

    return out;
}

static bool is_prefix(const std::string& s, const std::string& of) {
    if (s.size() > of.size()) {
        return false;
    }

    return std::equal(s.begin(), s.end(), of.begin());
}

void debugger::run() {
    int wait_status;
    int options = 0;
    waitpid(_pid, &wait_status, options);

    char* line = nullptr;
    while ((line = linenoise("minidbg> ")) != nullptr) {
        handle_command(line);
        linenoiseHistoryAdd(line);
        linenoiseFree(line);

        if (_quit) {
            break;
        }
    }
}

void debugger::set_breakpoint_at_address(std::intptr_t addr) {
    std::cout << "Set breakpoint at address 0x" << std::hex << addr << '\n';
    breakpoint bp{_pid, addr};
    bp.enable();
    _breakpoints[addr] = bp;
}

void debugger::delete_breakpoint_at_address(std::intptr_t addr) {
    auto find = _breakpoints.find(addr);
    if (find != _breakpoints.end()) {
        std::cout << "Deleting breakpoint at address 0x" << std::hex << addr << '\n';
        breakpoint& bp = find->second;
        bp.disable();
        return;
    }
    std::cout << "Breakpoint at address 0x" << std::hex << addr << " not found to delete.\n";
}

uint64_t debugger::get_register_value(pid_t pid, reg r) {
    user_regs_struct regs;
    ptrace(PTRACE_GETREGS, pid, nullptr, &regs);

    auto it = std::find_if(g_register_descriptors.begin(), g_register_descriptors.end(),
            [r](auto&& rd) { return rd.r == r; });

    // 1. Cast &regs -> uint64_t* so we can do word indexed access.
    // 2. Compute the index of the requested register in the g_register_descriptors array.
    // 3. Offset the pointer &regs by that index.
    // 4. Dereference it to get the 64-bit register value.
    return *(reinterpret_cast<uint64_t*>(&regs) + (it - begin(g_register_descriptors)));
}

void debugger::set_register_value(pid_t pid, reg r, uint64_t value) {
    user_regs_struct regs;
    ptrace(PTRACE_GETREGS, pid, nullptr, &regs);

    auto it = std::find_if(g_register_descriptors.begin(), g_register_descriptors.end(),
            [r](auto&& rd) { return rd.r == r; });

    // 1. Cast &regs -> uint64_t* so we can do word indexed access.
    // 2. Compute the index of the requested register in the g_register_descriptors array.
    // 3. Offset the pointer &regs by that index.
    // 4. Dereference it to get the 64-bit register value, and assign it the new value.
    *(reinterpret_cast<uint64_t*>(&regs) + (it - begin(g_register_descriptors))) = value;
    ptrace(PTRACE_SETREGS, pid, nullptr, &regs);
}

uint64_t debugger::get_register_value_from_dwarf_register(pid_t pid, int regnum) {
    auto it = std::find_if(g_register_descriptors.begin(), g_register_descriptors.end(), 
            [regnum](auto&& rd) { return rd.dwarf_r == regnum; });

    return get_register_value(pid, it->r);
}

std::string debugger::get_register_name(reg r) {
    auto it = std::find_if(g_register_descriptors.begin(), g_register_descriptors.end(),
            [r](auto&& rd) { return rd.r == r; });

    return it->name;
}

reg debugger::get_register_from_name(const std::string& name) {
    auto it = std::find_if(g_register_descriptors.begin(), g_register_descriptors.end(),
            [&name](auto&& rd) { return rd.name == name; });

    return it->r;
}

void debugger::dump_registers() {
    for (const auto& rd : g_register_descriptors) {
        std::cout             <<
            rd.name           <<
            " 0x"             << 
            std::setfill('0') <<
            std::setw(16)     <<
            std::hex          <<
            get_register_value(_pid, rd.r) << '\n';
    }
}

void debugger::handle_command(const std::string& line) {
    auto args = split(line, ' ');
    auto command = args[0];

    if (is_prefix(command, "cont")) {
        continue_execution();
    } else if (is_prefix(command, "register")) {
        if (is_prefix(args[1], "dump")) {
            dump_registers();
        } else if (is_prefix(args[1], "read")) {
            std::cout << get_register_value(_pid, get_register_from_name(args[2])) << '\n';
        } else if (is_prefix(args[1], "write")) {
            std::string val { args[3], 2 };
            set_register_value(_pid, get_register_from_name(args[2]), std::stoull(val, 0, 16ULL));
        }
    } else if (is_prefix(command, "break")) {
        std::string addr{args[1], 2}; //naively assume that the user has written 0xADDRESS
        set_breakpoint_at_address(std::stol(addr, 0, 16));
    } else if (is_prefix(command, "del")) {
        std::string addr{args[1], 2};
        delete_breakpoint_at_address(std::stol(addr, 0, 16));
    } else if (is_prefix(command, "quit")) {
        quit();
    } else {
        std::cerr << "Unknown command\n";
    }
}

void debugger::continue_execution() {
    ptrace(PTRACE_CONT, _pid, nullptr, nullptr);

    int wait_status;
    int options = 0;
    waitpid(_pid, &wait_status, options);
}

void debugger::quit() noexcept {
    _quit = true;
}

