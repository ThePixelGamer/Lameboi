#pragma once

#include "core/Gameboy.h"

#include "imgui_mem_editor.h"

inline ImU8 MemEditorRead(const ImU8* mem_, size_t offset) {
	// maybe should make read const
	auto* mem = reinterpret_cast<Memory*>(const_cast<ImU8*>(mem_));
	return mem->read(static_cast<u16>(offset & 0xFFFF));
}

inline void MemEditorWrite(ImU8* mem_, size_t offset, ImU8 data) {
	auto* mem = reinterpret_cast<Memory*>(mem_);
	mem->write(static_cast<u16>(offset & 0xFFFF), data);
}

namespace ui {
	class MemoryWindow {
		std::shared_ptr<Gameboy> gb;

		MemoryEditor mem_edit;
		bool& show;
	
	public:
		MemoryWindow(std::shared_ptr<Gameboy> gb, bool& show) :
			gb(gb),
			show(show)
		{
			mem_edit.ReadFn = &MemEditorRead;
			mem_edit.WriteFn = &MemEditorWrite;
		}

		void render() {
			if (show) {
				mem_edit.DrawWindow("Memory", &gb->mem, 0x10000);
			}
		}
	};
}