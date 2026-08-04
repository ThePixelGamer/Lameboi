#include "APUWindow.h"

#include <implot.h>
#include <string>

#include "core/Gameboy.h"

namespace ui {

struct ScrollingBuffer {
	int max;
	int offset;
	ImVector<ImVec2> data;

    ScrollingBuffer(int max_size = 2000) {
        max = max_size;
        offset = 0;
        data.reserve(max);
    }
    void AddPoint(float x, float y) {
        if (data.size() < max)
            data.push_back(ImVec2(x,y));
        else {
            data[offset] = ImVec2(x,y);
            offset = (offset + 1) % max;
        }
    }
    void Erase() {
        if (data.size() > 0) {
            data.shrink(0);
            offset = 0;
        }
    }
};

void APUWindow::render() {
	if (!show) return;
	
	ImGui::Begin("APU", &show);

	static ScrollingBuffer dataAnalog[4];

	static float t = 0, last_t = 0;
	t += ImGui::GetIO().DeltaTime;
	if ((t - last_t) >= 0.01f) {
		dataAnalog[0].AddPoint(t, sinf(2 * t));
		dataAnalog[1].AddPoint(t, gb.apu.square.sample());
		dataAnalog[2].AddPoint(t, gb.apu.wave.sample());
		dataAnalog[3].AddPoint(t, gb.apu.noise.sample());
	}

	auto plot = [&](const char* label, ScrollingBuffer& analog) {
		if (ImPlot::BeginPlot(label)) {
			ImPlot::SetupAxisLimits(ImAxis_X1, t - 2.0, t, ImGuiCond_Always);
			ImPlot::SetupAxisLimits(ImAxis_Y1, -1.5, 1.5, ImGuiCond_Always);
			if (analog.data.size() > 0) {
				ImPlot::PlotLine(label, &analog.data[0].x, &analog.data[0].y, analog.data.size(), {
					ImPlotProp_Offset, analog.offset,
					ImPlotProp_Stride, 2 * sizeof(float)
				});
			}
			ImPlot::EndPlot();
		}
	};

	plot("Square (Sweep)", dataAnalog[0]);
	plot("Square", dataAnalog[1]);
	plot("Wave", dataAnalog[2]);
	plot("Noise", dataAnalog[3]);


	ImGui::End();
}

}
