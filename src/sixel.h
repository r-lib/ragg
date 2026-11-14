#pragma once

#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <limits>
#include <numeric>
#include <cassert>
#include <iostream>

// Hardcoded stride for 24bpp RGB since Sixel has no alpha support
// Using pixfmt_type_24 format exclusively for this device
constexpr size_t bitmap_stride = 3;

namespace sixeldev_impl {

	struct color {
		explicit color() : rgb({ 255, 255, 255 }) {}
		explicit color(std::uint8_t r, std::uint8_t g, std::uint8_t b) : rgb({ r, g, b }) {}
		std::uint64_t dist_sq(const color& rhs) const {
			std::int64_t dr = (std::int64_t)rhs.rgb[0] - (std::int64_t)rgb[0];
			std::int64_t dg = (std::int64_t)rhs.rgb[1] - (std::int64_t)rgb[1];
			std::int64_t db = (std::int64_t)rhs.rgb[2] - (std::int64_t)rgb[2];
			return dr * dr + dg * dg + db * db;
		}
		std::array<std::uint8_t, 3> rgb;
	};

	struct color_node {
		size_t color_index;
		std::unique_ptr<color_node> lhs;
		std::unique_ptr<color_node> rhs;
	};

	struct kdtree_findres {
		const color_node *node { nullptr };
		std::uint64_t dist_sq { std::numeric_limits<std::uint64_t>::max() };
	};

	class kdtree {
		public:
			kdtree(const std::vector<color>& src) : palette(src) {
				std::vector<size_t> indices(src.size());
				std::iota(indices.begin(), indices.end(), 0);
				data = build_recursively(std::move(indices), 0);
			}
			size_t search(color c) {
				kdtree_findres res;
				search_recursively(data.get(), c, 0, res);
				return res.node->color_index;
			}
		private:
			std::unique_ptr<color_node> build_recursively(std::vector<size_t>&& src, size_t depth) {
				if (src.empty()) { return nullptr; }
				size_t axis = depth % 3, median = src.size() / 2;
				std::nth_element(src.begin(), src.begin() + median, src.end(), [this, axis](const auto& lhs, const auto& rhs){
					return palette[lhs].rgb[axis] < palette[rhs].rgb[axis];
				});

				auto node = std::make_unique<color_node>();
				node->color_index = src[median];

				std::vector<size_t> rhs(src.begin() + median + 1, src.end());
				src.resize(median);
				node->lhs = build_recursively(std::move(src), depth + 1);
				node->rhs = build_recursively(std::move(rhs), depth + 1);
				return node;
			}
			void search_recursively(const color_node *node, const color& c, int depth, kdtree_findres& res) {
				if (!node) { return; }

				auto dist_sq = c.dist_sq(palette[node->color_index]);
				if (dist_sq < res.dist_sq) {
					res.dist_sq = dist_sq;
					res.node = node;
				}
				int axis = depth % 3;
				std::int64_t dist_axis = (std::int64_t)c.rgb[axis] - (std::int64_t)palette[node->color_index].rgb[axis];
				const color_node *near_node { nullptr }, *far_node { nullptr };
				if (dist_axis < 0) {
					near_node = node->lhs.get();
					far_node = node->rhs.get();
				}
				else {
					near_node = node->rhs.get();
					far_node = node->lhs.get();
				}
				search_recursively(near_node, c, depth + 1, res);
				if (dist_axis * dist_axis <= res.dist_sq) {
					search_recursively(far_node, c, depth + 1, res);
				}
			}
			std::unique_ptr<color_node> data;
			std::vector<color> palette;
	};

	struct color_cluster {
		explicit color_cluster() : count(0), rgb({ 0, 0, 0 }) {}
		explicit color_cluster(std::uint8_t r, std::uint8_t g, std::uint8_t b) : count(1), rgb({ r, g, b }) {}
		std::uint32_t count;
		std::array<std::uint8_t, 3> rgb;
	};

