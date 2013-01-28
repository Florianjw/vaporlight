/*
 *  This file is part of vaporpp.
 *
 *  vaporpp is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  vaporpp is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with vaporpp.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <cassert>

#include <unistd.h>

#include <boost/algorithm/string.hpp>

#include "../lib/client.hpp"
#include "../util/ids.hpp"
#include "../util/colors.hpp"
#include "../util/signalhandling.hpp"

#include "settings.hpp"
#include "core.hpp"


/*
 * this program will make all lights blink
 */
int main(int argc, char** argv) {
	using std::string;
	
	std::vector<uint16_t> LEDs;

	// this try-block may not be the best style,
	// but it is required to enforce stack-unwinding
	// in the case of a runtime error:
	try {
		signalhandling::init( {SIGINT});
		
		switch(set_options(argc, argv)){
			case return_action::continue_work:
				break;
			case return_action::exit_succesfull:
				return 0;
			case return_action::exit_failed:
				return 1;
			default:
				assert(false);
		}
		
		LEDs = str_to_ids(settings::led_string);
		settings::client = vlpp::client(settings::server, settings::token, settings::port);
		std::vector<std::thread> threads;
		if (settings::synced) {
			threads.emplace_back(control_LEDs, LEDs);
		}
		else {
			for (auto LED : LEDs) {
				threads.emplace_back(control_LEDs, std::vector<uint16_t> {LED});
			}
		}
		
		// wait until SIGINT arrives, than call the threads back:
		// TODO: make this more robust
		while (true) {
			if (signalhandling::get_last_signal()) {
				settings::thread_return_flag = true;
				break;
			}
			usleep(50000);
		}
		for (auto& thread : threads) {
			thread.join();
		}
		return 0;
	}
	catch (vlpp::connection_failure&) {
		std::cerr << "Error: connection failure" << std::cerr;
		return 2;
	}
	catch (std::runtime_error& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 2;
	}
}

