#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <cstdint>
#include <unistd.h>
#include <atomic>
#include <functional>
#include <utility>

#include "../lib/client.hpp"
#include "../util/colors.hpp"

/**
 * @brief This struct contains the global stuff.
 */
struct settings{
	/**
	 * @brief The hostname of the server.
	 */
	static std::string server;
	
	/**
	 * @brief The authentication-token.
	 */
	static std::string token;
	
	/**
	 * @brief The server-port.
	 */
	static uint16_t port;
	
	/**
	 * @brief This is the commandline-argument, that identifies the controlled LEDs.
	 */
	static std::string led_string;
	
	/**
	 * @brief Whether all LEDs shall change their color synchronusly.
	 */
	static bool synced;
	
	/**
	 * @brief The number of steps between two colors when fading between them.
	 */
	static int fade_steps;
	
	/**
	 * @brief The minimal ammount of time that a color won't change.
	 */
	static useconds_t min_sleep_time;
	
	/**
	 * @brief The maximum ammount of time, that a color won't change.
	 */
	static useconds_t max_sleep_time;
	
	/**
	 * @brief The minimum ammount of time that will be needed to fade between colors.
	 */
	static useconds_t min_fade_time;
	
	/**
	 * @brief The maximum ammount of time that will be needed to fade between colors.
	 */
	static useconds_t max_fade_time;
	
	/**
	 * @brief The colorset that will be used to fade between.
	 */
	static std::vector<vlpp::rgba_color> colorset;
	
	/**
	 * @brief The actual client that holds the connection to the server.
	 */
	static vlpp::client client;
	
	/**
	 * @brief A flag that tells the controll-threads to return, if set to true.
	 */
	static std::atomic<bool> thread_return_flag;
	
	/**
	 * @brief The function that will be used to calculate the ratio between two colors when fading.
	 */
	static std::function<std::pair<double,double>(int, int)> color_ratio_function;
};

/**
 * @brief The action that the program shall perform after the arguments are processed.
 */
enum class return_action{
	exit_succesfull,
	exit_failed,
	continue_work //this would be just 'continue', if that wouldn't be a keyword
};

/**
 * @brief Parses the commandline-options and prepares the global settings.
 * @param argc the argument-count
 * @param argv the arguments
 */
return_action set_options(int argc, char **argv);


#endif

