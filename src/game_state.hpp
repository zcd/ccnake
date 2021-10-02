#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <queue>
#include <unordered_set>

enum direction {
  north,
  south,
  east,
  west,
};

enum tile {
  empty = 0,
  fruit,
  body,
};

struct point {
  const std::size_t i;
  const std::size_t j;
};

class snake {
 public:
  explicit snake(point start) : pending_growth_(0) { body_.push(start); }

  std::optional<point> walk(const point &p);
  void grow(uint32_t amount);
  point head() const;

 private:
  uint32_t pending_growth_;
  std::queue<point> body_;
};

typedef std::function<std::size_t(std::size_t, std::size_t)> range_rng;
typedef std::vector<point> tile_updates;

class playing_field {
 public:
  playing_field(std::size_t width, std::size_t height);

  tile get(const point &p) const;
  void put(const point &p, tile t);
  bool is_legal(const point &p) const;
  bool is_full() const;

  std::optional<point> find_empty(range_rng rng) const;

 private:
  point to_coord(std::size_t idx) const;
  std::size_t from_coord(const point &p) const;

  const std::size_t width_;
  const std::size_t height_;
  std::vector<tile> data_;
  std::unordered_set<std::size_t> free_spots_;
};

class game_state {
 public:
  static std::unique_ptr<game_state> create(
      std::size_t height, std::size_t width, point snake_origin,
      std::vector<point> fruits, uint32_t growth_rate, range_rng rng);

  tile_updates step(direction d);
  tile get_tile(point p) const { return field_->get(p); }

 private:
  game_state(std::unique_ptr<playing_field> field, std::unique_ptr<snake> s,
             uint32_t growth_rate, range_rng rng)
      : field_(std::move(field)),
        snake_(std::move(s)),
        growth_rate_(growth_rate),
        rng_(rng) {}

  const std::unique_ptr<playing_field> field_;
  const std::unique_ptr<snake> snake_;
  const uint32_t growth_rate_;
  const range_rng rng_;
};

#endif  // GAME_STATE_H