	struct color_box {
		color_box() : pixel_count(1), pixel_range({ 0, 0, 0 }) {}
		color_box(const std::uint8_t *data, size_t count) : pixel_count(count) {
			std::unordered_map<std::uint32_t, size_t> color_index;
			std::uint8_t min_r = 255, max_r = 0;
			std::uint8_t min_g = 255, max_g = 0;
			std::uint8_t min_b = 255, max_b = 0;

			auto pack_pixel = [](std::uint32_t r, std::uint32_t g, std::uint32_t b) {
				return (r << 24) | (g << 16) | (b << 8) | 0xFF;
			};
			size_t pixel_size = count * bitmap_stride;
			for (size_t i = 0; i < pixel_size; i += bitmap_stride) {
				std::uint32_t r = data[i], g = data[i + 1], b = data[i + 2];
				std::uint32_t color = pack_pixel(r, g, b);
				auto [itr, inserted] = color_index.try_emplace(color, colors.size());
				if (inserted) {
					color_cluster cluster(r, g, b);
					if (r < min_r) { min_r = r; } if (r > max_r) { max_r = r; }
					if (g < min_g) { min_g = g; } if (g > max_g) { max_g = g; }
					if (b < min_b) { min_b = b; } if (b > max_b) { max_b = b; }
					colors.emplace_back(cluster);
				}
				else {
					colors[itr->second].count += 1;
				}
			}
			pixel_range[0] = max_r - min_r;
			pixel_range[1] = max_g - min_g;
			pixel_range[2] = max_b - min_b;
		}
		color_box(std::vector<color_cluster>&& data, size_t count, const std::array<std::uint8_t, 3>& range)
			: colors(std::move(data)), pixel_count(count), pixel_range(range) { }
		std::pair<color_box, color_box> split() {
			if (colors.size() <= 1) { return { color_box(), *this }; }

			size_t channel_use { 0 };
			if (pixel_range[0] >= pixel_range[1] && pixel_range[0] >= pixel_range[2]) {
				channel_use = 0;
			}
			else if (pixel_range[1] >= pixel_range[2]) {
				channel_use = 1;
			}
			else {
				channel_use = 2;
			}
			std::sort(colors.begin(), colors.end(), [channel_use](const color_cluster& lhs, const color_cluster& rhs){
					return lhs.rgb[channel_use] < rhs.rgb[channel_use];
					});

			size_t half_pixels = pixel_count / 2, sum = 0, split_index = 0, colors_size = colors.size();
			for (size_t i = 0; i < colors_size; ++i) {
				sum += colors[i].count;
				if (sum >= half_pixels) {
					split_index = i + 1;
					break;
				}
			}
			if (split_index >= colors.size()) {
				split_index = colors.size() - 1;
				sum = pixel_count - colors.back().count;
			}

			std::vector<color_cluster> colors1(colors.begin() + split_index, colors.end());
			colors.resize(split_index);

			auto pixel_range1 = pixel_range;
			pixel_range1[channel_use] = colors1.back().rgb[channel_use] - colors1.front().rgb[channel_use];
			pixel_range[channel_use] = colors.back().rgb[channel_use] - colors.front().rgb[channel_use];

			return {
				color_box(std::move(colors), sum, pixel_range),
					color_box(std::move(colors1), pixel_count - sum, pixel_range1)
			};
		}
		color get_color() const {
			if (colors.empty()) { return color(); }

			double r = 0, g = 0, b = 0;
			for (const auto& c : colors) {
				r += 1.0 * c.rgb[0] * c.count / pixel_count;
				g += 1.0 * c.rgb[1] * c.count / pixel_count;
				b += 1.0 * c.rgb[2] * c.count / pixel_count;
			}
			return color(std::round(r), std::round(g), std::round(b));
		}
		size_t get_color_count() const { return colors.size(); };
		bool operator<(const color_box& rhs) const {
			return pixel_count < rhs.pixel_count;
		}

		std::vector<color_cluster> colors;
		size_t pixel_count;
		std::array<std::uint8_t, 3> pixel_range;
	};

