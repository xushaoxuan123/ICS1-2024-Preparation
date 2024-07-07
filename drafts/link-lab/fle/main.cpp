#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

using json = nlohmann::ordered_json;
using namespace std::string_literals;

std::string get_basename(std::string_view path) {
  return std::filesystem::path(path).filename().string();
}

std::string get_filename_without_extension(std::string_view path) {
  return std::filesystem::path(path).stem().string();
}

std::string trim(std::string_view s) {
  s.remove_prefix(std::min(s.find_first_not_of(" \t"), s.size()));
  s.remove_suffix(s.size() - s.find_last_not_of(" \t") - 1);
  return std::string(s);
}

std::string trim(std::string_view s, std::string_view chars) {
  s.remove_prefix(std::min(s.find_first_not_of(chars), s.size()));
  s.remove_suffix(s.size() - s.find_last_not_of(chars) - 1);
  return std::string(s);
}

auto split(std::string_view s, std::string_view delimiter) {
  std::vector<std::string> parts;
  size_t pos = 0;
  while (pos != std::string::npos) {
    auto p = s.find(delimiter, pos);
    parts.push_back(std::string(s.substr(pos, p - pos)));
    pos = p == std::string::npos ? p : p + delimiter.size();
  }
  return parts;
}

// Helper function to execute a shell command and capture its output
std::string exec(std::string_view cmd) {
  auto final_cmd = cmd.data() + " 2>/dev/null"s;
  FILE *pipe = popen(final_cmd.c_str(), "r");
  if (!pipe)
    throw std::runtime_error("popen() failed!");

  std::string result;
  char buffer[128];
  size_t bytes_read;

  while ((bytes_read = fread(buffer, 1, sizeof(buffer), pipe)) != 0) {
    result.append(buffer, bytes_read);
  }

  pclose(pipe);
  return result;
}

// Load FLE file
json load_fle(const std::string &file) {
  std::ifstream infile(file);
  std::string content((std::istreambuf_iterator<char>(infile)),
                      std::istreambuf_iterator<char>());

  if (content.substr(0, 2) == "#!") {
    content = content.substr(content.find('\n') + 1);
  }

  return json::parse(content);
}

// FLE_objdump function
void FLE_objdump(const std::vector<std::string> &files) {
  for (const auto &file : files) {
    json j = load_fle(file);
    std::cout << j.dump(4) << std::endl; // Pretty print with indent of 4
  }
}

// FLE_readfle function
void FLE_readfle(const std::vector<std::string> &files) { FLE_objdump(files); }

// FLE_nm function
void FLE_nm(const std::vector<std::string> &files) {
  for (const auto &file : files) {
    json fle = load_fle(file);
    if (fle["type"] != ".exe") {
      std::cerr << "File is not an executable FLE." << std::endl;
      continue;
    }
    std::cout << fle["symbols"].dump(4) << std::endl;
  }
}

// FLE_exec function
void FLE_exec(const std::vector<std::string> &files) {
  json fle = load_fle(files[0]);
  if (fle["type"] != ".exe") {
    std::cerr << "File is not an executable FLE." << std::endl;
    return;
  }

  std::vector<uint8_t> bs;
  for (const auto &line : fle[".load"]) {
    if (line.get<std::string>().substr(0, 5) == "🔢:") {
      std::stringstream ss(line.get<std::string>().substr(5));
      uint32_t byte;
      while (ss >> std::hex >> byte) {
        bs.push_back(static_cast<uint8_t>(byte));
      }
    }
  }

  size_t size = bs.size();
  void *mem = mmap(nullptr, size, PROT_READ | PROT_WRITE | PROT_EXEC,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mem == MAP_FAILED) {
    perror("mmap");
    exit(EXIT_FAILURE);
  }

  memcpy(mem, bs.data(), size);

  using FuncType = int (*)();
  FuncType func = reinterpret_cast<FuncType>(
      static_cast<uint8_t *>(mem) + fle["symbols"]["_start"].get<int>());
  func();
}

// eval function for expression evaluation
using SymbolTable = std::function<int(const std::string &)>;

int eval_inner_expr(const std::string &expr, const SymbolTable &symbols);

int eval_term(const std::string &term, const SymbolTable &symbols) {
  if (term.empty())
    return 0;
  if (term.substr(0, 2) == "0x") {
    return std::stoi(term, nullptr, 16);
  } else if (term[0] == '(' && term.back() == ')') {
    return eval_inner_expr(term.substr(1, term.size() - 2), symbols);
  } else if (isdigit(term[0]) || (term[0] == '-' && isdigit(term[1]))) {
    return std::stoi(term);
  } else {
    return symbols(term);
  }
}

