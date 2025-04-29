#ifndef __ENERGYMON_H__
#define __ENERGYMON_H__

#include <egt/painter.h>
#include <egt/ui>
#include <egt/window.h>
#include <vector>
#include "pac193x.h"
#include <cairo.h>

using namespace std;
using namespace egt;
using namespace egt::experimental;

#define VOLTAGE_MAX         4.0F        // 4.0 V
#define AVG_VOLTAGE_MAX     VOLTAGE_MAX // 4.0 V
#define CURRENT_MAX         1000.0F     // 1000 mA
#define AVG_CURRENT_MAX     CURRENT_MAX // 1000 mA
#define POWER_MAX           1000.0F     // 1000 mW
#define ENERGY_MAX          50000.0F    // 50000 mWh

typedef std::map<pacChannelMode, double> channelMaxVals;

#ifdef CONFIG_X86_64
typedef std::map<pacChan, double> channelMinVals;
const std::string dummy_channels[4] = {"VDD3V3", "VDDIODDR", "VDDCORE", "VDDCPU"};

const channelMinVals channelVMin{{pacChan::CH1, 3.290F},
                                   {pacChan::CH2, 1.300F},
                                   {pacChan::CH3, 1.040F},
                                   {pacChan::CH4, 1.040F}};

const channelMinVals channelVMax{{pacChan::CH1, 3.350F},
                                   {pacChan::CH2, 1.400F},
                                   {pacChan::CH3, 1.060F},
                                   {pacChan::CH4, 1.060F}};
#else

const channelMaxVals channelMax{{pacChannelMode::VOLTAGE, VOLTAGE_MAX},
                                   {pacChannelMode::AVG_VOLTAGE, AVG_VOLTAGE_MAX},
                                   {pacChannelMode::CURRENT, CURRENT_MAX},
                                   {pacChannelMode::AVG_CURRENT, AVG_CURRENT_MAX},
                                   {pacChannelMode::POWER, POWER_MAX},
                                   {pacChannelMode::ENERGY, ENERGY_MAX}};
#endif

struct CustomRadial : public egt::experimental::RadialF {
	CustomRadial(const std::string& title, const std::string& units)
			: m_title(title), m_units(units) {}

	using RadialF::RadialF;

	void updateTitle(const std::string& title) {
		m_title = title;
	}

    void updateUnits(const std::string& units) {
		m_units = units;
	}

	void draw(egt::Painter& painter, const egt::Rect& rect) override {
		using namespace egt;

        draw_box(painter, Palette::ColorId::bg, Palette::ColorId::border);

        const auto b = content_area();
        const auto c = b.center();
        auto text = this->text();
        const auto smalldim = std::min(b.width(), b.height());

        DefaultDim maxwidth = 0;
        for (auto& value : m_values) {
            if (value.width > maxwidth)
                maxwidth = value.width;
        }

        for (auto& value : m_values) {
            const auto radius = smalldim * 0.5f - (maxwidth * 0.5f);
            const auto angle1 = detail::to_radians<float>(-90, start_angle());
            const auto angle2 = detail::to_radians<float>(-90,
                                value_to_degrees(value.range->start(),
                                        value.range->end(),
                                        value.range->value()));

            painter.set(value.color);
            painter.line_width(value.width);
            if (value.flags.is_set(RadialFlag::rounded_cap))
                painter.line_cap(Painter::LineCap::round);
            else
                painter.line_cap(Painter::LineCap::butt);

            painter.draw(Arc(c, radius, angle1, angle2));
            painter.stroke();

            if (value.flags.is_set(RadialFlag::text_value)) {
                text = detail::format(value.range->value(), 3);
            }
        }

        if (!text.empty()) {
            auto target = Rect(Size(smalldim, smalldim));
            target.move_to_center(b.center());
            auto font = TextWidget::scale_font(target.size(), text,
                                               this->font());
            detail::draw_text(painter, 
                content_area(), m_title + "\n" + text + "\n" + m_units,
                font,
                {},
                AlignFlag::center,
                Justification::middle,
                color(Palette::ColorId::label_text));
        }
    }

	private:
	std::string m_title;
    std::string m_units;
};

class energyMon : public TopWindow {
public:
	explicit energyMon(TopWindow *parent = nullptr);
	virtual ~energyMon();

private:
	shared_ptr<pac193x> pac;
	vector<shared_ptr<RangeValue<float>>> ranges;
    shared_ptr<PeriodicTimer> sampleTimer;

    pacChannelMode channelMode[PAC_MAX_CHANNELS];

    void setChannelMode(int ch, pacChannelMode mode) {
        channelMode[ch] = mode;
    }

    pacChannelMode getChannelMode(int ch) {
        pacChannelMode mode = channelMode[ch];
        return mode;
    }

	void updateChannel(int channel);
};

#endif
