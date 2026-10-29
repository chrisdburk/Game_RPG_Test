#include "core/UI.h"

#include <cctype>
#include <iostream>
#include <sstream>

namespace ui {

namespace {

std::string trim(const std::string& text) {
    std::size_t start = 0;
    std::size_t end = text.size();
    while (start < end && std::isspace(static_cast<unsigned char>(text[start]))) ++start;
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1]))) --end;
    return text.substr(start, end - start);
}

bool parseInt(const std::string& text, int& out) {
    if (text.empty()) return false;
    try {
        std::size_t used = 0;
        int value = std::stoi(text, &used);
        if (used != text.size()) return false;
        out = value;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

int chooseImpl(const std::vector<std::string>& options, const std::string& prompt,
               const std::string* backLabel) {
    for (std::size_t i = 0; i < options.size(); ++i) {
        std::cout << "  " << (i + 1) << ") " << options[i] << '\n';
    }
    if (backLabel) std::cout << "  0) " << *backLabel << '\n';

    const int count = static_cast<int>(options.size());
    while (true) {
        int choice = 0;
        if (parseInt(readLine(prompt), choice)) {
            if (backLabel && choice == 0) return -1;
            if (choice >= 1 && choice <= count) return choice - 1;
        }
        std::cout << "  Please enter a number from " << (backLabel ? 0 : 1) << " to " << count
                  << ".\n";
    }
}

}  // namespace

void print(const std::string& text) { std::cout << text << '\n'; }

void wrap(const std::string& text, int indent) {
    const std::string pad(static_cast<std::size_t>(indent), ' ');
    const std::size_t limit = static_cast<std::size_t>(kWidth - indent);
    std::istringstream paragraphs(text);
    std::string paragraph;
    while (std::getline(paragraphs, paragraph)) {
        std::istringstream words(paragraph);
        std::string word;
        std::string line;
        while (words >> word) {
            if (!line.empty() && line.size() + 1 + word.size() > limit) {
                std::cout << pad << line << '\n';
                line.clear();
            }
            line += (line.empty() ? "" : " ") + word;
        }
        std::cout << pad << line << '\n';
    }
}

void divider(char c) { std::cout << std::string(kWidth, c) << '\n'; }

void header(const std::string& title) {
    const std::size_t inner = kWidth - 2;
    const std::string text = title.size() > inner ? title.substr(0, inner) : title;
    const std::size_t left = (inner - text.size()) / 2;
    const std::size_t right = inner - text.size() - left;
    std::cout << '\n'
              << '+' << std::string(inner, '-') << "+\n"
              << '|' << std::string(left, ' ') << text << std::string(right, ' ') << "|\n"
              << '+' << std::string(inner, '-') << "+\n";
}

std::string readLine(const std::string& prompt) {
    std::cout << prompt << " > " << std::flush;
    std::string line;
    if (!std::getline(std::cin, line)) throw InputClosed{};
    return trim(line);
}

int choose(const std::vector<std::string>& options, const std::string& prompt) {
    return chooseImpl(options, prompt, nullptr);
}

int chooseOrBack(const std::vector<std::string>& options, const std::string& prompt,
                 const std::string& backLabel) {
    return chooseImpl(options, prompt, &backLabel);
}

int readNumber(const std::string& prompt, int min, int max) {
    while (true) {
        int value = 0;
        if (parseInt(readLine(prompt), value) && value >= min && value <= max) return value;
        std::cout << "  Please enter a number from " << min << " to " << max << ".\n";
    }
}

bool confirm(const std::string& question) {
    while (true) {
        std::string answer = readLine(question + " (y/n)");
        if (answer.empty()) continue;
        char c = static_cast<char>(std::tolower(static_cast<unsigned char>(answer[0])));
        if (c == 'y') return true;
        if (c == 'n') return false;
    }
}

void pause() {
    std::cout << "  -- Press Enter to continue --" << std::flush;
    std::string line;
    if (!std::getline(std::cin, line)) throw InputClosed{};
}

std::string bar(int current, int maximum, int width) {
    int filled = maximum > 0 ? (current * width + maximum - 1) / maximum : 0;
    if (filled < 0) filled = 0;
    if (filled > width) filled = width;
    return "[" + std::string(static_cast<std::size_t>(filled), '#') +
           std::string(static_cast<std::size_t>(width - filled), '-') + "]";
}

std::string padRight(const std::string& text, std::size_t width) {
    if (text.size() >= width) return text + ' ';
    return text + std::string(width - text.size(), ' ');
}

std::string signedNumber(int value) {
    return (value >= 0 ? "+" : "") + std::to_string(value);
}

}  // namespace ui
