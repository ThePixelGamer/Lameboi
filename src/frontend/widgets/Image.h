#pragma once

#include <array>
#include <format>
#include <functional>

#include <imgui.h>

#include "render/Texture.h"
#include "util/Types.h"

namespace ui {

// ui widget that handles storage of pixel data and transfer to gltexture
template <size_t Width, size_t Height>
class Image {
public:
    constexpr static size_t W = Width;
    constexpr static size_t H = Height;

	using Container = std::array<u8, Width * Height * 4>;

private:
    Container pixels{};
    bool dirty = false;

    ImageTexture tex;
    ImVec2 u{ 0, 0 }, v{ 1, 1 };

public:
    Image() : tex(W, H, pixels.data()) {
        pixels.fill(0xFF);
    }

	struct Callback {
		using DrawExtra = std::function<void(const ImVec2& tl, const ImVec2& br, float zoom)>;
		using Click = std::function<void(u32 x, u32 y)>;
		using Hover = std::function<void(u32 x, u32 y)>;

		DrawExtra extra = nullptr;
		Click click = nullptr;
		Hover hover = nullptr;

		operator bool() {
			return extra || click || hover;
		}
	};

    void render(float zoom, bool grid = false, Callback callback = {}) {
        float adjWidth = W * v.x * zoom;
        float adjHeight = H * v.y * zoom;

        ImDrawList* drawlist = ImGui::GetWindowDrawList();

        if (dirty) {
            tex.update();
        }
        
        // hack: force sampler to nearest
        if (tex.FilterValue == GL_NEAREST) {
            if (auto nearest = ImGui::GetPlatformIO().DrawCallback_SetSamplerNearest)
                drawlist->AddCallback(nearest);
        }

        // todo: use ImageButton or Image based on clickCallback? 
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0, 0 });
		auto texid = tex.as<ImTextureID>();
		std::string imageid = std::format("##render{}", texid);
        bool clicked = ImGui::ImageButton(imageid.c_str(), texid, ImVec2(adjWidth, adjHeight), u, v);
        ImGui::PopStyleVar();

        // exit early if we're rendering a simple image
        if (!grid && !callback) {
            return;
        }

        // get coordinates to render over image
        ImVec2 tl = ImGui::GetItemRectMin();
        ImVec2 br = ImGui::GetItemRectMax();

        float region_sz = 8.0f * zoom; //region = 8x8 area
        auto& mouse = ImGui::GetIO().MousePos;
        u32 region_x = std::clamp((mouse.x - tl.x) / region_sz, 0.0f, adjWidth);
        u32 region_y = std::clamp((mouse.y - tl.y) / region_sz, 0.0f, adjHeight);

        // todo: relook over this and verify the logic is sane 💀
        if (grid) {
            constexpr auto line_color = IM_COL32(169, 169, 169, 255);
            float line_dist = 8.0f * zoom;

            for (float x = tl.x + line_dist; x <= br.x - line_dist; x += line_dist) {
                drawlist->AddLine(ImVec2(x, tl.y), ImVec2(x, br.y), line_color);
            }

            for (float y = tl.y + line_dist; y <= br.y - line_dist; y += line_dist) {
                drawlist->AddLine(ImVec2(tl.x, y), ImVec2(br.x, y), line_color);
            }

            // todo: I think I might want to move this to a callback?
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();

                ImGui::Text("Coordinate: (%d, %d)", region_x, region_y);

				if (callback.hover) {
					callback.hover(region_x, region_y);
				}

                constexpr float mini_zoom = 4.0f;
                ImVec2 uv0 = ImVec2((region_x * region_sz) / adjWidth, (region_y * region_sz) / adjHeight);
                ImVec2 uv1 = ImVec2(((region_x + 1.0f) * region_sz) / adjWidth, ((region_y + 1.0f) * region_sz) / adjHeight);
                ImGui::Image(tex.as<ImTextureID>(), ImVec2(region_sz * mini_zoom, region_sz * mini_zoom), uv0, uv1);

                ImGui::EndTooltip();
            }
        }

        if (callback.extra) {
            callback.extra(tl, br, zoom);
        }

        if (clicked && callback.click) {
            callback.click(region_x, region_y);
        }
    }

    // todo: remove
    void setSize(float x, float y) {
        v.x = x / W;
        v.y = y / H;
    }

    operator Container&() {
        dirty = true;
        return pixels;
    }

    Container& data() {
        dirty = true;
        return pixels;
    }
};

}
