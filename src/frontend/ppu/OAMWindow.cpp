#include "OAMWindow.h"

#include "core/Gameboy.h"

namespace ui {

void OAMWindow::render() {
	if (show) {
		ImGui::Begin("OAM", &show);

		gb.ppu.dumpSprites(pixels);
		tex.update();
		tex.render(zoom, grid);

		ImGui::ColorEdit3("Invisible Color", invisColor.data());
		PPU::invisPixel = invisColor;

		ImGui::End();
	}
}

} // namespace ui