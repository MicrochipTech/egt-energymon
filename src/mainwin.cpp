#include <iostream>
#include <egt/themes/lapis.h>
#include "mainwin.h"

EGT_EMBED(connected, "../resources/connected.png");
EGT_EMBED(disconnected, "../resources/disconnected.png");

mainWin::mainWin() {
	global_theme(std::make_unique<egt::LapisTheme>());

	maxCPU = false;

	//initCpuScaling();
	getAvailScalingGovernors(cpuGovernors);
	getAvailScalingFrequencies(cpuFrequencies);

	auto title = make_shared<Label>("Energy Monitoring");
	title->align(AlignFlag::expand_horizontal);
	title->font(egt::Font(28));

	auto mainWinVsizer = make_shared<VerticalBoxSizer>();
	expand(mainWinVsizer);
	add(mainWinVsizer);

	auto topFrame = make_shared<Frame>(Size(0, 60));
	mainWinVsizer->add(expand_horizontal(topFrame));

	topFrame->add(title);

	grid = make_shared<egt::StaticGrid>(egt::StaticGrid::GridSize(2, 3));
	grid->vertical_space(10);
	mainWinVsizer->add(egt::expand(grid));

	auto CPULabel = make_shared<Label>("CPU:---", Rect(0, 0, 100, 50));
	auto freqLabel = make_shared<Label>(getCpuFrequency(true) + " MHz", Rect(160, 0, 100, 50));
	auto frequencies = make_shared<egt::Scrollwheel>(egt::Rect(0, 0, 150, 200), cpuFrequencies);
	auto governors = make_shared<egt::Scrollwheel>(egt::Rect(0, 0, 200, 200), cpuGovernors);
	frequencies->font(egt::Font(28));
	governors->font(egt::Font(28));

	for (const auto& governor : cpuGovernors) {
#ifdef CONFIG_X86_64
		if (governor == "conservative") {
#else
		if (governor == getCpuScalingGovernor()) {
#endif
			if (governor != "userspace") {
				frequencies->disable();
			}

			governors->selected(&governor - &cpuGovernors[0]);
		}
	}

	governors->on_value_changed([this, governors, frequencies]() {
		if (governors->value() != "userspace") {
			frequencies->disable();

		} else {
			frequencies->enable();
		}

		setCpuScalingGovernor(governors->value());
	});

	for (const auto& freq : cpuFrequencies) {
#ifdef CONFIG_X86_64
		if (freq == "800 MHz") {
#else
		if (freq == getCpuFrequency(true) + " MHz") {
#endif
			frequencies->selected(&freq - &cpuFrequencies[0]);
		}
	}

	frequencies->on_value_changed([this, frequencies, freqLabel]() {		
		// trim off the "MHz" from the display string
		std::stringstream ss(frequencies->value());;
        std::string freq;
        ss >> freq;

		setCpuFrequency(freq, true);

	});

	auto optionHsizer = make_shared<HorizontalBoxSizer>();
	optionHsizer->add(frequencies);
	optionHsizer->add(governors);
	grid->add(expand(optionHsizer), 0, 2);

	auto pushCPU = make_shared<Button>("Start CPU Test", AlignFlag::center);
	pushCPU->font(egt::Font(28));
	pushCPU->on_click([this, pushCPU](egt::Event&) {
		if (maxCPU == true) {
			maxCPU = false;
			pushCPU->text("Start CPU Test");

			for (auto& t : threads) {
				t.join();
			}
			threads.clear();
		} else {
			maxCPU = true;
			pushCPU->text("Stop CPU Test");
			int numThreads = std::thread::hardware_concurrency();

			for (unsigned int i = 0; i < numThreads; ++i) {
				threads.emplace_back(&mainWin::maxTheCPU, this);
			}
		}
	});

	grid->add(pushCPU, 1, 2);

	auto tools = make_shared<CPUMonitorUsage>();

	auto bottomFrame = make_shared<Frame>(Size(0, 60));

	CPULabel->font(egt::Font(28));
	freqLabel->font(egt::Font(28));

	bottomFrame->add(CPULabel);
	bottomFrame->add(freqLabel);

	auto connection = make_shared<ImageLabel>(egt::Image("res:disconnected"), "");
	
	if (checkConnectionStatus("eth0") == true) {
		connection->image(egt::Image("res:connected"));
	}

	connection->align(AlignFlag::right);
	connection->image_align(AlignFlag::right);
	connection->font(egt::Font(28));

	bottomFrame->add((connection));

	mainWinVsizer->add(bottom(expand_horizontal(bottomFrame)));

	cputimer = make_shared<PeriodicTimer>(chrono::seconds(1));

	cputimer->on_timeout([this, tools, CPULabel, freqLabel, connection]() {
		tools->update();
		ostringstream ss;
		
		ss << "CPU: " << std::setfill('0') << std::setw(3) << static_cast <int>(tools->usage()) << "%";
		
		CPULabel->text(ss.str());
		freqLabel->text(getCpuFrequency(true) + " MHz");

		if (checkConnectionStatus("eth0") == true) {
			connection->image(egt::Image("res:connected"));
		} else {
			connection->image(egt::Image("res:disconnected"));
		}
	});

	cputimer->start();

	mon = make_shared<energyMon>(this);
}

mainWin::~mainWin() {

}