	class rle_writer {
		public:
			rle_writer(std::stringstream& ss) : dest(ss) {}
			void write(const std::uint8_t *data, size_t size) {
				while (data[size - 1] == 0) { --size; }
				size_t rep { 1 };
				std::uint8_t last { *data };
				auto flush = [&]() {
					if (rep < 4) {
						char ch = dict[last];
						for (size_t j = 0; j < rep; ++j) { dest << ch; }
					}
					else {
						dest << "!" << rep << dict[last];
					}
				};
				for (size_t i = 1; i < size; ++i) {
					std::uint8_t cur = data[i];
					if (cur == last) { ++rep; }
					else {
						flush();
						rep = 1;
						last = cur;
					}
				}
				flush();
			}
		private:
			static constexpr const char *dict = "?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
			std::stringstream& dest;
	};
	class encoder {
		public:
			encoder(size_t w, size_t h) : width(w), height(h) {
				data.clear();
				data << "\x1bP0;1;0q\"1;1;" << width << ";" << height;
			}
			void write_palette(const std::vector<color>& palette) {
				palette_size = palette.size();
				auto clamp = [](std::uint8_t c) { return (std::uint32_t)(c * 100 / 255); };
				for (size_t i = 0; i < palette_size; ++i) {
					const auto& c = palette[i].rgb;
					data << "#" << i << ";2;" << clamp(c[0]) << ";" << clamp(c[1]) << ";" << clamp(c[2]);
				}
			}
			void write_data(std::vector<size_t>& indices) {
				rle_writer writer(data);
				std::bitset<256> used = { 0 };
				std::vector<std::uint8_t> bits(256 * width, 0);
				auto encode_line = [&](size_t y, size_t dy) {
					for (size_t i = 0; i < dy; ++i) {
						size_t *row = indices.data() + width * (y + i);
						for (size_t j = 0; j < width; ++j) {
							size_t color = row[j];
							used.set(color);
							std::uint8_t *c = bits.data() + width * color;
							c[j] |= 1 << i;
						}
					}
					for (size_t i = 0; i < 256; ++i) {
						if (!used.test(i)) { continue; }
						data << "$#" << i;
						writer.write(bits.data() + width * i, width);
					}
					bits.assign(bits.size(), 0);
					used.reset();
				};
				size_t y = 0, y_lim = height > 6 ? height - 6 : 0;
				for (; y < y_lim; y += 6) {
					encode_line(y, 6);
					data << "-";
				}
				encode_line(y, height - y);
			}
			std::string get() {
				data << "\x1b\\";
				return data.str();
			}
		private:
			size_t width;
			size_t height;
			std::stringstream data;
			size_t palette_size;
	};
	class quantizer {
		public:
			quantizer(const std::uint8_t *data, size_t width, size_t height) {
				auto pixel_size = width * height;

				palette = median_cut(data, pixel_size, 256);
				std::sort(palette.begin(), palette.end(), [](const color& lhs, const color& rhs) {
                    std::uint32_t lhs_key = (lhs.rgb[0] << 16) | (lhs.rgb[1] << 8) | lhs.rgb[2];
                    std::uint32_t rhs_key = (rhs.rgb[0] << 16) | (rhs.rgb[1] << 8) | rhs.rgb[2];
                    return lhs_key > rhs_key;
				});
				auto itr = std::unique(palette.begin(), palette.end(), [](const color& lhs, const color& rhs){
                    return lhs.rgb[0] == rhs.rgb[0] && lhs.rgb[1] == rhs.rgb[1] && lhs.rgb[2] == rhs.rgb[2];
                });
				palette.erase(itr, palette.end());
				kdtree tree(palette);

				quant.reserve(pixel_size);
				for (size_t i = 0; i < pixel_size; ++i) {
					size_t data_begin = i * bitmap_stride;
					std::uint32_t r = data[data_begin], g = data[data_begin + 1], b = data[data_begin + 2];
					color c(r, g, b);
					quant.push_back(tree.search(c));
				}
			}
			const std::vector<color>& get_palette() { return palette; }
			std::vector<size_t>& get_quant() { return quant; }
		private:
			std::vector<color> median_cut(const std::uint8_t *data, size_t size, size_t max_colors) {
				color_box initial_box(data, size);

				std::priority_queue<color_box> pq;
				pq.push(std::move(initial_box));

				std::vector<color> result;
				result.reserve(max_colors);
				for (size_t i = 0; i < max_colors; ++i) {
					const auto& top = pq.top();
					if (top.get_color_count() <= 1) {
						result.push_back(top.get_color());
						pq.pop();
					}
					if (pq.empty()) { break; }
					auto top_box = pq.top(); pq.pop();
					auto [box1, box2] = top_box.split();
					pq.push(std::move(box1));
					pq.push(std::move(box2));
				}

				while (!pq.empty()) {
					result.push_back(pq.top().get_color());
					pq.pop();
				}
				if (result.size() > max_colors) { result.resize(max_colors); }
				return result;
			}
			std::vector<color> palette;
			std::vector<size_t> quant;
	};

	inline std::string as_sixel(const std::uint8_t *data, size_t width, size_t height) {
		quantizer quant(data, width, height);
		encoder enc(width, height);
		enc.write_palette(quant.get_palette());
		enc.write_data(quant.get_quant());
		return enc.get();
	}
}

