#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>

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
        if (!file) {
            throw std::runtime_error("Cannot open file: " + filename);
        }

        std::vector<int> numbers;
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

// ===== Фабрика фільтрів через реєстр =====

class FilterFactory {
    using FactoryFunction = std::function<std::unique_ptr<INumberFilter>(const std::string&)>;
    std::map<std::string, FactoryFunction> registry;

public:
    FilterFactory() {
        registry["EVEN"] = [](const std::string&) {
            return std::make_unique<EvenFilter>();
        };
        registry["ODD"] = [](const std::string&) {
            return std::make_unique<OddFilter>();
        };
        registry["GT"] = [](const std::string& input) {
            int threshold;
            std::istringstream iss(input.substr(2));
            if (!(iss >> threshold)) {
                throw std::invalid_argument("Invalid GT filter format: " + input);
            }
            return std::make_unique<GreaterThanFilter>(threshold);
        };
    }

    std::unique_ptr<INumberFilter> create_filter(const std::string& filter_str) const {
        for (const auto& [key, factory] : registry) {
            if (filter_str.rfind(key, 0) == 0) {
                return factory(filter_str);
            }
        }

        throw std::invalid_argument("Unknown filter: " + filter_str);
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
        FilterFactory factory;
        auto filter = factory.create_filter(filter_str);

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
