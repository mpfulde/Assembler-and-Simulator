#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cctype>
#include <unordered_map>
#include <vector>
#include <cstdint>

#define labelmaptype std::unordered_map<std::string, std::uint8_t>

void pass_one_parse_line(const std::string &line, uint8_t line_num, labelmaptype &map);
bool is_operand(const std::string &word);
uint8_t pass_two_parse_line(std::string line, labelmaptype &map);
int get_operation(const std::string &operation);
int num_opers(int op);
std::vector<int> get_oper_bits(const std::vector<std::string> &opers, labelmaptype &map);



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

    labelmaptype label_map;

    std::string line;
    uint8_t line_num = 0;
    while (getline(inputFile, line)) {
        pass_one_parse_line(line, line_num, label_map);
        line_num++;
    }

    inputFile = std::ifstream(argv[1]);

    if (listing) {
        std::cout << "*** LABEL LIST ***" << std::endl;
        for (auto label: label_map) {
            std::cout << label.first << "\t" << std::hex << std::uppercase  << label.second << std::endl;
        }
        std::cout << "*** MACHINE PROGRAM ***" << std::endl;
    }

    line_num = 0;
    std::vector<uint8_t> result_list;
    while (getline(inputFile, line)) {
        uint8_t result = pass_two_parse_line(line, label_map);
        if (listing) {
            std::string word;
            std::istringstream line_to_parse(line);
            std::ostringstream output_line;

            // gets rid of the label
            line_to_parse >> word;
            if (is_operand(word)) {
                line_to_parse = std::istringstream(line);
            }

            output_line << std::dec << line_num << ":" << std::hex << std::uppercase << result << "\t";
            while (line_to_parse >> word) {
                if (word.find(';') != std::string::npos) {
                    break;
                }
                output_line << word;
            }
            output_line << std::endl;
            std::cout << output_line.str();
        }

        result_list.push_back(result);
        line_num++;
    }

    std::ostream *output; // Default to standard output
    std::ofstream outputFile;
    outputFile.open(argv[2]);
    if (!outputFile) {
        std::cerr << "Error opening output file: " << argv[2] << std::endl;
        return 1;
    }
    output = &outputFile;// Use file output if specified

    (*output) << "v2.0 raw" << std::hex << std::uppercase << std::endl;
    for (auto hex : result_list) {
        (*output) << hex;
    }

    inputFile.close();
    // outputFile is closed automatically when going out of scope

    return 0;
}


void pass_one_parse_line(const std::string &line, uint8_t line_num, labelmaptype &map) {
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
        map[word] = line_num;
    }
}

uint8_t pass_two_parse_line(std::string line, labelmaptype &map) {
    std::string word;
    std::istringstream line_to_read(line);

    line_to_read >> word;
    while (not is_operand(word)) {
        line_to_read >> word;
    }

    int op = get_operation(word);
    if (op > 3 or op < 0) {
        std::cerr << word << " is an invalid operation";
        exit(1);
    }

    int num_operands = num_opers(op);
    std::vector<std::string> operands;
    while (word.front() != ';' or not line_to_read.eof()) {
        line_to_read >> word;

        operands.push_back(word);
    }

    if (static_cast<int>(operands.size()) != num_operands) {
        std::cerr << "Invalid number of operations: " << operands.size()
                  << ", when there should be: " << num_operands;
        exit(1);
    }

    std::vector<int> operand_bits = get_oper_bits(operands, map);
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

std::vector<int> get_oper_bits(const std::vector<std::string> &opers, labelmaptype &map) {
    std::vector<int> bits;
    if (static_cast<int>(opers.size()) == 1) {
        bits.push_back(map[opers.front()]);
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
