#include "breakpoint.h"
#include <sys/ptrace.h>
#include <sys/types.h>

breakpoint::breakpoint(pid_t pid, std::intptr_t addr)
        : _pid{pid}, _addr{addr}, _enabled{false}, _saved_data{}
    {}

void breakpoint::enable() {
    auto data = ptrace(PTRACE_PEEKDATA, _pid, _addr, nullptr);
    _saved_data = static_cast<uint8_t>(data & 0xFF);
    uint64_t int3 = 0xCC;
    uint64_t data_with_int3 = ((data & ~0xFF) | int3);
    ptrace(PTRACE_POKEDATA, _pid, _addr, data_with_int3);

    _enabled = true;
}

void breakpoint::disable() {
    auto data = ptrace(PTRACE_PEEKDATA, _pid, _addr, nullptr);
    auto restored_data = ((data & ~0xFF) | _saved_data);
    ptrace(PTRACE_POKEDATA, _pid, _addr, restored_data);

    _enabled = false;
}

bool breakpoint::is_enabled() const {
    return _enabled; 
}

std::intptr_t breakpoint::get_address() const {
    return _addr; 
}
