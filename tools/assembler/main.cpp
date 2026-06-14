/**
 * A simple two-pass assembler for the custom 8-bit CPU.
 *
 * How to compile (from the project root directory):
 * make all
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
    {"JLE", 0x18}, {"JGE", 0x19}, {"AND", 0x1A}, {"OR",  0x1B},
    {"XOR", 0x1C}, {"NOT", 0x1D}, {"PUSH", 0x1E}, {"POP", 0x1F},
    {"CALL", 0x20}, {"RET", 0x21}, {"SHL", 0x22}, {"SHR", 0x23},
    {"ROL", 0x24}, {"ROR", 0x25}, {"LDA_IDX", 0x26}, {"STA_IDX", 0x27},
    {"HLT", 0xFF}
};

// Map register names (text) to their byte value
std::map<std::string, uint8_t> REGISTERS = {
    {"A", 0x00}, {"B", 0x01}, {"C", 0x02}, {"D", 0x03},
    {"E", 0x04}, {"F", 0x05}, {"G", 0x06}, {"H", 0x07},
    {"I", 0x08}, {"J", 0x09}, {"K", 0x0A}, {"L", 0x0B},
    {"M", 0x0C}, {"N", 0x0D}, {"O", 0x0E}, {"P", 0x0F},
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

// Parses a token into an 8-bit byte (handles "10", "0x0A", etc.)
uint8_t parse_byte_value(std::string token) {
    token = to_upper(trim(token));
    if (token.empty()) return 0;

    try {
        if (token.rfind("0X", 0) == 0) {
            return (uint8_t)std::stoul(token, nullptr, 16);
        } else {
            return (uint8_t)std::stoul(token, nullptr, 10);
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Invalid byte value: " + token);
    }
}

// --- NEW --- This function parses the operands for .DB
std::vector<uint8_t> parse_db_operands(const std::string& str) {
    std::vector<uint8_t> bytes;
    std::string current_token;
    bool in_string = false;

    for (char c : str) {
        if (c == '"') {
            in_string = !in_string;
            if (!in_string && !current_token.empty()) {
                // End of a string, add its contents
                for (char ch : current_token) {
                    bytes.push_back((uint8_t)ch);
                }
                current_token.clear();
            }
        } else if (in_string) {
            // We are inside quotes, add char to string token
            current_token += c;
        } else if (c == ' ' || c == '\t' || c == ',') {
            // We are outside a string, and hit a delimiter
            if (!current_token.empty()) {
                // We have a number token, parse it
                bytes.push_back(parse_byte_value(current_token));
                current_token.clear();
            }
        } else {
            // We are outside a string, building a number token
            current_token += c;
        }
    }

    // Handle any trailing token (e.g., .DB 10)
    if (!current_token.empty()) {
        if (in_string) {
            throw std::runtime_error("Unterminated string in .DB directive");
        }
        bytes.push_back(parse_byte_value(current_token));
    }

    return bytes;
}


// Parses a 16-bit value string (e.g., "5", "0x1A", or "my_label")
uint16_t parse_operand(const std::string& token, const std::map<std::string, uint16_t>& labels) {
    std::string upper_token = to_upper(token);

    // 1. Is it a label?
    if (labels.count(upper_token)) {
        return labels.at(upper_token);
    }

    // 2. Is it a register? (Should be handled by caller)
    if (REGISTERS.count(upper_token)) {
        return REGISTERS.at(upper_token);
    }

    // 3. Is it a number?
    try {
        if (upper_token.rfind("0X", 0) == 0) {
            return std::stoul(upper_token, nullptr, 16);
        } else {
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

    std::ifstream infile(input_filename);
    if (!infile) {
        std::cerr << "Error: Cannot open input file " << input_filename << "\n";
        return 1;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(infile, line)) {
        lines.push_back(line);
    }
    infile.close();

    // --- 2. Assembler - PASS 1 (Label Pass) ---
    std::map<std::string, uint16_t> labels;
    uint16_t current_address = 0;

    for (const std::string& line_raw : lines) {
        std::string line = trim(line_raw.substr(0, line_raw.find(';')));
        if (line.empty()) continue;

        size_t label_pos = line.find(':');
        if (label_pos != std::string::npos) {
            std::string label = to_upper(trim(line.substr(0, label_pos)));
            if (labels.count(label)) {
                std::cerr << "Error: Duplicate label '" << label << "'\n";
                return 1;
            }
            labels[label] = current_address;
            line = trim(line.substr(label_pos + 1));
        }

        if (line.empty()) continue;

        std::vector<std::string> tokens = split_line(line);
        std::string mnemonic = to_upper(tokens[0]);

        // --- PASS 1 LOGIC ---
        if (OPCODES.count(mnemonic)) {
            // 1-byte
            if (mnemonic == "NOP" || mnemonic == "RET" || mnemonic == "HLT") {
                current_address += 1;
            }
            // 2-byte
            else if (mnemonic == "LDI" || mnemonic == "PUSH" || mnemonic == "POP" ||
                     mnemonic == "NOT" || mnemonic == "SHL" || mnemonic == "SHR" ||
                     mnemonic == "ROL" || mnemonic == "ROR" || mnemonic == "INC" ||
                     mnemonic == "DEC") {
                current_address += 2;
            }
            // 4-byte (Indexed Addressing)
            else if (mnemonic == "LDA_IDX" || mnemonic == "STA_IDX") {
                current_address += 4;
            }
            // 3-byte (everything else)
            else {
                current_address += 3;
            }
        } else if (mnemonic == ".DB") {
            // --- NEW --- Handle .DB directives
            std::string operand_str = trim(line.substr(tokens[0].length()));
            std::vector<uint8_t> db_bytes = parse_db_operands(operand_str);
            current_address += db_bytes.size();
        } else {
            std::cerr << "Error (Pass 1): Unknown mnemonic '" << mnemonic << "'\n";
            return 1;
        }
    }

    // --- 3. Assembler - PASS 2 (Code Generation Pass) ---
    std::vector<uint8_t> machine_code;

    for (const std::string& line_raw : lines) {
        std::string line = trim(line_raw.substr(0, line_raw.find(';')));
        if (line.empty()) continue;

        size_t label_pos = line.find(':');
        if (label_pos != std::string::npos) {
            line = trim(line.substr(label_pos + 1));
        }

        if (line.empty()) continue;

        std::vector<std::string> tokens = split_line(line);
        std::string mnemonic = to_upper(tokens[0]);

        try {
            // --- PASS 2 LOGIC ---
            if (OPCODES.count(mnemonic)) {
                // It's a regular instruction, write opcode and operands
                machine_code.push_back(OPCODES.at(mnemonic));

                // 1-byte
                if (mnemonic == "NOP" || mnemonic == "RET" || mnemonic == "HLT") {
                    // No operands
                }
                // 2-byte
                else if (mnemonic == "LDI") {
                    uint16_t value = parse_operand(tokens.at(1), labels);
                    machine_code.push_back((uint8_t)value);
                }
                else if (mnemonic == "PUSH" || mnemonic == "POP" || mnemonic == "NOT" ||
                         mnemonic == "SHL" || mnemonic == "SHR" || mnemonic == "ROL" ||
                         mnemonic == "ROR" || mnemonic == "INC" || mnemonic == "DEC") {
                    machine_code.push_back(REGISTERS.at(to_upper(tokens.at(1))));
                }
                // 4-byte (Indexed Addressing)
                else if (mnemonic == "LDA_IDX" || mnemonic == "STA_IDX") {
                    // Op: <addr/label>, <reg>
                    uint16_t addr = parse_operand(tokens.at(1), labels); // e.g., "HELLO_MSG"
                    uint8_t reg = REGISTERS.at(to_upper(tokens.at(2))); // e.g., "B"

                    machine_code.push_back((uint8_t)(addr >> 8));       // Addr HI
                    machine_code.push_back((uint8_t)(addr & 0xFF));     // Addr LO
                    machine_code.push_back(reg);                        // Reg Idx
                }
                // 3-byte
                else {
                    // Registers (MOV, ADD, etc.)
                    if (REGISTERS.count(to_upper(tokens.at(1)))) {
                        machine_code.push_back(REGISTERS.at(to_upper(tokens.at(1))));
                        machine_code.push_back(REGISTERS.at(to_upper(tokens.at(2))));
                    }
                    // Address (JMP, LDA, etc.)
                    else {
                        uint16_t addr = parse_operand(tokens.at(1), labels);
                        machine_code.push_back((uint8_t)(addr >> 8));
                        machine_code.push_back((uint8_t)(addr & 0xFF));
                    }
                }
            } else if (mnemonic == ".DB") {
                // --- NEW --- Handle .DB directives
                std::string operand_str = trim(line.substr(tokens[0].length()));
                std::vector<uint8_t> db_bytes = parse_db_operands(operand_str);
                machine_code.insert(machine_code.end(), db_bytes.begin(), db_bytes.end());
            } else {
                // This should have been caught in Pass 1, but we check again.
                throw std::runtime_error("Unknown mnemonic: " + mnemonic);
            }

        } catch (const std::exception& e) {
            std::cerr << "Assembly Error on line: " << line_raw << "\n";
            std::cerr << "Details: " << e.what() << "\n";
            return 1;
        }
    }

    // --- 4. File I/O - Write Binary File ---
    std::ofstream outfile(output_filename, std::ios::binary);
    if (!outfile) {
        std::cerr << "Error: Cannot open output file " << output_filename << "\n";
        return 1;
    }

    outfile.write(reinterpret_cast<const char*>(machine_code.data()), machine_code.size());
    outfile.close();

    std::cout << "Successfully assembled " << machine_code.size() << " bytes to "
              << output_filename << "\n";
    return 0;
}