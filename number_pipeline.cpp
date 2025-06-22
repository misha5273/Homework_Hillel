#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>

// ===== Інтерфейси =====

class INumberReader {
public:
    virtual std::vector<int> read_numbers(const std::string& filename) = 0;
    virtual ~INumberReader() = default;
};

class INumberFilter {
public:
    virtual bool keep(int number) const = 0;
    virtual ~INumberFilter() = default;
};

class INumberObserver {
public:
    virtual void on_number(int number) = 0;
    virtual void on_finished() = 0;
    virtual ~INumberObserver() = default;
};

// ===== Реалізація зчитування =====

class FileNumberReader : public INumberReader {
public:
    std::vector<int> read_numbers(const std::string& filename) override {
        std::ifstream file(filename);
        std::vector<int> numbers;

        if (!file) {
            std::cerr << "Error: Cannot open file " << filename << std::endl;
            return numbers;
        }

        int num;
        while (file >> num) {
            numbers.push_back(num);
        }

        return numbers;
    }
};

// ===== Реалізації фільтрів =====

class EvenFilter : public INumberFilter {
public:
    bool keep(int number) const override {
        return number % 2 == 0;
    }
};

class OddFilter : public INumberFilter {
public:
    bool keep(int number) const override {
        return number % 2 != 0;
    }
};

class GreaterThanFilter : public INumberFilter {
    int threshold;
public:
    explicit GreaterThanFilter(int t) : threshold(t) {}

    bool keep(int number) const override {
        return number > threshold;
    }
};

// ===== Обсервери =====

class PrintObserver : public INumberObserver {
public:
    void on_number(int number) override {
        std::cout << number << std::endl;
    }

    void on_finished() override {}
};

class CountObserver : public INumberObserver {
    int count = 0;
public:
    void on_number(int number) override {
        ++count;
    }

    void on_finished() override {
        std::cout << "Total numbers passed filter: " << count << std::endl;
    }
};

// ===== Фабрика фільтрів =====

class FilterFactory {
public:
    static std::unique_ptr<INumberFilter> create_filter(const std::string& filter_str) {
        if (filter_str == "EVEN") {
            return std::make_unique<EvenFilter>();
        } else if (filter_str == "ODD") {
            return std::make_unique<OddFilter>();
        } else if (filter_str.rfind("GT", 0) == 0) {
            int threshold;
            std::istringstream iss(filter_str.substr(2));
            if (!(iss >> threshold)) {
                throw std::invalid_argument("Invalid GT filter format");
            }
            return std::make_unique<GreaterThanFilter>(threshold);
        } else {
            throw std::invalid_argument("Unknown filter: " + filter_str);
        }
    }
};

// ===== Обробник чисел =====

class NumberProcessor {
    INumberReader& reader;
    INumberFilter& filter;
    std::vector<INumberObserver*> observers;

public:
    NumberProcessor(INumberReader& r, INumberFilter& f, const std::vector<INumberObserver*>& obs)
        : reader(r), filter(f), observers(obs) {}

    void run(const std::string& filename) {
        auto numbers = reader.read_numbers(filename);

        for (int number : numbers) {
            if (filter.keep(number)) {
                for (auto* obs : observers) {
                    obs->on_number(number);
                }
            }
        }

        for (auto* obs : observers) {
            obs->on_finished();
        }
    }
};

// ===== main =====

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: ./number_pipeline <FILTER> <FILENAME>" << std::endl;
        std::cerr << "Example: ./number_pipeline EVEN numbers.txt" << std::endl;
        return 1;
    }

    std::string filter_str = argv[1];
    std::string filename = argv[2];

    try {
        auto filter = FilterFactory::create_filter(filter_str);
        FileNumberReader reader;

PrintObserver printObserver;
        CountObserver countObserver;
        std::vector<INumberObserver*> observers = { &printObserver, &countObserver };

        NumberProcessor processor(reader, *filter, observers);
        processor.run(filename);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 2;
    }

    return 0;
}
