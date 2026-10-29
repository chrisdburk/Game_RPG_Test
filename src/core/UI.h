#pragma once

#include <cstddef>
#include <string>
#include <vector>

// Console input/output helpers. All game text goes through here so the
// presentation stays consistent: plain ASCII, kWidth columns wide.
namespace ui {

// Thrown when standard input closes (e.g. Ctrl+D) so the game can exit cleanly.
struct InputClosed {};

constexpr int kWidth = 64;

void print(const std::string& text = "");
// Prints text word-wrapped to the console width. Newlines start new paragraphs.
void wrap(const std::string& text, int indent = 0);
void divider(char c = '-');
// Prints a boxed, centered title.
void header(const std::string& title);

std::string readLine(const std::string& prompt);
// Shows numbered options and returns the 0-based index of the one chosen.
int choose(const std::vector<std::string>& options, const std::string& prompt = "Choose");
// Like choose(), but also offers "0) Back"; returns -1 when it is picked.
int chooseOrBack(const std::vector<std::string>& options, const std::string& prompt = "Choose",
                 const std::string& backLabel = "Back");
int readNumber(const std::string& prompt, int min, int max);
bool confirm(const std::string& question);
void pause();

// "[#######-------------]" style meter.
std::string bar(int current, int maximum, int width = 20);
std::string padRight(const std::string& text, std::size_t width);
// "+2", "-1", "+0"
std::string signedNumber(int value);

}  // namespace ui
