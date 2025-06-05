#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <algorithm>
#include <cctype>

struct LogSink {
    virtual void write(const std::string& msg) = 0;
    virtual ~LogSink() = default;
};

class ConsoleSink : public LogSink {
public:
    void write(const std::string& msg) override {
        std::cout << "[Console] " << msg << std::endl;
    }
};

class FileSink : public LogSink {
public:
    void write(const std::string& msg) override {
        std::ofstream file("app.log", std::ios::app);
        if (file.is_open()) {
            file << "[File] " << msg << std::endl;
        }
    }
};

class NullSink : public LogSink {
public:
    void write(const std::string& msg) override {
    }
};

enum class SinkType { CONSOLE, FILE, NONE };

class Logger {
public:
    static Logger& instance() {
        static Logger instance;
        return instance;
    }

    void set_sink(SinkType type) {
        switch (type) {
            case SinkType::CONSOLE:
                sink_ = std::make_unique<ConsoleSink>();
                std::cout << "Log sink set to CONSOLE.\n";
                break;
            case SinkType::FILE:
                sink_ = std::make_unique<FileSink>();
                std::cout << "Log sink set to FILE.\n";
                break;
            case SinkType::NONE:
                sink_ = std::make_unique<NullSink>();
                std::cout << "Log sink set to NONE (no output).\n";
                break;
        }
    }

    void log(const std::string& msg) {
        if (sink_) {
            sink_->write(msg);
        }
    }

private:
    std::unique_ptr<LogSink> sink_;
    Logger() {
        set_sink(SinkType::CONSOLE);
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

SinkType parse_sink_type(const std::string& input) {
    std::string s = input;
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return std::tolower(c);
    });

    if (s == "console") return SinkType::CONSOLE;
    if (s == "file") return SinkType::FILE;
    if (s == "none") return SinkType::NONE;

    throw std::invalid_argument("Unknown sink type: " + input);
}

int main(int argc, char* argv[]) {
    try {
        SinkType type = SinkType::CONSOLE;
        if (argc >= 2) {
            type = parse_sink_type(argv[1]);
        } else {
            std::cout << "No sink type specified. Using default: CONSOLE.\n";
        }

        Logger::instance().set_sink(type);
        Logger::instance().log("Test message 1");
        Logger::instance().log("Test message 2");
        Logger::instance().log("Test message 3");

        std::cout << "Logging complete.\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        std::cerr << "Valid options: console, file, none\n";
        return 1;
    }

    return 0;
}
