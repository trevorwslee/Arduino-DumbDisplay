
const uint8_t *PgmCopyBytes(const uint8_t *bytes, int byteCount, uint8_t *buffer) {
  for (int i = 0; i < byteCount; i++) {
   buffer[i] = pgm_read_byte(bytes + i);
  }
  return buffer;
}
