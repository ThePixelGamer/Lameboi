#include "OAMWindow.h"

#include <imgui.h>
#include "core/Gameboy.h"

namespace ui {

void OAMWindow::render() {
	if (show) {
		ImGui::Begin("OAM", &show);

		gb.ppu.dumpSprites(tex);
		tex.render(zoom, grid);

		ImGui::ColorEdit3("Invisible Color", (float*)&invisColor);
		PPU::invisPixel = Color((float*)&invisColor);

		ImGui::End();
	}
}

} // namespace ui