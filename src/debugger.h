#ifndef JUMI_DEBUGGER_H
#define JUMI_DEBUGGER_H
#include <string>

class debugger {
public:
    debugger(std::string prog_name, pid_t pid);
    void run();

private:
    std::string _prog_name;
    pid_t _pid;
    bool _quit;

    void handle_command(const std::string& line);

    void continue_execution();
    void quit() noexcept;
};

#endif
