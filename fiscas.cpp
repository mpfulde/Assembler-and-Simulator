#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cctype>
#include <unordered_map>

void pass_one_parse_line(const std::string& line, uint8_t line_num);
bool is_operand(const std::string& word);

void pass_two_parse_line(std::string line);

std::unordered_map<std::string, std::uint8_t> label_map;


int command_line_error(char* str) {
    std::cerr << "Usage: " << str << " <source file> <object file> [-1]\n";
    std::cerr << "\t\t-l : print listing to standard error\n";
    return 1;
}


std::string str_tolower(const std::string& str) {
    std::string new_str;
    for (char c : str) {
        int new_c = std::tolower(c);

        new_str.push_back(static_cast<char>(new_c));
    }

    return new_str;
}

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4) {
        return command_line_error(argv[0]);
    }

    if (argc == 4) {
        if (std::strcmp(argv[3], "-l") != 0){
            return command_line_error(argv[0]);
        }
    }

    std::ifstream inputFile(argv[1]);
    if (!inputFile) {
        std::cerr << "Error opening input file: " << argv[1] << std::endl;
        return 1;
    }

    std::ostream* output = &std::cout; // Default to standard output
    std::ofstream outputFile;
    if (argc >= 3) {
        outputFile.open(argv[2]);
        if (!outputFile) {
            std::cerr << "Error opening output file: " << argv[2] << std::endl;
            return 1;
        }
        output = &outputFile; // Use file output if specified
    }



    std::string line;
    uint8_t line_num = 0;
    while (getline(inputFile, line)) {
        pass_one_parse_line(line, line_num);
        line_num++;
    }

    while (getline(inputFile, line)) {
        pass_two_parse_line(line);
    }


    inputFile.close();
    // outputFile is closed automatically when going out of scope

    return 0;
}



void pass_one_parse_line(const std::string& line, uint8_t line_num) {
    std::string word;
    std::istringstream line_to_read(line);

    //  check if first word is one of the 4 operands (ie if its a lable or op)
    line_to_read >> word;

    // checks if the first word is a comment
    if (word.front() == ';'){
        return;
    }
    if (not is_operand(word)) {
        // remove last character from the string
        if (word.back() == ':') {
            word.pop_back();
        }
        label_map[word] = line_num;
    }

    // thats kinda all pass one does (in line label is not definition)
}

void pass_two_parse_line(std::string line) {
    std::string word;
    std::istringstream line_to_read(line);

    line_to_read >> word;
    while(not is_operand(word)){

    }
}

bool is_operand(const std::string& word) {
    std::string lower_word = str_tolower(word);
    if (word == "add" or word == "and" or word == "not" or word == "bnz") {
        return true;
    }

    return false;
}
