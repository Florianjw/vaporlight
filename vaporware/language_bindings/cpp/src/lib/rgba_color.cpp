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


#include "rgba_color.hpp"

#include <stdexcept>
#include <cctype>
#include <iomanip>

//private function to convert two hex-characters to a byte:
uint8_t hex_to_byte(char highbyte, char lowbyte);

vlpp::rgba_color::rgba_color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha):
	red(red), green(green), blue(blue), alpha(alpha)
{}

vlpp::rgba_color::rgba_color(std::string colorcode) {
	if (!colorcode.empty() && colorcode[0] == '#') {
		colorcode.erase(0,1);
	}
	switch (colorcode.length()) {
		case 8:
			alpha = hex_to_byte(colorcode[6], colorcode[7]);
			//fallthrough
		case 6:
			red = hex_to_byte(colorcode[0], colorcode[1]);
			green = hex_to_byte(colorcode[2], colorcode[3]);
			blue = hex_to_byte(colorcode[4], colorcode[5]);
			break;
		default:
			throw std::invalid_argument("invalid colorcode");
	}
}


bool vlpp::rgba_color::operator==(const rgba_color& other) const{
	if(red != other.red) return false;
	if(green != other.green) return false;
	if(blue != other.blue) return false;
	if(alpha != other.alpha) return false;
	return true;
}

bool vlpp::rgba_color::operator!=(const rgba_color& other) const{
	return !(*this == other);
}

bool vlpp::rgba_color::operator<(const rgba_color& other) const{
	if( alpha < other.alpha ) {
		return true;
	}
	else if(alpha > other.alpha){
		return false;
	}
	else if( red < other.red ) {
		return true;
	}
	else if(red > other.red){
		return false;
	}
	else if( green < other.green ) {
		return true;
	}
	else if(green > other.green){
		return false;
	}
	else if( blue < other.blue ) {
		return true;
	}
	else if(blue > other.blue){
		return false;
	}
	return false;
}


uint8_t hex_to_byte(char highbyte, char lowbyte) {
	if (!isxdigit(highbyte) || !isxdigit(lowbyte)) {
		throw std::invalid_argument("invalid colorcode");
	}
	uint8_t returnval = 0;
	
	if (isdigit(highbyte)) {
		returnval = ((unsigned char)highbyte - '0') * 0x10;
	}
	else {
		returnval = (10 + (unsigned char)tolower(highbyte) -'a') * 0x10;
	}
	
	if (isdigit(lowbyte)) {
		returnval += (lowbyte - '0');
	}
	else {
		returnval += (10 + tolower(lowbyte) -'a');
	}
	
	return returnval;
}


std::ostream& operator<<(std::ostream& stream, const vlpp::rgba_color& col){
	stream << "#" << std::hex << std::setfill('0') 
	       << std::setw(2) << static_cast<int>(col.red)
	       << std::setw(2) << static_cast<int>(col.green)
	       << std::setw(2) << static_cast<int>(col.blue)
	       << std::setw(2) << static_cast<int>(col.alpha); 
	return stream;
}
