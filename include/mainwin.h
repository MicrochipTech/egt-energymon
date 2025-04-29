#ifndef MAINWIN_H_
#define MAINWIN_H_

#include <egt/ui>
#include <egt/window.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include "energymon.h"
#include "pac193x.h"

using namespace std;
using namespace egt;
using namespace egt::experimental;

class mainWin : public TopWindow {
public:
	explicit mainWin();
	virtual ~mainWin();

	shared_ptr<energyMon> mon;
	shared_ptr<PeriodicTimer> cputimer;
	shared_ptr<egt::StaticGrid> grid;

private:

	std::vector<std::thread> threads;
	std::atomic<bool> maxCPU;

	std::vector<std::string> cpuFrequencies;
	std::vector<std::string> cpuGovernors;

	bool checkConnectionStatus(const std::string& iface) {
		struct ifaddrs* ifaddr;
		if (getifaddrs(&ifaddr) == -1) {
			perror("getifaddrs");
			return false;
		}

		bool exists = false;
		for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
			if (ifa->ifa_name == iface && (ifa->ifa_flags & IFF_RUNNING) && (ifa->ifa_flags & IFF_UP)) {
				exists = true;
				break;
			}
		}

		freeifaddrs(ifaddr);
		return exists;
	}

	void getAvailScalingFrequencies(std::vector<std::string>& freqs) {
#ifdef CONFIG_X86_64
		freqs = { "90 MHz", "250 MHz", "600 MHz", "800 MHz", "1000 MHz" };
#else
		std::ifstream file("/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies");
		std::string freq;
		if (file.is_open()) {
			while (file >> freq) {
				int f = std::stoi(freq);
				freqs.push_back(std::to_string(f / 1000) + " MHz");

			}
			file.close();
		} else {
			cerr << "Could not get list of scaling frequencies" << endl;
		}
#endif
	}

	void getAvailScalingGovernors(std::vector<std::string>& governors) {
#ifdef CONFIG_X86_64
		governors = { "conservative", "ondemand", "userspace", "powersave", "performance" };
#else
		std::ifstream file("/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors");
		std::string governor;
		if (file.is_open()) {
			while (file >> governor) {
				governors.push_back(governor);
			}
			file.close();
		} else {
			cerr << "Could not get list of scaling governors" << endl;
		}
#endif
	}

	std::string getCpuScalingGovernor(void) {
		std::ifstream file("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
		std::string governor;

		if (file.is_open()) {
			file >> governor;
			file.close();
		} else {
			cerr << "Could not Get CPU scaling governor" << std::endl;
		}

		return governor;
	}

	void setCpuScalingGovernor(std::string governor) {
#ifdef CONFIG_X86_64
		cout << "Setting CPU governor to " << governor << endl;
#else
		std::ofstream file("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
		if (file.is_open()) {
			file << governor;
			file.close();
		} else {
			cerr << "Could not set CPU scaling governor" << std::endl;
		}
#endif
	}

	std::string getCpuFrequency(bool inMHz) {
		std::ifstream file("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");

		std::string freq;
		
		if (file.is_open()) {
			std::getline(file, freq);
			file.close();

			if (inMHz == true) {
				int f;
				
				f = std::stoi(freq);
				return std::to_string(f / 1000);
			} else {
				return freq;
			}
		} else {
			std::cerr << "Error: Unable to open scaling_cur_freq" << std::endl;
		}

		return freq;
	}

	void setCpuFrequency(const std::string& freq, bool fromMHz) {
		std::string frequency;

		if (fromMHz == true) {
			frequency = std::to_string(stoi(freq) * 1000);
		} else {
			frequency = freq;
		}
		
#ifdef CONFIG_X86_64
		cout << "Setting CPU frequency to " << frequency << endl;
#else
		std::ofstream file("/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed");
		if (file.is_open()) {
			file << frequency;
			file.close();
		} else {
			std::cerr << "Error: Unable to open scaling_setspeed" << std::endl;
		}
#endif
	}

	void maxTheCPU(void) {
		volatile long long sum = 0;
		while (maxCPU == true) {
			for (long long i = 0; i < 1e7; ++i) {
				sum += i;
			}
			sum = 0;

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
};

#endif /* MAINWIN_H_ */
