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


enum:uint8_t{ CHANNEL_B_MAX = UINT8_MAX };

const vlpp::rgba_color WHITE(CHANNEL_B_MAX, CHANNEL_B_MAX, CHANNEL_B_MAX, CHANNEL_B_MAX);
const vlpp::rgba_color BLACK(0, 0, 0, CHANNEL_B_MAX);
const vlpp::rgba_color RED(CHANNEL_B_MAX, 0, 0, CHANNEL_B_MAX);
const vlpp::rgba_color BLUE(0, 0, CHANNEL_B_MAX, CHANNEL_B_MAX);
const vlpp::rgba_color GREEN(0, CHANNEL_B_MAX, 0, CHANNEL_B_MAX);
const vlpp::rgba_color YELLOW(CHANNEL_B_MAX, CHANNEL_B_MAX, 0, CHANNEL_B_MAX);
const vlpp::rgba_color CYAN(0, CHANNEL_B_MAX, CHANNEL_B_MAX, CHANNEL_B_MAX);
const vlpp::rgba_color MAGENTA(CHANNEL_B_MAX, 0, CHANNEL_B_MAX, CHANNEL_B_MAX);


const std::vector<vlpp::rgba_color> BLACK_WHITE = {BLACK, WHITE};
const std::vector<vlpp::rgba_color> REAL_COLORS = {RED, BLUE, GREEN, YELLOW, CYAN, MAGENTA};
const std::vector<vlpp::rgba_color> ALL_COLORS  = {BLACK, WHITE, RED, BLUE, GREEN, YELLOW, CYAN, MAGENTA};
const std::vector<vlpp::rgba_color> MOST_COLORS = {WHITE, RED, BLUE, GREEN, YELLOW, CYAN, MAGENTA};

const std::map<std::string, std::vector<vlpp::rgba_color>> COLOR_SETS_MAP = {
	{"b_w", BLACK_WHITE}, {"real", REAL_COLORS}, {"all", ALL_COLORS},
	{"most", MOST_COLORS}
};

const std::map<std::string, const vlpp::rgba_color> COLOR_MAP = {
	{"black", BLACK}, {"white", WHITE},
	{"red", RED}, {"blue", BLUE}, {"green", GREEN}, {"yellow", YELLOW},
	{"cyan", CYAN}, {"magenta", MAGENTA}
};

#endif // COLORS_HPP

