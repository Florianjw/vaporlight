#include "core.hpp"

#include <cstdint>
#include <random>
#include <chrono>
#include <mutex>

#include "settings.hpp"

#include <cstdio>

void control_LEDs(const std::vector<uint16_t>& led_list) {
	// first set up the random-number-generators:
	std::default_random_engine generator(
		static_cast<unsigned long>(std::chrono::system_clock::now().time_since_epoch().count()) );
	std::uniform_int_distribution<useconds_t> sleep_time_distribution(
			settings::min_sleep_time, settings::max_sleep_time);
	std::uniform_int_distribution<useconds_t> fade_time_distribution(
			settings::min_fade_time, settings::max_fade_time);
	std::uniform_int_distribution<size_t> color_distribution(0, settings::colorset.size() - 1);
	
	// and now start the actual work:
	vlpp::rgba_color last_color;
	while(true){
		if(settings::thread_return_flag){
			return;
		}
		vlpp::rgba_color tmp = settings::colorset.at(color_distribution(generator));
		if(tmp == last_color){
			continue;
		}
		fade_to(led_list, fade_time_distribution(generator), last_color, tmp);
		last_color = tmp;
		usleep (sleep_time_distribution(generator));
	}
}

void fade_to(const std::vector<uint16_t>& led_list,
		useconds_t fade_time, const vlpp::rgba_color& old_color,
		const vlpp::rgba_color& new_color){
	useconds_t time_per_step = fade_time / settings::fade_steps;
	for(int i=0; i < settings::fade_steps; ++i){
		auto color_shares = settings::color_ratio_function(i, settings::fade_steps);
		auto p_new = color_shares.first;
		auto p_old = color_shares.second;
		vlpp::rgba_color tmp{
			// i really WANT this narrowing conversion:
			uint8_t(old_color.red*p_old + new_color.red*p_new),
			uint8_t(old_color.green*p_old + new_color.green*p_new),
			uint8_t(old_color.blue*p_old + new_color.blue*p_new),
			uint8_t(old_color.alpha*p_old + new_color.alpha*p_new)
		};
		set_leds(led_list, tmp);
		usleep(time_per_step);
	}
	set_leds(led_list, new_color);
}

std::pair<double, double> get_linear_color_ratio(int current_step, int total_steps){
	double tmp = static_cast<double>(current_step) / total_steps;
	return {tmp, 1 - tmp};
}

void set_leds(const std::vector<uint16_t>& led_list, const vlpp::rgba_color& col){
	static std::mutex m;
	std::lock_guard<std::mutex> lock(m);
	for(auto led: led_list){
		settings::client.set_led(led, col);
	}
	settings::client.flush();
}