int eval_inner_expr(const std::string &expr, const SymbolTable &symbols) {
  std::istringstream iss(expr);
  std::string token;
  int result = 0;
  char op = '+';

  while (iss >> token) {
    int value = eval_term(token, symbols);
    if (op == '+') {
      result += value;
    } else if (op == '-') {
      result -= value;
    }
    iss >> op;
  }

  return result;
}

std::vector<uint8_t> eval_expr(const std::string &expr,
                               const SymbolTable &symbols) {
  std::smatch match;
  std::regex pattern(R"((\w+)\((.*)\))");
  if (!std::regex_match(expr, match, pattern)) {
    throw std::runtime_error("Invalid expression: " + expr);
  }

  std::string type = match[1];
  std::string inner_expr = match[2];
  int result = eval_inner_expr(inner_expr, symbols);
  if (type == "i32") {
    return {static_cast<uint8_t>(result), static_cast<uint8_t>(result >> 8),
            static_cast<uint8_t>(result >> 16),
            static_cast<uint8_t>(result >> 24)};
  } else {
    throw std::runtime_error("Unsupported type: " + type);
  }
}

// FLE_ld function
void FLE_ld(const std::vector<std::string> &args) {
  using Item =
      std::pair<std::string, std::vector<std::pair<std::string, json>>>;
  // std::unordered_map<std::string, std::vector<std::pair<std::string, json>>>
  //     sections;
  std::vector<Item> sections;
  auto exists = [&](const std::string &name) -> bool {
    for (auto &[sec_name, sec] : sections) {
      if (sec_name == name) {
        return true;
      }
    }
    return false;
  };
  auto find_section = [&](const std::string &name)
      -> std::vector<std::pair<std::string, json>> & {
    for (auto &[sec_name, sec] : sections) {
      if (sec_name == name) {
        return sec;
      }
    }
    sections.emplace_back(name, std::vector<std::pair<std::string, json>>());
    return sections.back().second;
  };
  std::map<std::string, json> fles;
  std::string dest;

  for (size_t i = 0; i < args.size(); ++i) {
    if (args[i] == "-o")
      continue;
    if (i > 0 && args[i - 1] == "-o") {
      dest = args[i];
    } else {
      std::ifstream infile(args[i]);
      json fle_content;
      infile >> fle_content;
      if (fle_content["type"] != ".obj") {
        throw std::runtime_error("File is not an object file.");
      }

      fles[args[i]] = fle_content;
      for (auto &[k, v] : fle_content.items()) {
        if (k[0] == '.') {
          // sections[k].emplace_back(args[i], v);
          find_section(k).emplace_back(args[i], v);
        }
      }
    }
  }

  if (dest.empty()) {
    throw std::runtime_error("No output file specified.");
  }

  std::map<std::string, int> symbols;
  std::vector<uint8_t> res;

  for (int iter = 0; iter < 2; ++iter) {
    res.clear();

    for (auto &[sec_name, sec] : sections) {
      for (auto &[fle, sec_body] : sec) {
        auto temp = sec_name.substr(1);
        std::replace(temp.begin(), temp.end(), '.', '_');
        auto key = fle + "." + temp;
        symbols[key] = res.size();

        for (auto &item : sec_body) {
          auto temp = split(item.get<std::string>(), ": ");
          std::string token = temp[0];

          if (token == "📤") {
            std::string symb = trim(temp[1]);
            if (iter == 0 && symbols.count(symb)) {
              throw std::runtime_error("Multiple definition of " + symb);
            }
            symbols[symb] = res.size();
          } else if (token == "🏷️") {
            std::string symb = trim(temp[1]);
            symbols[fle + "." + symb] = res.size();
          } else if (token == "🔢") {
            std::string xs = trim(temp[1]);
            auto bytes = split(xs, " ");
            // for (size_t i = 0; i < xs.size(); i += 2) {
            //   res.push_back(std::stoi(xs.substr(i, 2), nullptr, 16));
            // }
            for (auto &byte : bytes) {
              res.push_back(std::stoi(byte, nullptr, 16));
            }
          } else if (token == "❓") {
            std::string expr = trim(temp[1]);

            auto Symbols = [&](const std::string &key) -> int {
              if (iter == 0) {
                return symbols.count(key) ? symbols[key] : 0;
              } else {
                std::string key_l = fle + "." + key;
                if (symbols.count(key_l)) {
                  return symbols[key_l];
                } else if (symbols.count(key)) {
                  return symbols[key];
                } else {
                  throw std::runtime_error("Undefined symbol: " + key);
                }
              }
            };

            expr = std::regex_replace(expr, std::regex("📍"),
                                      "(" + std::to_string(res.size()) + ")");
            auto reloc_bytes = eval_expr(expr, Symbols);
            res.insert(res.end(), reloc_bytes.begin(), reloc_bytes.end());
          }
        }
      }
    }
  }

  std::unordered_map<std::string, int> symbols_global;
  for (auto &[k, v] : symbols) {
    if (v != 0 && k.find('.') == std::string::npos) {
      symbols_global[k] = v;
    }
  }

  std::vector<std::string> parts;
  for (size_t i = 0; i < res.size(); i += 16) {
    std::stringstream ss;
    ss << "🔢: ";
    for (size_t j = i; j < std::min(i + 16, res.size()); ++j) {
      ss << std::hex << std::setw(2) << std::setfill('0') << (int)res[j] << " ";
    }
    parts.push_back(trim(ss.str()));
  }

  json output = {
      {"type", ".exe"}, {"symbols", symbols_global}, {".load", parts}};

  std::ofstream outfile(dest);
  outfile << "#!./exec\n\n" << output.dump(4) << std::endl;
  chmod(dest.c_str(), 0755);
}

