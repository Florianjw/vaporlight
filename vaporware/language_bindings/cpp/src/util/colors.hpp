#ifndef COLORS_HPP
#define COLORS_HPP
#include <vector>
#include <map>
#include <string>

#include "../lib/rgba_color.hpp"

/**
 * @brief Converts a string to a list of colors.
 * @param str the string
 * @return a vector that contains the colors
 */
std::vector<vlpp::rgba_color> str_to_cols(const std::string& str);

/**
 * @brief converts a string to a color.
 * @param str the string
 * @return the color that was represented by the string
 */
vlpp::rgba_color str_to_col(const std::string& str);

extern const vlpp::rgba_color WHITE;
extern const vlpp::rgba_color BLACK;
extern const vlpp::rgba_color RED;
extern const vlpp::rgba_color BLUE;
extern const vlpp::rgba_color GREEN;
extern const vlpp::rgba_color YELLOW;
extern const vlpp::rgba_color CYAN;
extern const vlpp::rgba_color MAGENTA;


extern const std::vector<vlpp::rgba_color> BLACK_WHITE;
extern const std::vector<vlpp::rgba_color> REAL_COLORS;
extern const std::vector<vlpp::rgba_color> ALL_COLORS;
extern const std::vector<vlpp::rgba_color> MOST_COLORS;

#endif // COLORS_HPP

