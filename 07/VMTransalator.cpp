#include <iostream>
#include <fstream>
#include <sstream>
#include <bitset>
#include <algorithm>
#include <filesystem>
#include <unordered_map>

// Define Parser class to handle parsing of VM instructions
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

            std::cout<<current_instruction<<"\n";

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

            // > 1 accounts for newline '\n' character in each line. 
            // Also all commands have length > 1.
            if (current_instruction.size() > 1) {
                // std::cout<<"advance stops on "<<current_instruction<<"\n";
                break;
            }

        }

        // remove return / newline
        if (!current_instruction.empty())
            while ((current_instruction.back() == '\r') || 
                   (current_instruction.back() == '\n') || 
                   (current_instruction.back() == ' ') || 
                   (current_instruction.back() == '\t'))
                {current_instruction.pop_back(); if (current_instruction.empty()) break;}
        
        std::cout<<current_instruction.size()<<" 111\n";
    }

    enum CommandType {
        C_ARITHMETIC = 0,
        C_PUSH = 1,
        C_POP = 2,
        C_LABEL = 3,
        C_GOTO = 4,
        C_IF = 5,
        C_FUNCTION = 6,
        C_RETURN = 7,
        C_CALL = 8
    };

    std::vector<std::string> splitCommand(const std::string& str) {
        std::vector<std::string> tokens;
        std::istringstream iss(str);
        std::string token;
        
        // Extract tokens using whitespace as delimiter
        while (iss >> token) {
            tokens.push_back(token);
        }
        return tokens;
    }

    CommandType commandType() {

        if (current_instruction.empty()) {
            throw std::runtime_error("No current instruction");
        }

        std::vector<std::string> tokens = splitCommand(current_instruction);
        
        const std::string& command = tokens[0];
        
        if (command == "push") {
            return C_PUSH;
        }
        else if (command == "pop") {
            return C_POP;
        }
        else if (command == "label") {
            return C_LABEL;
        }
        else if (command == "goto") {
            return C_GOTO;
        }
        else if (command == "if-goto") {
            return C_IF;
        }
        else if (command == "function") {
            return C_FUNCTION;
        }
        else if (command == "call") {
            return C_CALL;
        }
        else if (command == "return") {
            return C_RETURN;
        }
        else {
            return C_ARITHMETIC;
        }
    }

    std::string arg1() {
        std::vector<std::string> tokens = splitCommand(current_instruction);
        
        if (tokens.empty()) {
            throw std::runtime_error("Empty instruction in arg1()");
        }
        
        if (commandType() == 0) {  // C_ARITHMETIC
            return tokens[0];
        }
        else if (commandType() == 7) {  // C_RETURN
            throw std::runtime_error("Invalid call to arg1() for C_RETURN");
        }
        else {
            if (tokens.size() < 2) {
                throw std::runtime_error("Not enough tokens for arg1()");
            }
            return tokens[1];
        }
    }

    std::int32_t arg2() {
        std::vector<std::string> tokens = splitCommand(current_instruction);
    
        if (tokens.size() < 3) {
            throw std::runtime_error("Not enough arguments for arg2()");
        }
        
        return std::stoi(tokens[2]);
    }
public:
    std::string current_instruction;
private:
    std::ifstream input_file;
};

