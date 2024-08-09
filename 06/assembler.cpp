#include <iostream>
#include <fstream>
#include <sstream>
#include <bitset>
#include <algorithm>
#include <filesystem>
#include <unordered_map>

// Define Parser class to handle parsing of assembly instructions
class Parser {
public:
    Parser(const std::string& filename) {
        input_file.open(filename);

        if (!input_file.is_open()) {
            std::cerr << "Error: Unable to open input file.\n";
        }
    }

    bool hasMoreLines() {
        return !input_file.eof();
    }

    // Method to read the next instruction from the input file
    void advance() {
        while (true) {
            if (!hasMoreLines()) break;
            std::getline(input_file, current_instruction);

            // Remove leading and trailing whitespaces
            size_t start = current_instruction.find_first_not_of(" \t");
            size_t end = current_instruction.find_last_not_of(" \t");

            if (start != std::string::npos && end != std::string::npos) {
                current_instruction = current_instruction.substr(start, end - start + 1);
            }
            else {
                continue;
            }

            // Check for comments and remove them
            size_t comment_pos = current_instruction.find("//");
            if (comment_pos != std::string::npos) {
                current_instruction = current_instruction.substr(0, comment_pos);
            }

            // > 1 accounts for newline '\n' character in each line. Also all commands have length > 1.
            if (current_instruction.size() > 1) {
                // std::cout<<"advance stops on "<<current_instruction<<"\n";
                break;
            }
        }

        // remove whitespaces
        std::string::iterator end_pos = std::remove(current_instruction.begin(), current_instruction.end(), ' ');
        current_instruction.erase(end_pos, current_instruction.end());

        // remove return / newline
        while ((current_instruction.back() == '\r') || (current_instruction.back() == '\n'))
            current_instruction.pop_back();
    }

    const int instructionType() {
        // @symbol
        if (current_instruction[0] == '@') {
            return 0;
        }
        // (loop)
        else if (current_instruction[0] == '(' && current_instruction.back() == ')') {
            return 1;
        }
        // dest=comp;jump
        else {
            return 2;
        }
    }

    std::string symbol() {
        if (instructionType() == 0) {
            return current_instruction.substr(1, current_instruction.size()-1);
        }
        else if (instructionType() == 1) {
            return current_instruction.substr(1, current_instruction.size()-2);
        }
        else {
            std::cerr << "Invalid call to symbol() for C_INSTRUCTION\n";
        }
    }

    std::string getCurrentInstruction() {
        // std::cout<<"Parser returns "<<current_instruction<<"\n";
        return current_instruction;
    }

private:
    std::ifstream input_file;
    std::string current_instruction;
};

// Define Code class to handle translation of assembly instructions to binary code
class Code {
public:
    // Method to translate a C-instruction to binary code
    std::string translateCInstruction(const std::string& instruction) {
        std::unordered_map<std::string, std::string> comp_codes = {
                    {"0",   "0101010"},
                    {"1",   "0111111"},
                    {"-1",  "0111010"},
                    {"D",   "0001100"},
                    {"A",   "0110000"},
                    {"M",   "1110000"},
                    {"!D",  "0001101"},
                    {"!A",  "0110001"},
                    {"!M",  "1110001"},
                    {"-D",  "0001111"},
                    {"-A",  "0110011"},
                    {"-M",  "1110011"},
                    {"D+1", "0011111"},
                    {"A+1", "0110111"},
                    {"M+1", "1110111"},
                    {"D-1", "0001110"},
                    {"A-1", "0110010"},
                    {"M-1", "1110010"},
                    {"D+A", "0000010"},
                    {"D+M", "1000010"},
                    {"D-A", "0010011"},
                    {"D-M", "1010011"},
                    {"A-D", "0000111"},
                    {"M-D", "1000111"},
                    {"D&A", "0000000"},
                    {"D&M", "1000000"},
                    {"D|A", "0010101"},
                    {"D|M", "1010101"}
                };

                // Map for the destination part
                std::unordered_map<std::string, std::string> dest_codes = {
                    {"",    "000"},
                    {"M",   "001"},
                    {"D",   "010"},
                    {"DM",  "011"},
                    {"MD",  "011"},
                    {"A",   "100"},
                    {"AM",  "101"},
                    {"MA",  "101"},
                    {"AD",  "110"},
                    {"DA",  "110"},
                    {"ADM", "111"},
                    {"AMD", "111"},
                    {"DAM", "111"},
                    {"DMA", "111"},
                    {"MDA", "111"},
                    {"MAD", "111"}
                };

                // Map for the jump part
                std::unordered_map<std::string, std::string> jump_codes = {
                    {"",    "000"},
                    {"JGT", "001"},
                    {"JEQ", "010"},
                    {"JGE", "011"},
                    {"JLT", "100"},
                    {"JNE", "101"},
                    {"JLE", "110"},
                    {"JMP", "111"}
                };

                size_t equalsPos = instruction.find('=');
                size_t semicolonPos = instruction.find(';');

                // std::cout<<jump_codes[""]<<" kk\n";

                std::string destPart, compPart, jumpPart;

                if (equalsPos == std::string::npos){
                    destPart = "";
                    compPart = instruction.substr(0, semicolonPos);
                    jumpPart = instruction.substr(semicolonPos + 1);
                    // std::cout<<" 11 "<<destPart<<" "<<compPart<<" "<<jumpPart<<"\n";
                }

                else if (semicolonPos == std::string::npos){
                    destPart = instruction.substr(0, equalsPos);
                    compPart = instruction.substr(equalsPos + 1);
                    jumpPart = "";
                    // std::cout<<"22 "<<destPart<<" "<<compPart<<" "<<jumpPart<<"\n";
                }

                else {
                    std::cerr<<"Both = and ; can't be used in the same instruction!\n";
                }

                // std::cout<<"33 "<<destPart<<", "<<compPart<<", "<<jumpPart<<"\n";

                // Get the binary codes for each part
                std::string destCode = dest_codes[destPart];
                std::string compCode = comp_codes[compPart];
                std::string jumpCode = jump_codes[jumpPart];

                // std::cout<<destPart<<" "<<destCode<<" dest\n"<<compPart<<" "<<compCode<<" comp\n"<<jumpPart<<" "<<jumpCode<<" jump\n\n"; 
                // Combine the binary codes

        return "111" + compCode + destCode + jumpCode;
    }

