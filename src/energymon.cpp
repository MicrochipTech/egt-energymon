#include <random>
#include "energymon.h"
#include "mainwin.h"

template<>
const std::pair<pacChannelMode, char const*> detail::EnumStrings<pacChannelMode>::data[] =
{
    {pacChannelMode::VOLTAGE, "V"},
    {pacChannelMode::AVG_VOLTAGE, "V AVG"},
    {pacChannelMode::CURRENT, "mA"},
    {pacChannelMode::AVG_CURRENT, "mA AVG"},
    {pacChannelMode::POWER, "mW"},
    {pacChannelMode::ENERGY, "mWh"},
};

std::ostream& operator<<(std::ostream& os, const pacChannelMode& mode)
{
    return os << detail::enum_to_string(mode);
}

#ifdef CONFIG_X86_64
static double genRandomData(double min, double max) {
	std::random_device rd;
    std::mt19937 eng(rd());

    std::uniform_real_distribution<> distr(min, max);

    return distr(eng);
}
#endif

energyMon::energyMon(TopWindow *parent) {
	auto mainwin = static_cast<mainWin*>(parent);
	pac = make_shared<pac193x>();

	auto range = make_shared<egt::RangeValue<float>>(0.0, VOLTAGE_MAX, VOLTAGE_MAX);

	for (int channel=CH1; channel < PAC_MAX_CHANNELS; channel++) {
		double value;
		std::string title;

		setChannelMode(channel, pacChannelMode::VOLTAGE);
#ifdef CONFIG_X86_64
		value = genRandomData(channelVMin.at((pacChan)channel), channelVMax.at((pacChan)channel));
		title = dummy_channels[channel];
#else
		pac->getChannelValue(channel, pacChannelMode::VOLTAGE, value);
		title = pac->getChannelName(channel);
#endif
		auto radial = make_shared<CustomRadial>(title, detail::enum_to_string(getChannelMode(channel)));
		radial->add(range, egt::Palette::grey, 10);

		auto rangeCh = make_shared<RangeValue<float>>(0.0, VOLTAGE_MAX, value);
		radial->font(egt::Font(28));
		radial->add(rangeCh, Palette::blue, 5, RadialF::RadialFlag::text_value);

		radial->on_event([this, chan = channel, radial](Event & event) {
			if (event.id() == EventId::pointer_click) {
				auto rangeCh = ranges.at(chan);
				double value;
				std::string title;

				int mode = (int)getChannelMode(chan);
				
				if (mode == (int)pacChannelMode::ENERGY) {
					mode = (int)pacChannelMode::VOLTAGE;
				} else {
					mode++;
				}

#ifdef CONFIG_X86_64
				title = dummy_channels[chan];
#else
				title = pac->getChannelName(chan);
#endif
				radial->updateTitle(title);
				radial->updateUnits(detail::enum_to_string((pacChannelMode)mode));

				setChannelMode(chan, (pacChannelMode)mode);
				updateChannel(chan);
			}
		});

		ranges.push_back(rangeCh);

		mainwin->grid->add(egt::expand(radial));
	}

	sampleTimer = make_shared<PeriodicTimer>(std::chrono::seconds(1));

	sampleTimer->on_timeout([this]() {
		for (int channel=CH1; channel < PAC_MAX_CHANNELS; channel++) {
			updateChannel(channel);
		}
	});

	sampleTimer->start();
}

energyMon::~energyMon() {

}

void energyMon::updateChannel(int channel) {
	double value;
	pacChannelMode mode;
	auto rangeCh = ranges.at(channel);

	mode = getChannelMode(channel);
#ifdef CONFIG_X86_64
	value = genRandomData(channelVMin.at((pacChan)channel), channelVMax.at((pacChan)channel));
	rangeCh->end(channelVMax.at((pacChan)channel));
#else
	pac->getChannelValue(channel, mode, value);
	rangeCh->end(channelMax.at(mode));
#endif

	rangeCh->value(value);
}