class CodeWriter {
public:
    int ctr = 0;
    int static_ctr = 0;
    std::unordered_map<int, int> static_symbol_table;
    std::string writeArithmetic(std::string op) {

        ctr++;

        std::string write_arithmetic = "";

        if (op == "add") {
            // write_arithmetic += "@SP\nM=M-1\nA=M\nD=M\n";
            write_arithmetic += "@SP\nAM=M-1\nD=M\n";
            write_arithmetic += "@SP\nA=M-1\nM=M+D\n";
        }
        else if (op == "sub") {
            write_arithmetic += "@SP\nAM=M-1\nD=M\n";
            write_arithmetic += "@SP\nA=M-1\nM=M-D\n";
        }
        else if (op == "neg") {
            write_arithmetic += "@SP\nA=M-1\nM=-M\n";
        }
        else if (op == "eq") {
            write_arithmetic += "@SP\nAM=M-1\nD=M\n";
            write_arithmetic += "@SP\nA=M-1\nM=M-D\n";
            // set flag maybe for shorter further comparisons ... possible?
            write_arithmetic += "D=M\n@EQ_" + std::to_string(ctr) + "_TRUE\nD;JEQ\n";
            write_arithmetic += "@SP\nA=M-1\nM=0\n";
            write_arithmetic += "@EQ_" + std::to_string(ctr) + "_END\n0;JMP\n";
            write_arithmetic += "(EQ_" + std::to_string(ctr) + "_TRUE)\n@SP\nA=M-1\nM=-1\n";
            write_arithmetic += "(EQ_" + std::to_string(ctr) + "_END)\n";
        }
        else if (op == "gt") {
            write_arithmetic += "@SP\nAM=M-1\nD=M\n";
            write_arithmetic += "@SP\nA=M-1\nM=M-D\n";
            write_arithmetic += "D=M\n@GT_" + std::to_string(ctr) + "_TRUE\nD;JGT\n";
            write_arithmetic += "@SP\nA=M-1\nM=0\n";
            write_arithmetic += "@GT_" + std::to_string(ctr) + "_END\n0;JMP\n";
            write_arithmetic += "(GT_" + std::to_string(ctr) + "_TRUE)\n@SP\nA=M-1\nM=-1\n";
            write_arithmetic += "(GT_" + std::to_string(ctr) + "_END)\n";
        }
        else if (op == "lt") {
            write_arithmetic += "@SP\nAM=M-1\nD=M\n";
            write_arithmetic += "@SP\nA=M-1\nM=M-D\n";
            write_arithmetic += "D=M\n@LT_" + std::to_string(ctr) + "_TRUE\nD;JLT\n";
            write_arithmetic += "@SP\nA=M-1\nM=0\n";
            write_arithmetic += "@LT_" + std::to_string(ctr) + "_END\n0;JMP\n";
            write_arithmetic += "(LT_" + std::to_string(ctr) + "_TRUE)\n@SP\nA=M-1\nM=-1\n";
            write_arithmetic += "(LT_" + std::to_string(ctr) + "_END)\n";
        }
        else if (op == "and") {
            write_arithmetic += "@SP\nAM=M-1\nD=M\n";
            write_arithmetic += "@SP\nA=M-1\nM=M&D\n";
        }
        else if (op == "or") {
            write_arithmetic += "@SP\nAM=M-1\nD=M\n";
            write_arithmetic += "@SP\nA=M-1\nM=M|D\n";
        }
        else if (op == "not") {
            write_arithmetic += "@SP\nA=M-1\nM=!M\n";
        }
        else {
            throw std::runtime_error("Invalid arithmetic operation");
        }

        return write_arithmetic;
    }

    std::string writePushPop(int commandType, std::string segment, int index, std::string filename_without_extension) {
        std::string push_pop_code = "";

        if (commandType == Parser::C_PUSH) {
            if (segment == "constant") {
                push_pop_code += "@" + std::to_string(index) + "\n";
                push_pop_code += "D=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";
            }

            else if (segment == "local") {
                push_pop_code += "@LCL\nD=M\n";
                push_pop_code += "@" + std::to_string(index) + "\n";
                push_pop_code += "A=D+A\nD=M\n";
                push_pop_code += "@SP\nA=M\nM=D\n@SP\nM=M+1\n";
            }
            else if (segment == "argument") {
                push_pop_code += "@ARG\nD=M\n";
                push_pop_code += "@" + std::to_string(index) + "\n";
                push_pop_code += "A=D+A\nD=M\n";
                push_pop_code += "@SP\nA=M\nM=D\n@SP\nM=M+1\n";
            }
            else if (segment == "this") {
                push_pop_code += "@THIS\nD=M\n";
                push_pop_code += "@" + std::to_string(index) + "\n";
                push_pop_code += "A=D+A\nD=M\n";
                push_pop_code += "@SP\nA=M\nM=D\n@SP\nM=M+1\n";
            }
            else if (segment == "that") {
                push_pop_code += "@THAT\nD=M\n";
                push_pop_code += "@" + std::to_string(index) + "\n";
                push_pop_code += "A=D+A\nD=M\n";
                push_pop_code += "@SP\nA=M\nM=D\n@SP\nM=M+1\n";
            }
            else if (segment == "temp") {
                push_pop_code += "@" + std::to_string(5 + index) + "\nD=M\n";
                push_pop_code += "@SP\nA=M\nM=D\n@SP\nM=M+1\n";
            }
            else if (segment == "pointer") {
                if (index == 0) {
                    // push pointer 0 -> push @THIS on stack
                    push_pop_code += "@THIS\nD=M\n";
                    push_pop_code += "@SP\nA=M\nM=D\n@SP\nM=M+1\n";
                }
                else {
                    // push pointer 1 -> push @THAT on stack
                    push_pop_code += "@THAT\nD=M\n";
                    push_pop_code += "@SP\nA=M\nM=D\n@SP\nM=M+1\n";
                }
            }
            else if (segment == "static") {

                std::string static_symbol;
                if (static_symbol_table.find(index) != static_symbol_table.end()) {
                    static_symbol = filename_without_extension + "." + std::to_string(static_symbol_table[index]);
                }
                else {
                    static_symbol = filename_without_extension + "." + std::to_string(static_ctr);
                    static_symbol_table[index] = static_ctr;
                    static_ctr++;
                }

                push_pop_code += "@" + static_symbol + "\nD=M\n";
                push_pop_code += "@SP\nA=M\nM=D\n@SP\nM=M+1\n";
            }
        }



        else if (commandType == Parser::C_POP) {

            if (segment == "constant") {
                std::cerr<<"Wrong instruction 'pop constant x' received.\n";
            }
            else if (segment == "local") {
                push_pop_code += "@LCL\nD=M\n";
                push_pop_code += "@" + std::to_string(index) + "\n";
                push_pop_code += "D=D+A\n";
                push_pop_code += "@R13\nM=D\n";
                push_pop_code += "@SP\nAM=M-1\nD=M\n";
                push_pop_code += "@R13\nA=M\nM=D\n";
            }
            else if (segment == "argument") {
                push_pop_code += "@ARG\nD=M\n";
                push_pop_code += "@" + std::to_string(index) + "\n";
                push_pop_code += "D=D+A\n";
                push_pop_code += "@R13\nM=D\n";
                push_pop_code += "@SP\nAM=M-1\nD=M\n";
                push_pop_code += "@R13\nA=M\nM=D\n";
            }
            else if (segment == "this") {
                push_pop_code += "@THIS\nD=M\n";
                push_pop_code += "@" + std::to_string(index) + "\n";
                push_pop_code += "D=D+A\n";
                push_pop_code += "@R13\nM=D\n";
                push_pop_code += "@SP\nAM=M-1\nD=M\n";
                push_pop_code += "@R13\nA=M\nM=D\n";
            }
            else if (segment == "that") {
                push_pop_code += "@THAT\nD=M\n";
                push_pop_code += "@" + std::to_string(index) + "\n";
                push_pop_code += "D=D+A\n";
                push_pop_code += "@R13\nM=D\n";
                push_pop_code += "@SP\nAM=M-1\nD=M\n";
                push_pop_code += "@R13\nA=M\nM=D\n";
            }
            else if (segment == "temp") {
                push_pop_code += "@SP\nAM=M-1\nD=M\n";
                push_pop_code += "@" + std::to_string(5 + index) + "\n";
                push_pop_code += "M=D\n";
            }
            else if (segment == "pointer") {
                if (index == 0) {
                    // pop pointer 0 -> pop to @THIS from stack
                    push_pop_code += "@SP\nAM=M-1\nD=M\n";
                    push_pop_code += "@THIS\nM=D\n";
                }
                else {
                    // pop pointer 1 -> pop to @THAT from stack
                    push_pop_code += "@SP\nAM=M-1\nD=M\n";
                    push_pop_code += "@THAT\nM=D\n";
                }
            }
            else if (segment == "static") {
                std::string static_symbol;
                if (static_symbol_table.find(index) != static_symbol_table.end()) {
                    static_symbol = filename_without_extension + "." + std::to_string(static_symbol_table[index]);
                }
                else {
                    static_symbol = filename_without_extension + "." + std::to_string(static_ctr);
                    static_symbol_table[index] = static_ctr;
                    static_ctr++;
                }

                push_pop_code += "@SP\nAM=M-1\nD=M\n";
                push_pop_code += "@" + static_symbol + "\n";
                push_pop_code += "M=D\n";
            }

        }

        return push_pop_code;
    }

