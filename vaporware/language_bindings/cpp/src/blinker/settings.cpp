#include "settings.hpp"

#include <iostream>
#include <boost/program_options.hpp>

#include "core.hpp"

#include "../util/ids.hpp"
#include "../util/colors.hpp"

std::string settings::server;
std::string settings::token;
uint16_t settings::port;
std::string settings::led_string;
bool settings::synced = false;
int settings::fade_steps = UINT8_MAX;
useconds_t settings::min_sleep_time = 0;
useconds_t settings::max_sleep_time = 100000;
useconds_t settings::min_fade_time  = 0;
useconds_t settings::max_fade_time  = 100000;
std::vector<vlpp::rgba_color> settings::colorset = REAL_COLORS;
vlpp::client settings::client;
std::atomic<bool> settings::thread_return_flag(false);
std::function<std::pair<double,double>(int, int)> settings::color_ratio_function = get_linear_color_ratio;



return_action set_options(int argc, char** argv) {
	using boost::program_options::value;
	
	std::string tmp_colorset_str;
	
	boost::program_options::options_description desc;
	desc.add_options()
		("help,h", "print this help")
		("sync,y", "makes the LEDs blink asynchronus.")
		("token,t", value<std::string>(&settings::token), "sets the authentication-token")
		("server,s", value<std::string>(&settings::server), "sets the servername")
		("port,p", value<uint16_t>(&settings::port)->default_value(vlpp::client::DEFAULT_PORT),
		 "sets the server-port")
		("leds,l", value<std::string>(&settings::led_string), "sets the number of leds")
		("min-sleep", value<useconds_t>(&settings::min_sleep_time),
		 "changes the minimum sleep-time")
		("max-sleep,S", value<useconds_t>(&settings::max_sleep_time),
		 "changes the maximum sleep-time")
		("colors,c", value<std::string>(&tmp_colorset_str), "sets the used colorset")
		("min-fade", value<useconds_t>(&settings::min_fade_time), "changes the minimum fade time")
		("max-fade,f", value<useconds_t>(&settings::max_fade_time), "changes the maximum fade time")
		("fade-steps,F", value<int>(&settings::fade_steps), "sets the number of steps for fading");

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return return_action::exit_succesfull;
	}
	
	if(settings::fade_steps < 1){
		std::cerr << "Error: There must be one step at minimum to fade between colors." << std::endl;
		return return_action::exit_failed;
	}
	
	if(vm.count("sync")){
		settings::synced = true;
	}
	settings::colorset = str_to_cols(tmp_colorset_str);
	return return_action::continue_execution;
}
