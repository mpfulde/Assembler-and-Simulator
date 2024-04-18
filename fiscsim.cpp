#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <vector>
#include <cstdint>
#include <iomanip>
#include <algorithm>

void decode_and_run(std::ifstream &file, int cycles, bool disassemble);

int get_register(int bits, int pos = 0);

void printReg(int reg);

# define bnz_instruct_bits 6
# define instruction_and_bits 63
# define reg_bits 2

// forcing a 6 bit program counter
class registers {

public:
    registers() {
        pc = {0};
        z = false;
    }
    unsigned int pc : 6; // limits pc to 6
    bool z;
    uint8_t regs[4]{0, 0, 0, 0};

};

int command_line_error(const std::string& str) {
    str.substr(1, str.find('/'));
    std::cerr << "Usage: " << str << " <object file> [cycles] [-d]\n";
    std::cerr << "\t\t-d : print disassembly listing with each cycle\n";
    std::cerr << "\t\tif cycles are unspecified the CPU will run for 20 "
                 "cycles\n";
    return 1;
}

int main(int argc, char **argv) {

    if (argc < 2 or argc > 4) {
        return command_line_error(argv[0]);
    }

    bool disassemble = false;

    int cycles = 20;
    std::string arg2str;
    std::string arg3str;
    if (argc > 2) {
        arg2str = std::string(argv[2]);
    }

    if (argc > 3) {
        arg3str = std::string(argv[3]);
    }

    if (argc > 2) {
        if (arg2str == "-d") {
            disassemble = true;
        } else if (std::all_of(arg2str.begin(), arg2str.end(),
                               ::isdigit)) {
            // if arg2 is a cycle number
            cycles = std::stoi(arg2str);
        } else if (not arg2str.empty()) {
            return command_line_error(argv[0]);
        }
    }

    if (argc > 3) {
        if (arg3str == "-d") {
            if (disassemble) {
                return command_line_error(argv[0]);
            } else {
                disassemble = true;
            }
        } else if (not arg3str.empty()) {
            return command_line_error(argv[0]);
        }
    }

    std::ifstream inputFile(argv[1]);
    if (!inputFile) {
        std::cerr << "Error opening input file: " << argv[1] << std::endl;
        return 1;
    }

    std::string line;
    getline(inputFile, line);
    if (line != "v2.0 raw") {
        std::cerr << "Invalid object file";
    }

    decode_and_run(inputFile, cycles, disassemble);

    inputFile.close();
    return 0;
}

void decode_and_run(std::ifstream &file, int cycles, bool disassemble) {
    std::string line;

    registers r;
    r.pc = 0;
    r.z = false;

    std::vector<std::string> lines;

    std::stringstream line_parse;
    uint8_t instruction;

    while (getline(file, line)) {
        lines.push_back(line);
    }

    for (auto cur_cycle = 1; cur_cycle <= cycles; cur_cycle++) {

        // example: pc attempts to 62 but line 62 is not defined
        if (r.pc >= lines.size()) {
            std::cerr << "Attempting to jump to code outside of instruction "
                         "memory " << std::endl;
            exit(1);
        }
        instruction = std::strtol(lines.at(r.pc).c_str(), nullptr, 16);

        int op = instruction >> bnz_instruct_bits;
        int remaining_bits = instruction & instruction_and_bits;
        int location; // will be modified when needed
        int inReg1;
        int inReg2;
        int destReg;
        switch(op) {
            case 0:
                // add op
                inReg1 = get_register(remaining_bits, 0);
                inReg2 = get_register(remaining_bits, 2);
                destReg = get_register(remaining_bits, 4);
                r.regs[destReg] = r.regs[inReg1] + r.regs[inReg2];
                r.z = r.regs[destReg] == 0;
                r.pc++;
                break;
            case 1:
                // and op
                inReg1 = get_register(remaining_bits, 0);
                inReg2 = get_register(remaining_bits, 2);
                destReg = get_register(remaining_bits, 4);

                r.regs[destReg] = r.regs[inReg1] & r.regs[inReg2];
                r.z = r.regs[destReg] == 0;
                r.pc++;
                break;
            case 2:
                // not op
                inReg1 = get_register(remaining_bits, 0);
                destReg = get_register(remaining_bits, 4);

                r.regs[destReg] = ~r.regs[inReg1];
                r.z = r.regs[destReg] == 0;
                r.pc++;
                break;
            case 3:
                // bnz
                location = remaining_bits;
                if (not r.z) {
                    r.pc = location;
                } else {
                    r.pc++;
                    r.z = false;
                }


                break;
            default:
                std::cerr << "invalid operation with op code: " << op <<
                std::endl;
                exit(1);
        }


        std::cout << "Cycle:" << std::dec << cur_cycle << " ";
        std::cout << "State:PC:" << std::setw(2) << std::setfill('0') <<
                    std::hex << std::uppercase << static_cast<int>(r.pc);
        std::cout << " Z:" << (r.z ? 1 : 0 );
        std::cout << " R0: " << std::setw(2) << std::setfill('0') <<
                    std::hex << std::uppercase << static_cast<int>(r.regs[0]);
        std::cout << " R1: " << std::setw(2) << std::setfill('0') <<
                    std::hex << std::uppercase << static_cast<int>(r.regs[1]);
        std::cout << " R2: " << std::setw(2) << std::setfill('0') <<
                    std::hex << std::uppercase << static_cast<int>(r.regs[2]);
        std::cout << " R3: " << std::setw(2) << std::setfill('0') <<
                    std::hex << std::uppercase << static_cast<int>(r.regs[3]);
        std::cout << std::endl;
        if (disassemble) {
            std::cout << "Disassembly: ";
            switch(op) {
                case 0:
                    std::cout << "add ";
                    printReg(destReg);
                    std::cout << " ";
                    printReg(inReg1);
                    std::cout << " ";
                    printReg(inReg2);
                    std::cout << std::endl << std::endl;
                    break;
                case 1:
                    std::cout << "and ";
                    printReg(destReg);
                    std::cout << " ";
                    printReg(inReg1);
                    std::cout << " ";
                    printReg(inReg2);
                    std::cout << std::endl << std::endl;
                    break;
                case 2:
                    std::cout << "not ";
                    printReg(destReg);
                    std::cout << " ";
                    printReg(inReg1);
                    std::cout << std::endl << std::endl;
                    break;
                case 3:
                    std::cout << "bnz ";
                    std::cout << std::dec << remaining_bits;
                    std::cout << std::endl << std::endl;
                    break;
                default:
                    std::cerr << "Something went wrong during disassembly";
                    exit(1);
            }
        }
    }
}

void printReg(int reg) {
    switch (reg) {
        case 0:
            std::cout << "r0";
            break;
        case 1:
            std::cout << "r1";
            break;
        case 2:
            std::cout << "r2";
            break;
        case 3:
            std::cout << "r3";
            break;
        default:
            std::cerr << "Something went wrong during disassembly";
            exit(1);
    }
}

int get_register(int bits, int pos) {
    pos = 6 - pos - reg_bits;
    // 1 1 1 1 1 0
    // pos = 2
    // 1 1 1 1
    int reg = bits >> pos;
    reg = 3 & reg;


    return reg;
}