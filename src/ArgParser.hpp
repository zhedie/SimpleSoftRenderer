#pragma once

#include <vector>
#include <string>
#include <functional>
#include <iostream>
#include <sstream>
#include <cstring>



struct ShortCircuitOption {
    std::string short_name;
    std::string long_name;
    std::string description;
    std::function<void(void)> callback;

    ShortCircuitOption(std::string sname, std::string lname, std::string description, std::function<void(void)> callback)
        : short_name(sname), long_name(lname), description(description), callback(callback) {}
};

struct Option {
    std::string short_name;
    std::string long_name;
    std::string description;
    std::string type;
    std::string value;

    Option(std::string sname, std::string lname, std::string description, std::string type, std::string value)
        : short_name(sname), long_name(lname), description(description), type(type), value(value) {}
};

struct Argument {
    std::string name;
    std::string description;
    std::string type;
    std::string value;

    Argument(std::string name, std::string description, std::string type, std::string value)
        : name(name), description(description), type(type), value(value) {}
};

template<typename T>
std::string to_string(T value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

template<typename T>
T cast_string(std::string value) {
    std::istringstream iss(value);
    T result;
    iss >> result;
    return result;
}

template<>
bool cast_string<bool>(std::string value) {
    for (auto &i : value) {
        i = tolower(i);
    }
    if (value == "true" || value == "1") {
        return true;
    }
    else if (value == "false" || value == "0") {
        return false;
    }
    else {
        std::cout << "Error: Get value failed!" << std::endl;
        std::cout << "Value does not match type." << std::endl;
        std::exit(-1);
    }
}

class ArgParser {
private:
    std::vector<ShortCircuitOption> sc_options;
    std::vector<Option> options;
    std::vector<Argument> arguments;
    std::vector<Argument> named_arguments;

    void check_option_name(const std::string &sname, const std::string &lname) {
        if (sname.size() == 0 && lname.size() == 0) {
            std::cout << "Error: Add option failed!" << std::endl;
            std::cout << "The long name and the short name of option can't be empty at same time." << std::endl;
            std::exit(-1);
        }
        // check format
        if (sname.size() != 0) {
            if (sname.size() < 2 || sname[0] != '-' || sname[1] == '-') {
                std::cout << "Error: Add option failed!" << std::endl;
                std::cout << "The short name of option must be '-' followed by a character(except '-')." << std::endl;
                std::exit(-1);
            }
        }
        if (lname.size() != 0) {
            if (lname.size() < 3 || lname.substr(0, 2) != "--" || lname[2] == '-') {
                std::cout << "Error: Add option failed!" << std::endl;
                std::cout << "The long name of option must be \"--\" followed by several characters(except '-')." << std::endl;
                std::exit(-1);
            }
        }
        // check existence
        for (const auto &opt : sc_options) {
            if (opt.short_name == sname) {
                std::cout << "Error: Add option failed!" << std::endl;
                std::cout << "The short name of option(" << sname << ") has already existed." << std::endl;
                std::exit(-1);
            }
            if (opt.long_name == lname) {
                std::cout << "Error: Add option failed!" << std::endl;
                std::cout << "The long name of option(" << lname << ") has already existed." << std::endl;
                std::exit(-1);
            }
        }
        for (const auto &opt : options) {
            if (opt.short_name == sname) {
                std::cout << "Error: Add option failed!" << std::endl;
                std::cout << "The short name of option(" << sname << ") has already existed." << std::endl;
                std::exit(-1);
            }
            if (opt.long_name == lname) {
                std::cout << "Error: Add option failed!" << std::endl;
                std::cout << "The long name of option(" << lname << ") has already existed." << std::endl;
                std::exit(-1);
            }
        }
    }

    void check_argument_name(const std::string &name) {
        for (const auto &arg : arguments) {
            if (arg.name == name) {
                std::cout << "Error: Add argument failed!" << std::endl;
                std::cout << "Argument name(" << name << ") has already existed." << std::endl;
                std::exit(-1);
            }
        }
        for (const auto &arg : named_arguments) {
            if (arg.name == name) {
                std::cout << "Error: Add argument failed!" << std::endl;
                std::cout << "Argument name(" << name << ") has already existed." << std::endl;
                std::exit(-1);
            }
        }
    }

    void parse_sc_options(int argc, char **argv, std::vector<bool> &visit) {
        for (int i = 0; i < argc; ++i) {
            if (!visit[i]) {
                if (!strcmp(argv[i], "--")) {
                    for (const auto &scopt : sc_options) {
                        if (scopt.long_name == argv[i]) {
                            scopt.callback();
                            std::exit(0);
                        }
                    }
                }
                else if (argv[i][0] == '-') {
                    if (strlen(argv[i]) == 2) {
                        // The aggregation of the short name of short circuit options is not supported.
                        for (const auto &scopt : sc_options) {
                            if (scopt.short_name.back() == argv[i][1]) {
                                scopt.callback();
                                std::exit(0);
                            }
                        }
                    }
                }
            }
        }
    }

    void parse_options(int argc, char **argv, std::vector<bool> &visit) {
        for (int i = 0; i < argc; ++i) {
            if (!visit[i]) {
                if (!strncmp(argv[i], "--", 2)) {
                    for (auto &opt : options) {
                        if (opt.long_name == argv[i]) {
                            if (opt.type == "bool") {
                                opt.value = "1";
                            }
                            else {
                                if (i + 1 >= argc || argv[i + 1][0] == '-') {
                                    std::cout << "Error: Parse failed!" << std::endl;
                                    std::cout << "Can not find value of option " << argv[i] << " in cmdline." << std::endl;
                                    std::exit(-1);
                                }
                                else {
                                    opt.value = argv[i + 1];
                                    visit[i + 1] = true;
                                }
                            }
                            visit[i] = true;
                        }
                    }
                }
                else if (argv[i][0] == '-') {
                    int len = static_cast<int>(strlen(argv[i]));
                    for (int ind = 1; ind < len; ++ind) {
                        bool is_found = false;
                        for (auto &opt : options) {
                            if (opt.short_name.back() == argv[i][ind]) {
                                if (opt.type != "bool") {
                                    if (i + 1 >= argc || argv[i + 1][0] == '-') {
                                        std::cout << "Error: Parse failed!" << std::endl;
                                        std::cout << "Can not find value of option " << argv[i] << " in cmdline." << std::endl;
                                        std::exit(-1);
                                    }
                                    else if (len > 2) {
                                        std::cout << "Error: Parse failed!" << std::endl;
                                        std::cout << "The aggregation of the short name of options which has value is not supported." << std::endl;
                                        std::exit(-1);
                                    }
                                    else {
                                        opt.value = argv[i + 1];
                                        visit[i + 1] = true;
                                        is_found = true;
                                        break;
                                    }
                                }
                                else {
                                    opt.value = "1";
                                    is_found = true;
                                    break;
                                }
                            }
                        }
                        if (is_found == false) {
                            std::cout << "Error: Parse failed!" << std::endl;
                            std::cout << "Unknown option " << argv[i] << "." << std::endl;
                            std::exit(-1);
                        }
                    }
                    visit[i] = true;
                }
            }
        }
    }

    void parse_named_arguments(int argc, char **argv, std::vector<bool> &visit) {
        for (int i = 0; i < argc; ++i) {
            if (!visit[i]) {
                const int len = static_cast<int>(strlen(argv[i]));
                for (int j = 0; j < len; ++j) {
                    if (argv[i][j] == '=') {
                        std::string str = std::string(argv[i]);
                        std::string name = str.substr(0, j);
                        std::string value = str.substr(j + 1);
                        bool is_found = false;
                        for (auto &arg : named_arguments) {
                            if (arg.name == name) {
                                arg.value = value;
                                is_found = true;
                                visit[i] = true;
                            }
                        }
                        if (!is_found) {
                            std::cout << "Error: Parse failed!" << std::endl;
                            std::cout << "Unknown named argument " << argv[i] << "." << std::endl;
                            std::exit(-1);
                        }
                        break;
                    }
                }
            }
        }
    }

    void parse_arguments(int argc, char **argv, std::vector<bool> &visit) {
        int cur_pos = 0;
        for (auto &arg : arguments) {
            while (visit[cur_pos] && cur_pos < argc) {
                ++cur_pos;
            }
            if (cur_pos >= argc) {
                return;
            }
            arg.value = argv[cur_pos];
            visit[cur_pos] = true;
        }
    }

public:
    ArgParser() {}

    template<typename T>
    void add_option(std::string sname, std::string lname, std::string description, T default_value) {
        check_option_name(sname, lname);
        if (typeid(T).name() == "bool") {
            std::cout << "Error: Add opion failed!" << std::endl;
            std::cout << "Do not support setting the option of BOOL type with the default value." << std::endl;
            std::exit(-1);
        }
        options.emplace_back(sname, lname, description, typeid(T).name(), to_string(default_value));
    }

    void add_option(std::string sname, std::string lname, std::string description) {
        check_option_name(sname, lname);
        options.emplace_back(sname, lname, description, "bool", "0");
    }

    void add_sc_option(std::string sname, std::string lname, std::string description, std::function<void(void)> callback) {
        check_option_name(sname, lname);
        sc_options.emplace_back(sname, lname, description, callback);
    }

    void add_argument(std::string name, std::string description, std::string type) {
        check_argument_name(name);
        arguments.emplace_back(name, description, type, "");
    }

    void add_named_argument(std::string name, std::string description, std::string type) {
        check_argument_name(name);
        named_arguments.emplace_back(name, description, type, "");
    }

    void parse(int argc, char** argv) {
        std::vector<bool> visit(argc, false);
        visit[0] = true;    // program name
        parse_sc_options(argc, argv, visit);
        parse_options(argc, argv, visit);
        parse_named_arguments(argc, argv, visit);
        parse_arguments(argc, argv, visit);
        for (int i = 0; i < argc; ++i) {
            if (!visit[i]) {
                std::cout << "Error: Parse failed!" << std::endl;
                std::cout << "Excess argument " << argv[i] << std::endl;
                std::exit(-1);
            }
        }
        for (const auto &i : named_arguments) {
            if (i.value.size() == 0) {
                std::cout << "Error: Parse failed!" << std::endl;
                std::cout << "Missing argument [" << i.name << "]" << std::endl;
                std::exit(-1);
            }
        }
        for (const auto &i : arguments) {
            if (i.value.size() == 0) {
                std::cout << "Error: Parse failed!" << std::endl;
                std::cout << "Missing argument [" << i.name << "]" << std::endl;
                std::exit(-1);
            }
        }
    }

    template<typename T>
    T get_option(std::string name) {
        for (const auto &opt : options) {
            if (opt.short_name == name || opt.long_name == name) {
                return cast_string<T>(opt.value);
            }
        }
        std::cout << "Error: Can not find option by name \"" << name << "\"!" << std::endl;
        std::exit(-1);
    }

    template<typename T>
    T get_argument(std::string name) {
        for (const auto &arg : named_arguments) {
            if (arg.name == name) {
                return cast_string<T>(arg.value);
            }
        }
        for (const auto &arg : arguments) {
            if (arg.name == name) {
                return cast_string<T>(arg.value);
            }
        }
        std::cout << "Error: Can not find option by name \"" << name << "\"!" << std::endl;
        std::exit(-1);
    }
};

