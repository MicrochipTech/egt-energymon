#include "pac193x.h"

pac193x::pac193x() {
	dev = "/sys/bus/iio/devices/iio:device0/";
}

pac193x::~pac193x() {

}

std::string pac193x::getPacSysfsFile(const std::string f) {
	std::ifstream file(f);
	std::string val;
    
	if (file.is_open()) {
        std::getline(file, val);
    	file.close();
    } else {
		std::cerr << "Error: Unable to open PAC193x sysfs file " << f << std::endl;
	}

	return val;
}

std::string pac193x::readRawValue(int ch, pacChannelMode mode) {
	std::string str;
	std::string f;

	switch (mode) {
		case pacChannelMode::CURRENT:			
			f = dev + "in_current" + std::to_string(ch) + "_raw";
			str = getPacSysfsFile(f);
			break;

		case pacChannelMode::AVG_CURRENT:
			f = dev + "in_current" + std::to_string(ch + 4) + "_mean_raw";
			str = getPacSysfsFile(f);
			break;

		case pacChannelMode::VOLTAGE:
			f = dev + "in_voltage" + std::to_string(ch) + "_raw";
			str = getPacSysfsFile(f);
			break;

		case pacChannelMode::AVG_VOLTAGE:
			f = dev + "in_voltage" + std::to_string(ch + 4) + "_mean_raw";
			str = getPacSysfsFile(f);
			break;

		case pacChannelMode::POWER:
			f = dev + "in_power" + std::to_string(ch) + "_raw";
			str = getPacSysfsFile(f);
			break;

		case pacChannelMode::ENERGY:
			f = dev + "in_energy" + std::to_string(ch) + "_raw";
			str = getPacSysfsFile(f);
			break;

		default:
			break;
	}

	return str;
}

std::string pac193x::readScaleValue(int ch, pacChannelMode mode) {
	std::string str;
	std::string f;

	switch (mode) {
		case pacChannelMode::CURRENT:
		f = dev + "in_current" + std::to_string(ch) + "_scale";
			str = getPacSysfsFile(f);
			break;

		case pacChannelMode::AVG_CURRENT:
			f = dev + "in_current" + std::to_string(ch + 4) + "_scale";
			str = getPacSysfsFile(f);
			break;

		case pacChannelMode::VOLTAGE:
			f = dev + "in_voltage" + std::to_string(ch) + "_scale";
			str = getPacSysfsFile(f);
			break;

		case pacChannelMode::AVG_VOLTAGE:
			f = dev + "in_voltage" + std::to_string(ch + 4) + "_scale";
			str = getPacSysfsFile(f);
			break;

		case pacChannelMode::POWER:
			f = dev + "in_power" + std::to_string(ch) + "_scale";
			str = getPacSysfsFile(f);
			break;

		case pacChannelMode::ENERGY:
			f = dev + "in_energy" + std::to_string(ch) + "_scale";
			str = getPacSysfsFile(f);
			break;

		default:
			break;
	}

	return str;
}

std::string pac193x::getChannelName(int ch) {
	std::string str;
	std::string f;

	// name is the same for all modes, so just grab one of them
	f = dev + "in_current" + std::to_string(ch + 1) + "_label";
	str = getPacSysfsFile(f);

	// trim off the sub string from the name set in the dts
	size_t pos = str.find("_");
    if (pos != std::string::npos) {
        return str.substr(0, pos);
    }
	
	return str;
}

std::string pac193x::getChannelShuntValue(int ch) {
	std::string str;
	std::string f;

	f = dev + "in_shunt_resistor" + std::to_string(ch + 1);
	str = getPacSysfsFile(f);

	return str;
}

void pac193x::getChannelValue(int ch, pacChannelMode mode, double& value) {
	double v = stod(readRawValue(ch + 1, mode));
	double scale = stod(readScaleValue(ch + 1, mode));

	switch (mode) {
		case pacChannelMode::VOLTAGE:
		case pacChannelMode::AVG_VOLTAGE:
		case pacChannelMode::POWER:
			value = scale * (v / 1000);
			break;

		case pacChannelMode::ENERGY:
			value = scale * (v / (60 * 60));		
			break;

		default:
			value = v * scale;
			break;
	};
}
