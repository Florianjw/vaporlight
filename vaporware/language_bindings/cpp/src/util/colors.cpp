#include "colors.hpp"

#include <sstream>
#include <set>

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


using namespace vlpp;

std::vector<rgba_color> str_to_cols(const std::string& str){
	using std::string;
	using std::set;
	set<rgba_color> colors;
	std::istringstream data(str);
	string tmp;
	while(getline(data, tmp, ',')){
		auto key_it = COLOR_SETS_MAP.find(tmp);
		if(key_it != COLOR_SETS_MAP.end()){
			for(auto& col: key_it->second){
				colors.insert(col);
			}
		}
		else{
			colors.insert(str_to_col(tmp));
		}
	}
	return {colors.begin(), colors.end()};
}

vlpp::rgba_color str_to_col(const std::string& str){
	auto col_map_it = COLOR_MAP.find(str);
	if(col_map_it != COLOR_MAP.end()){
		return col_map_it->second;
	}
	else{
		return {str};
	}
}
