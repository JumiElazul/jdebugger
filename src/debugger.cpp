#include "debugger.h"
#include <iostream>
#include <linenoise.h>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <sys/ptrace.h>

debugger::debugger(std::string prog_name, pid_t pid)
    : _prog_name{std::move(prog_name)}, _pid{pid}, _quit(false) {}

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

void debugger::handle_command(const std::string& line) {
    auto args = split(line, ' ');
    auto command = args[0];

    if (is_prefix(command, "continue")) {
        continue_execution();
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

