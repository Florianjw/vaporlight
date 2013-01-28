#ifndef CORE_HPP
#define CORE_HPP

#include "../util/colors.hpp"

#include <cstdint>
#include <vector>
#include <utility>

#include <unistd.h>

/**
 * @brief Takes control of some LEDs and let's them fade in some random ways.
 * @param LEDs the IDs of the LEDs that will be controlled by this function
 */
void control_LEDs(const std::vector<uint16_t>& led_list);

/**
 * @brief fades some LEDs to a new color.
 * @param LEDs a vector that contains the LED-IDs
 * @param fade_time the time that will be used to fade to the new color.
 * @param old_color the old color
 * @param new_color the new color
 */
void fade_to(const std::vector<uint16_t>& led_list, 
		useconds_t fade_time, const vlpp::rgba_color& old_color,
		const vlpp::rgba_color& new_color);

/**
 * @brief Sets some LEDs to a new color in a synchronized way.
 * @param LEDs a vector that contains the LED-IDs
 * @param col the new color
 */
void set_leds(const std::vector<uint16_t>& led_list, const vlpp::rgba_color& col);

/**
 * @brief Returns a linear ratio between two colors, based on the ratio 
 * of the current step vs. the total number of steps.
 * @param current_step the current step
 * @param total_steps the total number of steps
 * @return a pair of two ratios where the first one represents the share of the 
 * old color and the second the share of the new color.
 */
std::pair<double, double> get_linear_color_ratio(int current_step, int total_steps);



#endif
