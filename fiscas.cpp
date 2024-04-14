#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cctype>
#include <vector>
#include <cstdint>
#include <iomanip>
#include <algorithm>

class LabelMap {
public:
    LabelMap() = default;

    void insert(const std::string &key, uint8_t val) {
        map_tool.emplace_back(key, val);
    }

    void print_labels() {
        for (const auto &label: map_tool) {
            std::cout << label.first << " \t";
            std::cout << std::setw(2) << std::setfill('0') << std::hex
                      << std::uppercase << label.second << std::endl;
        }
    }

    uint8_t get_val(const std::string &key) {
        for (auto &i: map_tool) {
            if (i.first == key) {
                int value = i.second;
                return value;
            }
        }

        throw std::out_of_range("No label found for the inserted key");
    }

private:
    std::vector<std::pair<std::string, int>> map_tool;
};

void pass_one_parse_line(const std::string &line, int line_num, LabelMap &map);

bool is_operand(const std::string &word);

uint8_t pass_two_parse_line(std::string line, LabelMap &map);

int get_operation(const std::string &operation);

int num_opers(int op);

std::vector<int>
get_oper_bits(const std::vector<std::string> &opers, LabelMap &map);


int command_line_error(char *str) {
    std::cerr << "Usage: " << str << " <source file> <object file> [-1]\n";
    std::cerr << "\t\t-l : print listing to standard error\n";
    return 1;
}


std::string str_tolower(const std::string &str) {
    std::string new_str;
    for (char c: str) {
        int new_c = std::tolower(c);

        new_str.push_back(static_cast<char>(new_c));
    }

    return new_str;
}

bool check_line(const std::string &line) {
    std::stringstream line_checker = std::stringstream(line);
    std::string checker;
    line_checker >> checker;
    return checker.front() == ';';
}

int main(int argc, char *argv[]) {


    if (argc < 3 || argc > 4) {
        return command_line_error(argv[0]);
    }

    bool listing = false;
    if (argc == 4) {
        if (std::strcmp(argv[3], "-l") != 0) {
            return command_line_error(argv[0]);
        } else {
            listing = true;
        }
    }

    std::ifstream inputFile(argv[1]);
    if (!inputFile) {
        std::cerr << "Error opening input file: " << argv[1] << std::endl;
        return 1;
    }

    LabelMap label_map;

    std::string line;
    int line_num = 0;
    while (getline(inputFile, line)) {
        if (check_line(line)) {
            // we skip comments
            continue;
        }
        pass_one_parse_line(line, line_num, label_map);
        line_num++;
    }

    inputFile = std::ifstream(argv[1]);

    line_num = 0;
    std::vector<int> result_list;
    while (getline(inputFile, line)) {
        if (check_line(line)) {
            // we skip comments
            continue;
        }
        int result = pass_two_parse_line(line, label_map);

        result_list.push_back(result);
        line_num++;
    }

    if (listing) {

        std::cout << "*** LABEL LIST ***" << std::endl;
        label_map.print_labels();
        std::cout << "*** MACHINE PROGRAM ***" << std::endl;

        std::string word;
        line_num = 0;
        inputFile = std::ifstream(argv[1]);
        while (getline(inputFile, line)) {
            if (check_line(line)) {
                // we skip comments
                continue;
            }
            std::istringstream line_to_parse(line);
            std::ostringstream output_line;

            // gets rid of the label
            line_to_parse >> word;
            if (is_operand(word)) {
                line_to_parse = std::istringstream(line);
            }

            output_line << std::setw(2) << std::setfill('0') << std::hex
                        << std::uppercase << line_num << ":" << std::setw(2)
                        << std::setfill('0') << result_list[line_num]
                        << "\t";
            while (line_to_parse >> word) {
                if (word.find(';') != std::string::npos) {
                    output_line;
                    break;
                }
                if (not is_operand(word)) {
                    output_line << " ";
                }
                output_line << word;
            }
            output_line << std::endl;
            std::cout << output_line.str();
            line_num++;
        }
    }


    std::ostream *output; // Default to standard output
    std::ofstream outputFile;
    outputFile.open(argv[2]);
    if (!outputFile) {
        std::cerr << "Error opening output file: " << argv[2] << std::endl;
        return 1;
    }
    output = &outputFile;// Use file output if specified

    (*output) << "v2.0 raw" << std::endl;
    for (auto hex: result_list) {
        (*output) << std::setw(2) << std::setfill('0') << std::hex <<
                  std::uppercase << hex << std::endl;
    }

    inputFile.close();
    // outputFile is closed automatically when going out of scope

    return 0;
}


