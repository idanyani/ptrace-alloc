#ifndef PTRACE_ALLOC_LOGGER_H
#define PTRACE_ALLOC_LOGGER_H

#include <iostream>

class Logger {
  public:
    enum class Verbosity { ON, OFF }; // can be extended (low-high etc.)

    explicit Logger(std::ostream& out = std::cout) : out_(out), verbosity_(Verbosity::ON) {}

    template<typename T>
    Logger& operator<<(const T& t) {
        if (verbosity_ == Verbosity::ON)
            out_ << t;
        return *this;
    }

    static constexpr class Endl {} endl = Endl();

    Logger& operator<<(Endl) {
        if (verbosity_ == Verbosity::ON)
            out_ << std::endl;
        return *this;
    }

    Logger& operator<<(Verbosity verbosity) {
        verbosity_ = verbosity;
        return *this;
    }

  private:
    std::ostream&   out_;
    Verbosity       verbosity_;
};

#endif //PTRACE_ALLOC_LOGGER_H
