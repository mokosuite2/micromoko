#ifndef MICROMOKO_TWITTER_OAUTH_H
#define MICROMOKO_TWITTER_OAUTH_H

#include <stddef.h>

/** \enum OAuthMethod
 * signature method to used for signing the request.
 */
typedef enum {
    OA_HMAC=0, ///< use HMAC-SHA1 request signing method
    OA_RSA, ///< use RSA signature
    OA_PLAINTEXT ///< use plain text signature (for testing only)
} OAuthMethod;

// hashing functions
char *oauth_sign_hmac_sha1 (const char *m, const char *k);
char *oauth_sign_rsa_sha1 (const char *m, const char *k);
char *oauth_sign_plaintext (const char *m, const char *k);  // from oauth.c

char *oauth_body_hash_encode(size_t len, unsigned char *digest);

char *oauth_sign_url2 (const char *url, char **postargs,
  OAuthMethod method,
  const char *http_method, //< HTTP request method
  const char *c_key, //< consumer key - posted plain text
  const char *c_secret, //< consumer secret - used as 1st part of secret-key
  const char *t_key, //< token key - posted plain text in URL
  const char *t_secret //< token secret - used as 2st part of secret-key
);

int oauth_split_post_parameters(const char *url, char ***argv, short qesc);
int oauth_split_url_parameters(const char *url, char ***argv);

char *oauth_sign_array2 (int *argcp, char***argvp,
  char **postargs,
  OAuthMethod method,
  const char *http_method, //< HTTP request method
  const char *c_key, //< consumer key - posted plain text
  const char *c_secret, //< consumer secret - used as 1st part of secret-key
  const char *t_key, //< token key - posted plain text in URL
  const char *t_secret //< token secret - used as 2st part of secret-key
);

void oauth_add_protocol(int *argcp, char ***argvp,
  OAuthMethod method,
  const char *c_key, //< consumer key - posted plain text
  const char *t_key //< token key - posted plain text in URL
);

int oauth_cmpstringp(const void *p1, const void *p2);

int param_exists(char **argv, int argc, char *key);
void add_param_to_array(int *argcp, char ***argvp, const char *addparam);

char *catenc(int len, ...);
void free_array(int *argcp, char ***argvp);

const char* get_param(char** argv, int argc, const char* key);

#endif /* MICROMOKO_TWITTER_OAUTH_H */
