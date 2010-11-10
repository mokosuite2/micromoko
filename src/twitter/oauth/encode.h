
#ifndef __MICROMOKO_TWITTER_ENCODE_H
#define __MICROMOKO_TWITTER_ENCODE_H

char *encode_base64(int size, const unsigned char *src);
int decode_base64(unsigned char *dest, const char *src);

char b64_encode(unsigned char u);
unsigned char b64_decode(char c);
int b64_is_base64(char c);

char *url_escape(const char *string);
char *url_unescape(const char *string, size_t *olen);

char *serialize_url (int argc, int start, char **argv);
char *serialize_url_parameters (int argc, char **argv);

#endif  /* __MICROMOKO_TWITTER_ENCODE_H */