void
pass_one_parse_line(const std::string &line, int line_num, LabelMap &map) {
    std::string word;
    std::istringstream line_to_read(line);

    //  check if first word is one of the 4 operands (ie if its a lable or op)
    line_to_read >> word;

    // checks if the first word is a comment
    if (word.front() == ';') {
        return;
    }
    if (not is_operand(word)) {
        // remove last character from the string
        if (word.back() == ':') {
            word.pop_back();
        }
        map.insert(word, line_num);
    }
}

uint8_t pass_two_parse_line(std::string line, LabelMap &map) {
    std::string word;
    std::istringstream line_to_read(line);

    line_to_read >> word;
    if (not is_operand(word)) {
        auto temp = word;
        line_to_read >> word;
        if (not is_operand(word)) {
            std::cerr << temp << " is an invalid operation" << std::endl;
            exit(1);
        }
    }

    int op = get_operation(word);
    if (op > 3 or op < 0) {
        std::cerr << word << " is an invalid operation" << std::endl;
        exit(1);
    }

    int num_operands = num_opers(op);
    std::vector<std::string> operands;
    while (line_to_read >> word) {
        if (word.find(';') != std::string::npos) {
            break;
        }

        operands.push_back(word);
    }

    if (static_cast<int>(operands.size()) != num_operands) {
        std::cerr << "Invalid number of operations: " << operands.size()
                  << ", when there should be: " << num_operands << std::endl;
        exit(1);
    }

    std::vector<int> operand_bits = get_oper_bits(operands, map);
    int dest_bit = operand_bits.front();
    operand_bits.erase(operand_bits.begin());
    operand_bits.push_back(dest_bit);
    uint8_t line_byte = 0;
    line_byte = line_byte << 2 | op;
    if (num_operands == 1) {
        line_byte = line_byte << 6 | operand_bits.front();
    } else if (num_operands == 2) {
        line_byte = line_byte << 2 | operand_bits.front();
        line_byte = line_byte << 2 | 0; // makes sure these two bits are 0
        line_byte = line_byte << 2 | operand_bits.back();
    } else {
        for (auto b: operand_bits) {
            line_byte = line_byte << 2 | b;
        }
    }

    return line_byte;
}

std::vector<int>
get_oper_bits(const std::vector<std::string> &opers, LabelMap &map) {
    std::vector<int> bits;
    if (static_cast<int>(opers.size()) == 1) {
        int val = map.get_val(opers[0]);
        bits.push_back(val);
    } else {
        for (auto oper: opers) {
            oper = str_tolower(oper);
            if (oper == "r0") {
                bits.push_back(0);
            } else if (oper == "r1") {
                bits.push_back(1);
            } else if (oper == "r2") {
                bits.push_back(2);
            } else if (oper == "r3") {
                bits.push_back(3);
            } else {
                std::cerr << "invalid argument " << oper;
                exit(1);
            }
        }
    }
    return bits;
}

int num_opers(int op) {
    if (op == 0 or op == 1) {
        return 3;
    } else if (op == 2) {
        return 2;
    } else if (op == 3) {
        return 1;
    }

    return -1;
}

int get_operation(const std::string &operation) {
    std::string op = str_tolower(operation);
    if (op == "add") {
        return 0;
    } else if (op == "and") {
        return 1;
    } else if (op == "not") {
        return 2;
    } else if (op == "bnz")
        return 3;
    return -1;
}

bool is_operand(const std::string &word) {
    std::string lower_word = str_tolower(word);
    if (word == "add" or word == "and" or word == "not" or word == "bnz") {
        return true;
    }

    return false;
}