std::vector<std::string> splitlines(std::string_view s) {
  std::vector<std::string> lines;
  std::istringstream ss(s.data());
  std::string line;
  while (std::getline(ss, line, '\n')) {
    lines.push_back(line);
  }
  return lines;
}

// elf_to_fle function
json elf_to_fle(const std::string &binary, const std::string &section) {
  std::string command =
      "objcopy --dump-section " + section + "=/dev/stdout " + binary;
  std::string section_data = exec(command.c_str());

  command = "readelf -r " + binary;
  std::string relocs = exec(command.c_str());

  command = "objdump -t " + binary;
  std::string names = exec(command.c_str());

  struct Symbol {
    char symb_type;
    std::string section;
    unsigned int offset;
    std::string name;
  };

  std::vector<Symbol> symbols;
  std::regex pattern(
      R"(^([0-9a-fA-F]+)\s+(l|g)\s+(\w+)?\s+([.a-zA-Z0-9_]+)\s+([0-9a-fA-F]+)\s+(.*)$)");
  for (auto &line : splitlines(names)) {
    if (std::smatch match; std::regex_match(line, match, pattern)) {
      unsigned int offset = std::stoul(match[1].str(), nullptr, 16);
      char symb_type = match[2].str()[0];
      std::string section = match[4].str();
      std::string name = match[6].str();
      std::replace(name.begin(), name.end(), '.', '_'); // 替换点为下划线

      symbols.push_back(Symbol(symb_type, section, offset, name));
    }
  }

  std::map<int, std::pair<int, std::string>> relocations;
  bool enabled = true;
  pattern = std::regex(
      R"(^\s*([0-9a-fA-F]+)\s+([0-9a-fA-F]+)\s+(\S+)\s+([0-9a-fA-F]+)\s+(.*)$)");
  for (auto &line : splitlines(relocs)) {
    if (line.find("Relocation section") != std::string::npos) {
      enabled = line.find(".rela" + section) != std::string::npos;
    } else if (enabled) {
      if (std::smatch match; std::regex_match(line, match, pattern)) {
        int offset = std::stoi(match[1], nullptr, 16);
        std::string expr = match[5];

        std::istringstream iss(expr);
        std::vector<std::string> rs(std::istream_iterator<std::string>{iss},
                                    std::istream_iterator<std::string>());
        if (rs.empty()) {
          throw std::runtime_error("Empty relocation expression");
        }
        expr = rs[0];
        for (size_t i = 1; i < rs.size(); ++i) {
          std::stringstream ss;
          unsigned int num;
          if (std::sscanf(rs[i].c_str(), "%x", &num) > 0) {
            ss << "0x" << std::hex << num;
            expr += " " + ss.str();
          } else {
            expr += " " + rs[i];
          }
        }
        expr += " - 📍";

        if (match[3] != "R_X86_64_PC32" && match[3] != "R_X86_64_PLT32" &&
            match[3] != "R_X86_64_32") {
          throw std::runtime_error("Unsupported relocation " + match[3].str());
        }

        std::replace(expr.begin(), expr.end(), '.', '_');

        relocations[offset] = {4, "i32(" + expr + ")"};
      }
    }
  }

  std::vector<std::string> res;
  int skip = 0;
  std::vector<uint8_t> holding;

  auto do_dump = [&](std::vector<uint8_t> &holding) {
    if (!holding.empty()) {
      std::stringstream ss;
      ss << "🔢: ";
      for (auto &byte : holding) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)byte << " ";
      }
      res.push_back(trim(ss.str()));
      holding.clear();
    }
  };

  for (int i = 0, len = section_data.size(); i < len; ++i) {
    for (auto &sym : symbols) {
      if (sym.section == section && sym.offset == i) {
        do_dump(holding);
        if (sym.symb_type == 'l') {
          res.push_back("🏷️: " + sym.name);
        } else {
          res.push_back("📤: " + sym.name);
        }
      }
    }
    if (relocations.find(i) != relocations.end()) {
      auto [skip_temp, reloc] = relocations[i];
      do_dump(holding);
      res.push_back("❓: " + reloc);
      skip = skip_temp;
    }
    if (skip > 0) {
      --skip;
    } else {
      holding.push_back(section_data[i]);
      if (holding.size() == 16) {
        do_dump(holding);
      }
    }
  }
  do_dump(holding);

  return res;
}