    std::string writeInit() {
        std::string init_code = "@256\nD=A\n@SP\nM=D\n";
        return init_code;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_filename>\n";
        return 1;
    }

    std::filesystem::path input_path(argv[1]);

    if (!std::filesystem::exists(input_path)) {
        std::cerr << "Error: Input file does not exist.\n";
        return 1;
    }

    Parser parser(input_path.string());
    CodeWriter code;

    std::string filename_without_extension = input_path.stem().string();

    // Create an output file with .asm extension in the same directory as the input file
    std::filesystem::path output_path = input_path.parent_path() / input_path.stem();
    output_path.replace_extension(".asm");

    std::ofstream output_file(output_path);

    if (!output_file.is_open()) {
        std::cerr << "Error: Unable to create output file.\n";
        return 1;
    }

    int line = 1;
    while (parser.hasMoreLines()) {
        parser.advance();

        // get the current command type

        if (!parser.current_instruction.size()) continue;

        Parser::CommandType cmd_type = parser.commandType();
        std::cout << cmd_type <<"\n";
        std::string output_code;
        
        switch (cmd_type) {
            case Parser::C_ARITHMETIC:
                output_code = code.writeArithmetic(parser.arg1());
                break;
                
            case Parser::C_PUSH:

            case Parser::C_POP:
                output_code = code.writePushPop(cmd_type, parser.arg1(), parser.arg2(), filename_without_extension);
                break;
                
            case Parser::C_LABEL:
                break;
                
            case Parser::C_GOTO:
                break;
                
            case Parser::C_IF:
                break;
                
            case Parser::C_FUNCTION:
                break;
                
            case Parser::C_RETURN:
                break;
                
            case Parser::C_CALL:
                break;
        }
        
        output_file << output_code;
        
        if (!output_code.empty()) 
            if (output_code.back() != '\n') {
                output_file << '\n';
            }
    }
    output_file << "(END)\n@END\n0;JMP\n";
    output_file.close();

    std::cout << "Transalation completed successfully.\n";

    return 0;
}
