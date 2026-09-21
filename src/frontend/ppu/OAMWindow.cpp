#include "OAMWindow.h"

#include <imgui.h>
#include "core/Gameboy.h"

namespace ui {

void OAMWindow::render() {
	if (show) {
		ImGui::Begin("OAM", &show);
		
		for (u8 obj = 0; obj < 40; ++obj) {
			size_t rgbIndex = ((obj / 8) * 64 * 8) + ((obj % 8) * 8);
			gb.ppu.dumpTile(&tex[rgbIndex * 4], 64 * 4, gb.ppu.sprites[obj].tile, gb.ppu.sprites[obj].getBank());
		}
		
		tex.render(zoom, grid);

		ImGui::ColorEdit3("Invisible Color", (float*)&invisColor);
		PPU::invisPixel = Color((float*)&invisColor);

		ImGui::End();
	}
}

} // namespace ui