// FLE_cc function
void FLE_cc(const std::vector<std::string> &options) {
  std::vector<std::string> CFLAGS = {"-static", "-fPIE", "-nostdlib",
                                     "-ffreestanding",
                                     "-fno-asynchronous-unwind-tables"};
  std::vector<std::string> gcc_cmd = {"gcc", "-c"};
  gcc_cmd.insert(gcc_cmd.end(), CFLAGS.begin(), CFLAGS.end());
  gcc_cmd.insert(gcc_cmd.end(), options.begin(), options.end());

  std::string binary;
  auto it = std::find(gcc_cmd.begin(), gcc_cmd.end(), "-o");
  if (it != gcc_cmd.end() && ++it != gcc_cmd.end()) {
    binary = *it;
  } else {
    throw std::runtime_error("Output file not specified.");
  }

  std::string command = "";
  for (const auto &arg : gcc_cmd) {
    command += " " + arg;
  }
  if (std::system(command.c_str()) != 0) {
    throw std::runtime_error("gcc command failed");
  }

  command = "objdump -h " + binary;
  std::string objdump_output = exec(command.c_str());

  std::vector<std::string> lines;
  std::stringstream ss(objdump_output);
  std::string line;
  while (std::getline(ss, line, '\n')) {
    lines.push_back(line);
  }

  json res;
  res["type"] = ".obj";

  for (size_t i = 0; i < lines.size(); ++i) {
    std::regex pattern(R"(^\s*([0-9]+)\s+(\.(\w|\.)+)\s+([0-9a-fA-F]+)\s+.*$)");
    std::smatch match;
    if (std::regex_match(lines[i], match, pattern)) {
      std::string section = match[2];
      std::string flags_line = lines[i + 1];
      std::vector<std::string> flags;
      std::stringstream ss(flags_line);
      std::string flag;
      while (std::getline(ss, flag, ',')) {
        flag = trim(flag);
        flags.push_back(flag);
      }

      if (std::find(flags.begin(), flags.end(), "ALLOC") != flags.end() &&
          section.find("note.gnu.property") == std::string::npos) {
        res[section] = elf_to_fle(binary, section);
      }
    }
  }

  std::ofstream outfile(get_filename_without_extension(binary) + ".fle");
  outfile << res.dump(4) << std::endl;
}

// Main function
int main(int argc, char *argv[]) {
  std::string tool = "FLE_"s + get_basename(argv[0]);
  std::vector<std::string> args(argv + 1, argv + argc);

  if (tool == "FLE_objdump") {
    FLE_objdump(args);
  } else if (tool == "FLE_readfle") {
    FLE_readfle(args);
  } else if (tool == "FLE_nm") {
    FLE_nm(args);
  } else if (tool == "FLE_exec") {
    FLE_exec(args);
  } else if (tool == "FLE_ld") {
    FLE_ld(args);
  } else if (tool == "FLE_cc") {
    FLE_cc(args);
  } else {
    std::cerr << tool << " is not implemented." << std::endl;
    return 1;
  }

  return 0;
}
