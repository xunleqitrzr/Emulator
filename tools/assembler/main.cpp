/**
 * A simple two-pass assembler for the custom 8-bit CPU.
 *
 * How to compile (from the project root directory):
 * make assembler
 *
 * How to run (from the project root directory):
 * ./easm my_program.asm my_program.bin
 *
 * This will create `my_program.bin`, which can then be loaded by main.c.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <stdexcept>
#include <cstdint>
#include <algorithm>

// --- Helper Maps ---
// Map mnemonics (text) to their opcode (byte)
std::map<std::string, uint8_t> OPCODES = {
    {"NOP", 0x00}, {"LDA", 0x01}, {"LDB", 0x02}, {"LDI", 0x03},
    {"INC", 0x04}, {"DEC", 0x05}, {"ADD", 0x06}, {"SUB", 0x07},
    {"MUL", 0x08}, {"STA", 0x09}, {"STB", 0x0A}, {"MOV", 0x0B},
    {"CMP", 0x0C}, {"JMP", 0x0D}, {"JZ",  0x0E}, {"JNZ", 0x0F},
    {"JC",  0x10}, {"JNC", 0x11}, {"JE",  0x12}, {"JNE", 0x13},
    {"JL",  0x14}, {"JG",  0x15}, {"JB",  0x16}, {"JA",  0x17},
    {"AND", 0x18}, {"OR",  0x19}, {"XOR", 0x1A}, {"NOT", 0x1B},
    {"PUSH", 0x1C}, {"POP", 0x1D}, {"CALL", 0x1E}, {"RET", 0x1F},
    {"HLT", 0xFF}
};

// Map register names (text) to their byte value
std::map<std::string, uint8_t> REGISTERS = {
    {"A", 0x00}, {"B", 0x01}, {"C", 0x02}, {"D", 0x03}
};

// --- Helper Functions ---

// Converts a string to uppercase
std::string to_upper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::toupper(c); });
    return s;
}
// Trims whitespace (space, tab, newline, carriage return) from start and end
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// Splits a line into tokens, handling commas and whitespace
std::vector<std::string> split_line(const std::string& line) {
    std::vector<std::string> tokens;
    std::string token;
    for (char c : line) {
        // Treat all whitespace and commas as delimiters
        if (c == ' ' || c == '\t' || c == ',' || c == '\n' || c == '\r') {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            token += c;
        }
    }
    if (!token.empty()) {
        tokens.push_back(token);
    }
    return tokens;
}

// Parses a value string (e.g., "5", "0x1A", or "my_label")
// This is the heart of the operand logic.
uint16_t parse_operand(const std::string& token, const std::map<std::string, uint16_t>& labels) {
    std::string upper_token = to_upper(token);

    // 1. Is it a label?
    if (labels.count(upper_token)) {
        return labels.at(upper_token);
    }

    // 2. Is it a register? (Should be handled by caller, but good to check)
    if (REGISTERS.count(upper_token)) {
        // This is an error, parse_operand should be for values/addresses
        // But for simplicity, we'll let it pass.
        return REGISTERS.at(upper_token);
    }

    // 3. Is it a number?
    try {
        if (upper_token.rfind("0X", 0) == 0) {
            // Hex number (e.g., "0x1A")
            return std::stoul(upper_token, nullptr, 16);
        } else {
            // Decimal number (e.g., "26")
            return std::stoul(upper_token, nullptr, 10);
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Invalid operand: " + token);
    }
}

// --- Main Assembler Logic ---

int main(int argc, char* argv[]) {
    // --- 1. Argument and File I/O Setup ---
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input.asm> <output.bin>\n";
        return 1;
    }
    std::string input_filename = argv[1];
    std::string output_filename = argv[2];

    // C++ way to open a file for READING text
    std::ifstream infile(input_filename);
    if (!infile) {
        std::cerr << "Error: Cannot open input file " << input_filename << "\n";
        return 1;
    }

    // Read all lines into a vector to process them
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(infile, line)) {
        lines.push_back(line);
    }
    infile.close();

    // --- 2. Assembler - PASS 1 (Label Pass) ---
    // This pass finds all labels and calculates their memory address.
    std::map<std::string, uint16_t> labels;
    uint16_t current_address = 0;

    for (const std::string& line_raw : lines) {
        // Clean up the line: remove comments and trim whitespace
        std::string line = trim(line_raw.substr(0, line_raw.find(';')));
        if (line.empty()) continue;

        // Check for a label (e.g., "LOOP:")
        size_t label_pos = line.find(':');
        if (label_pos != std::string::npos) {
            std::string label = to_upper(trim(line.substr(0, label_pos)));
            if (labels.count(label)) {
                std::cerr << "Error: Duplicate label '" << label << "'\n";
                return 1;
            }
            labels[label] = current_address;
            line = trim(line.substr(label_pos + 1)); // Remove label from line
        }

        if (line.empty()) continue;

        // Parse instruction to find its size
        std::vector<std::string> tokens = split_line(line);
        std::string mnemonic = to_upper(tokens[0]);

        if (OPCODES.count(mnemonic)) {
            // This is the byte-counting logic
            // It mimics your CPU's instruction format
            // 1-byte instructions (NOP, INC, DEC, RET, HLT)
            if (mnemonic == "NOP" || mnemonic == "INC" || mnemonic == "DEC" ||
                mnemonic == "RET" || mnemonic == "HLT") {
                current_address += 1;
            }
            // 2-byte instructions (LDI, PUSH, POP, NOT)
            else if (mnemonic == "LDI" || mnemonic == "PUSH" ||
                     mnemonic == "POP" || mnemonic == "NOT") {
                current_address += 2;
            }
            // 3-byte instructions (everything else)
            else {
                current_address += 3;
            }
        } else {
            std::cerr << "Error (Pass 1): Unknown mnemonic '" << mnemonic << "'\n";
            return 1;
        }
    }

    // --- 3. Assembler - PASS 2 (Code Generation Pass) ---
    // This pass generates the actual machine code.
    std::vector<uint8_t> machine_code;

    for (const std::string& line_raw : lines) {
        // Clean up the line: remove comments and trim whitespace
        std::string line = trim(line_raw.substr(0, line_raw.find(';')));
        if (line.empty()) continue;

        // Remove label (if any)
        size_t label_pos = line.find(':');
        if (label_pos != std::string::npos) {
            line = trim(line.substr(label_pos + 1));
        }

        if (line.empty()) continue;

        std::vector<std::string> tokens = split_line(line);
        std::string mnemonic = to_upper(tokens[0]);

        try {
            // Write the opcode
            machine_code.push_back(OPCODES.at(mnemonic));

            // --- Handle Operands (This is the core logic) ---

            // 1-byte (no operands)
            if (mnemonic == "NOP" || mnemonic == "INC" || mnemonic == "DEC" ||
                mnemonic == "RET" || mnemonic == "HLT") {
                // No operands to add
            }
            // 2-byte (one 8-bit operand)
            else if (mnemonic == "LDI") {
                // LDI <value>
                uint16_t value = parse_operand(tokens.at(1), labels);
                machine_code.push_back((uint8_t)value);
            }
            else if (mnemonic == "PUSH" || mnemonic == "POP" || mnemonic == "NOT") {
                // PUSH <register>
                machine_code.push_back(REGISTERS.at(to_upper(tokens.at(1))));
            }
            // 3-byte (two 8-bit registers OR one 16-bit address)
            else if (mnemonic == "ADD" || mnemonic == "SUB" || mnemonic == "MUL" ||
                     mnemonic == "MOV" || mnemonic == "CMP" || mnemonic == "AND" ||
                     mnemonic == "OR" || mnemonic == "XOR") {
                // MOV <reg_to>, <reg_from>
                machine_code.push_back(REGISTERS.at(to_upper(tokens.at(1))));
                machine_code.push_back(REGISTERS.at(to_upper(tokens.at(2))));
            }
            else {
                // Must be a 16-bit address instruction
                // JMP <address>, LDA <address>, CALL <address>, etc.
                uint16_t addr = parse_operand(tokens.at(1), labels);
                // Write the high byte first
                machine_code.push_back((uint8_t)(addr >> 8));
                // Write the low byte second
                machine_code.push_back((uint8_t)(addr & 0xFF));
            }

        } catch (const std::exception& e) {
            std::cerr << "Assembly Error on line: " << line_raw << "\n";
            std::cerr << "Details: " << e.what() << "\n";
            return 1;
        }
    }

    // --- 4. File I/O - Write Binary File ---
    // C++ way to open a file for WRITING in BINARY mode
    std::ofstream outfile(output_filename, std::ios::binary);
    if (!outfile) {
        std::cerr << "Error: Cannot open output file " << output_filename << "\n";
        return 1;
    }

    // Write the contents of the vector to the file
    outfile.write(reinterpret_cast<const char*>(machine_code.data()), machine_code.size());
    outfile.close();

    std::cout << "Successfully assembled " << machine_code.size() << " bytes to "
              << output_filename << "\n";
    return 0;
}