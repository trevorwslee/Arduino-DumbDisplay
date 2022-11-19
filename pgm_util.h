
const uint8_t *PgmCopyBytes(const uint8_t *bytes, int byteCount, uint8_t *buffer) {
  for (int i = 0; i < byteCount; i++) {
    buffer[i] = pgm_read_byte(bytes + i);
  }
  return buffer;
}

const uint16_t *PgmCopyWords(const uint16_t *words, int wordCount, uint16_t *buffer) {
  for (int i = 0; i < wordCount; i++) {
    buffer[i] = pgm_read_word(words + i);
  }
  return buffer;
}

