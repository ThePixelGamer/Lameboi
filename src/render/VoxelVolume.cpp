#include "VoxelVolume.h"

#include "util/MemoryMap.h"

inline glm::vec3 col_to_vec(u32 c) {
	return glm::vec3((c & 0xFF) / 255.0f, ((c >> 8) & 0xFF) / 255.0f, (c >> 16) / 255.0f); 
}

inline glm::vec3 col_to_vec(u8 r, u8 g, u8 b) {
	return glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f); 
}

#define MAKE_VOX_CHUNK_ID(c0,c1,c2,c3) ( (c0<<0) | (c1<<8) | (c2<<16) | (c3<<24) )

void VoxelVolume::load(std::filesystem::path path) {
	shader.use();
	shader.setInt("volume", 0);

	MemoryMap file{path, Access::Read, 0};
	MemoryMap::Section data = file.map();
	size_t offset = 0;

	auto readu8 = [&]() {
		return data[offset++];
	};

	auto readu32 = [&]() {
		auto& ret = reinterpret_cast<u32&>(data[offset]);
		offset += 4;
		return ret;
	};

	if (readu32() != MAKE_VOX_CHUNK_ID('V', 'O', 'X', ' ')) {
		LB_INFO(VOX, "Failed to load {}, not valid VOX format", path.string());
		return;
	}

	u32 version = readu32();
	bool hasPalette = false;

	while (offset != std::filesystem::file_size(path)) {
		u32 id = readu32();
		u32 chunk_bytes = readu32();
		u32 child_bytes = readu32();

		switch (id) {
			case MAKE_VOX_CHUNK_ID('M', 'A', 'I', 'N'): break;

			case MAKE_VOX_CHUNK_ID('P', 'A', 'C', 'K'): models.reserve(readu32()); break;

			case MAKE_VOX_CHUNK_ID('S', 'I', 'Z', 'E'): {
				u32 w = readu32();
				u32 d = readu32();
				u32 h = readu32();
				Model& model = models.emplace_back(w, h, d);
				shader.setFloat("voxel_size", 0.01f);
			} break;

			case MAKE_VOX_CHUNK_ID('X', 'Y', 'Z', 'I'): {
				u32 voxel_count = readu32();
				Model& model = models.back();

				const uint32_t x_s = 1;
				const uint32_t y_s = model.texture.W;
				const uint32_t z_s = model.texture.W * model.texture.H;

				while (voxel_count--) {
					u8 x = readu8();
					u8 z = readu8();
					u8 y = readu8();
					model.volume.at((x * x_s) + (y * y_s) + (z * z_s)) = readu8();
				}

				model.texture.update(model.volume.data());
			} break;

			case MAKE_VOX_CHUNK_ID('R', 'G', 'B', 'A'): {
				hasPalette = true;

				for (int i = 0; i <= 254; ++i) {
					u8 r = readu8();
					u8 g = readu8();
					u8 b = readu8();
					readu8();

                    palette[i + 1] = col_to_vec(r, g, b); 
                }
				
				u8 r = readu8();
				u8 g = readu8();
				u8 b = readu8();
				readu8();
			} break;

			default: offset += chunk_bytes;
		}
	}

	if (!hasPalette) {
		palette = {
			col_to_vec(0x000000), col_to_vec(0xffffff), col_to_vec(0xccffff), col_to_vec(0x99ffff), col_to_vec(0x66ffff), col_to_vec(0x33ffff), col_to_vec(0x00ffff), col_to_vec(0xffccff), 
			col_to_vec(0xccccff), col_to_vec(0x99ccff), col_to_vec(0x66ccff), col_to_vec(0x33ccff), col_to_vec(0x00ccff), col_to_vec(0xff99ff), col_to_vec(0xcc99ff), col_to_vec(0x9999ff),
			col_to_vec(0x6699ff), col_to_vec(0x3399ff), col_to_vec(0x0099ff), col_to_vec(0xff66ff), col_to_vec(0xcc66ff), col_to_vec(0x9966ff), col_to_vec(0x6666ff), col_to_vec(0x3366ff), 
			col_to_vec(0x0066ff), col_to_vec(0xff33ff), col_to_vec(0xcc33ff), col_to_vec(0x9933ff), col_to_vec(0x6633ff), col_to_vec(0x3333ff), col_to_vec(0x0033ff), col_to_vec(0xff00ff),
			col_to_vec(0xcc00ff), col_to_vec(0x9900ff), col_to_vec(0x6600ff), col_to_vec(0x3300ff), col_to_vec(0x0000ff), col_to_vec(0xffffcc), col_to_vec(0xccffcc), col_to_vec(0x99ffcc), 
			col_to_vec(0x66ffcc), col_to_vec(0x33ffcc), col_to_vec(0x00ffcc), col_to_vec(0xffcccc), col_to_vec(0xcccccc), col_to_vec(0x99cccc), col_to_vec(0x66cccc), col_to_vec(0x33cccc),
			col_to_vec(0x00cccc), col_to_vec(0xff99cc), col_to_vec(0xcc99cc), col_to_vec(0x9999cc), col_to_vec(0x6699cc), col_to_vec(0x3399cc), col_to_vec(0x0099cc), col_to_vec(0xff66cc), 
			col_to_vec(0xcc66cc), col_to_vec(0x9966cc), col_to_vec(0x6666cc), col_to_vec(0x3366cc), col_to_vec(0x0066cc), col_to_vec(0xff33cc), col_to_vec(0xcc33cc), col_to_vec(0x9933cc),
			col_to_vec(0x6633cc), col_to_vec(0x3333cc), col_to_vec(0x0033cc), col_to_vec(0xff00cc), col_to_vec(0xcc00cc), col_to_vec(0x9900cc), col_to_vec(0x6600cc), col_to_vec(0x3300cc),
			col_to_vec(0x0000cc), col_to_vec(0xffff99), col_to_vec(0xccff99), col_to_vec(0x99ff99), col_to_vec(0x66ff99), col_to_vec(0x33ff99), col_to_vec(0x00ff99), col_to_vec(0xffcc99),
			col_to_vec(0xcccc99), col_to_vec(0x99cc99), col_to_vec(0x66cc99), col_to_vec(0x33cc99), col_to_vec(0x00cc99), col_to_vec(0xff9999), col_to_vec(0xcc9999), col_to_vec(0x999999), 
			col_to_vec(0x669999), col_to_vec(0x339999), col_to_vec(0x009999), col_to_vec(0xff6699), col_to_vec(0xcc6699), col_to_vec(0x996699), col_to_vec(0x666699), col_to_vec(0x336699),
			col_to_vec(0x006699), col_to_vec(0xff3399), col_to_vec(0xcc3399), col_to_vec(0x993399), col_to_vec(0x663399), col_to_vec(0x333399), col_to_vec(0x003399), col_to_vec(0xff0099), 
			col_to_vec(0xcc0099), col_to_vec(0x990099), col_to_vec(0x660099), col_to_vec(0x330099), col_to_vec(0x000099), col_to_vec(0xffff66), col_to_vec(0xccff66), col_to_vec(0x99ff66),
			col_to_vec(0x66ff66), col_to_vec(0x33ff66), col_to_vec(0x00ff66), col_to_vec(0xffcc66), col_to_vec(0xcccc66), col_to_vec(0x99cc66), col_to_vec(0x66cc66), col_to_vec(0x33cc66), 
			col_to_vec(0x00cc66), col_to_vec(0xff9966), col_to_vec(0xcc9966), col_to_vec(0x999966), col_to_vec(0x669966), col_to_vec(0x339966), col_to_vec(0x009966), col_to_vec(0xff6666),
			col_to_vec(0xcc6666), col_to_vec(0x996666), col_to_vec(0x666666), col_to_vec(0x336666), col_to_vec(0x006666), col_to_vec(0xff3366), col_to_vec(0xcc3366), col_to_vec(0x993366), 
			col_to_vec(0x663366), col_to_vec(0x333366), col_to_vec(0x003366), col_to_vec(0xff0066), col_to_vec(0xcc0066), col_to_vec(0x990066), col_to_vec(0x660066), col_to_vec(0x330066),
			col_to_vec(0x000066), col_to_vec(0xffff33), col_to_vec(0xccff33), col_to_vec(0x99ff33), col_to_vec(0x66ff33), col_to_vec(0x33ff33), col_to_vec(0x00ff33), col_to_vec(0xffcc33), 
			col_to_vec(0xcccc33), col_to_vec(0x99cc33), col_to_vec(0x66cc33), col_to_vec(0x33cc33), col_to_vec(0x00cc33), col_to_vec(0xff9933), col_to_vec(0xcc9933), col_to_vec(0x999933),
			col_to_vec(0x669933), col_to_vec(0x339933), col_to_vec(0x009933), col_to_vec(0xff6633), col_to_vec(0xcc6633), col_to_vec(0x996633), col_to_vec(0x666633), col_to_vec(0x336633), 
			col_to_vec(0x006633), col_to_vec(0xff3333), col_to_vec(0xcc3333), col_to_vec(0x993333), col_to_vec(0x663333), col_to_vec(0x333333), col_to_vec(0x003333), col_to_vec(0xff0033),
			col_to_vec(0xcc0033), col_to_vec(0x990033), col_to_vec(0x660033), col_to_vec(0x330033), col_to_vec(0x000033), col_to_vec(0xffff00), col_to_vec(0xccff00), col_to_vec(0x99ff00), 
			col_to_vec(0x66ff00), col_to_vec(0x33ff00), col_to_vec(0x00ff00), col_to_vec(0xffcc00), col_to_vec(0xcccc00), col_to_vec(0x99cc00), col_to_vec(0x66cc00), col_to_vec(0x33cc00),
			col_to_vec(0x00cc00), col_to_vec(0xff9900), col_to_vec(0xcc9900), col_to_vec(0x999900), col_to_vec(0x669900), col_to_vec(0x339900), col_to_vec(0x009900), col_to_vec(0xff6600), 
			col_to_vec(0xcc6600), col_to_vec(0x996600), col_to_vec(0x666600), col_to_vec(0x336600), col_to_vec(0x006600), col_to_vec(0xff3300), col_to_vec(0xcc3300), col_to_vec(0x993300),
			col_to_vec(0x663300), col_to_vec(0x333300), col_to_vec(0x003300), col_to_vec(0xff0000), col_to_vec(0xcc0000), col_to_vec(0x990000), col_to_vec(0x660000), col_to_vec(0x330000), 
			col_to_vec(0x0000ee), col_to_vec(0x0000dd), col_to_vec(0x0000bb), col_to_vec(0x0000aa), col_to_vec(0x000088), col_to_vec(0x000077), col_to_vec(0x000055), col_to_vec(0x000044),
			col_to_vec(0x000022), col_to_vec(0x000011), col_to_vec(0x00ee00), col_to_vec(0x00dd00), col_to_vec(0x00bb00), col_to_vec(0x00aa00), col_to_vec(0x008800), col_to_vec(0x007700), 
			col_to_vec(0x005500), col_to_vec(0x004400), col_to_vec(0x002200), col_to_vec(0x001100), col_to_vec(0xee0000), col_to_vec(0xdd0000), col_to_vec(0xbb0000), col_to_vec(0xaa0000),
			col_to_vec(0x880000), col_to_vec(0x770000), col_to_vec(0x550000), col_to_vec(0x440000), col_to_vec(0x220000), col_to_vec(0x110000), col_to_vec(0xeeeeee), col_to_vec(0xdddddd), 
			col_to_vec(0xbbbbbb), col_to_vec(0xaaaaaa), col_to_vec(0x888888), col_to_vec(0x777777), col_to_vec(0x555555), col_to_vec(0x444444), col_to_vec(0x222222), col_to_vec(0x111111)
		};
	}

	glUniform3fv(glGetUniformLocation(shader.ID, "colorPalette"), 256, &(palette[0].x));
}