    // Method to translate an A-instruction to binary code
    std::string translateAInstruction(const std::string& instruction, std::unordered_map<std::string, int>& symbolTable) {
        std::string symbol = instruction.substr(1);

        int address;

        // Check if symbol is numeric
        if (std::all_of(symbol.begin(), symbol.end(), ::isdigit)) {
            address = std::stoi(symbol);
        } 
        else {
            // Symbolic reference
            auto it = symbolTable.find(symbol);

            if (it != symbolTable.end()) {
                // Symbol found in the symbol table
                address = it->second;
            } else {
                // New variable, add to symbol table and assign an address
                symbolTable[symbol] = nextVariableAddress;
                address = nextVariableAddress;
                nextVariableAddress++;
            }
        }

        std::bitset<15> binary_address(address);
        return "0" + binary_address.to_string();
    }

    // Method to update the symbol table during the first pass
    void updateSymbolTable(const std::string& instruction, std::unordered_map<std::string, int>& symbolTable, int& currentAddress) {
        // std::cout<<"Update request for : "<<instruction<<", at address "<<currentAddress<<"\n";
        if (instruction[0] == '(' && instruction.back() == ')') {
            // Label declaration (xxx)
            std::string label = instruction.substr(1, instruction.size() - 2);
            symbolTable[label] = currentAddress + 1;
        } 
        
        else if (instruction[0] == '@') {
            // A-instruction or numeric label
            std::string symbol = instruction.substr(1);

            if (!std::all_of(symbol.begin(), symbol.end(), ::isdigit)) {
                // A-instruction with symbolic reference
                symbolTable[symbol] = currentAddress + 1;
            }
        } 
        
        else {
            // C-instruction or other cases, increment the address
            currentAddress++;
        }
    }

private:
    int nextVariableAddress = 16; // Starting address for variables
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_filename>\n";
        return 1;
    }

    // Get the input filename from the command line arguments
    std::filesystem::path input_path(argv[1]);

    if (!std::filesystem::exists(input_path)) {
        std::cerr << "Error: Input file does not exist.\n";
        return 1;
    }

    // Create instances of Parser, Code, and symbol table
    Parser parser(input_path.string());  // Use string() to convert path to a string
    Code code;
    // std::unordered_map<std::string, int> symbolTable;
    std::unordered_map<std::string, int> symbolTable = {
        {"R0", 0},
        {"R1", 1},
        {"R2", 2},
        {"R3", 3},
        {"R4", 4},
        {"R5", 5},
        {"R6", 6},
        {"R7", 7},
        {"R8", 8},
        {"R9", 9},
        {"R10",10},
        {"R11",11},
        {"R12",12},
        {"R13",13},
        {"R14",14},
        {"R15",15},
        {"SP", 0},
        {"LCL",1},
        {"ARG",2},
        {"THIS",3},
        {"THAT",4},
        {"SCREEN", 16384},
        {"KBD", 24576}
    };


    // First pass: Build the symbol table
    int currentAddress = 0;
    while (parser.hasMoreLines()) {
        parser.advance();
        std::string current_instruction = parser.getCurrentInstruction();
        // std::cout<<currentAddress<<" "<<current_instruction<<"\n";
        if (parser.instructionType() == 1) {
            currentAddress--;
            code.updateSymbolTable(current_instruction, symbolTable, currentAddress);
        }
        currentAddress++;
    }

    // Second pass: Generate machine code
    parser = Parser(input_path.string()); // Reset parser to the beginning of the file
    currentAddress = 0;

    // Create an output file with .hack extension in the same directory as the input file
    std::filesystem::path output_path = input_path.parent_path() / input_path.stem();
    output_path.replace_extension(".hack");

    std::ofstream output_file(output_path);

    if (!output_file.is_open()) {
        std::cerr << "Error: Unable to create output file.\n";
        return 1;
    }

    int line = 1;
    while (parser.hasMoreLines()) {
        parser.advance();
        std::string current_instruction = parser.getCurrentInstruction();

        // std::cout << line << " " << current_instruction << "\n";
        line++;

        if (current_instruction.empty() || current_instruction[0] == '/' || current_instruction[0] == '\n') {
            // std::cout << "skipped " << current_instruction << "\n";
            continue;
        }

        std::string binary_code;

        if (parser.instructionType() == 0) {
            // A-instruction
            binary_code = code.translateAInstruction(current_instruction, symbolTable);
        } 
        else if(parser.instructionType() == 2) {
            // C-instruction
            binary_code = code.translateCInstruction(current_instruction);
        }
        else continue;

        // Write the binary code to the output file
        output_file << binary_code << "\n";
    }

    // Close the output file
    output_file.close();

    std::cout << "Assembly process completed successfully.\n";

    return 0;
}
