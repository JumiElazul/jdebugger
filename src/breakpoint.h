#ifndef JUMI_BREAKPOINT_H
#define JUMI_BREAKPOINT_H
#include <cstdint>
#include <sys/types.h>

class breakpoint {
public:
    breakpoint() = default;
    breakpoint(pid_t pid, std::intptr_t addr);

    void enable();
    void disable();

    [[nodiscard]] bool is_enabled() const;
    [[nodiscard]] std::intptr_t get_address() const;

private:
    pid_t _pid;
    std::intptr_t _addr;
    bool _enabled;
    uint8_t _saved_data;
};

#endif
