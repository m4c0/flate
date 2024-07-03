module flate;
import :dedup;

static constexpr auto compress(yoyo::writer &w, const uint8_t *data,
                               unsigned size) {
  auto syms = dedup_all(data, size);
  return mno::req<void>{};
}

mno::req<void> flate::compress(yoyo::writer &w, const void *data,
                               unsigned size) {
  return ::compress(w, static_cast<const uint8_t *>(data), size);
}
