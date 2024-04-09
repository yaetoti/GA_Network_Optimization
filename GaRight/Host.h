#pragma once

struct TopologyUnit {
  TopologyUnit(float x, float y)
  : m_x(x)
  , m_y(y) {
  }

  virtual ~TopologyUnit() = default;

  float GetX() const {
    return m_x;
  }

  float GetY() const {
    return m_y;
  }

protected:
  float m_x;
  float m_y;
};

struct Switch final : TopologyUnit {
  Switch()
    : TopologyUnit(0.0f, 0.0f)
    , m_bandwidth(0) {
  }

  Switch(float x, float y, size_t bandwidth)
  : TopologyUnit(x, y)
  , m_bandwidth(bandwidth) {
  }

  size_t GetBandwidth() const {
    return m_bandwidth;
  }

private:
  size_t m_bandwidth;
};

struct Host final : TopologyUnit {
  Host()
  : TopologyUnit(0.0f, 0.0f)
  , m_output(0) {
  }

  Host(float x, float y, size_t output)
  : TopologyUnit(x, y)
  , m_output(output) {
  }

  size_t GetOutput() const {
    return m_output;
  }

private:
  size_t m_output;
};
