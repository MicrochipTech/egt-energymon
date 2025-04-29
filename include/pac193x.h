#ifndef __PAC193X_H__
#define __PAC193X_H__

#include <egt/ui>
#include <egt/window.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <algorithm>

using namespace std;
using namespace egt;
using namespace egt::experimental;
namespace fs = std::filesystem;

typedef enum pacChan {
	CH1,
	CH2,
	CH3,
	CH4,
	PAC_MAX_CHANNELS,
} pacChan;

typedef enum pacChannelMode {
	VOLTAGE,
    AVG_VOLTAGE,
    CURRENT,
    AVG_CURRENT,
	POWER,
	ENERGY,
} pacChannelMode;

class pac193x {
    public:
        explicit pac193x();
        virtual ~pac193x();

        void getChannelValue(int ch, pacChannelMode mode, double& value);
        std::string getChannelName(int ch);
        std::string getChannelShuntValue(int ch);

    private:
        std::string dev;
        std::string getPacSysfsFile(const std::string f);
        std::string readRawValue(int ch, pacChannelMode mode);
        std::string readScaleValue(int ch, pacChannelMode mode);
};

std::ostream& operator<<(std::ostream& os, const pacChannelMode& mode);

#endif
