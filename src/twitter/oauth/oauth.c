#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

#include "oauth.h"
#include "encode.h"

static void oauth_sign_array2_process (int *argcp, char***argvp,
  char **postargs,
  OAuthMethod method,
  const char *http_method, //< HTTP request method
  const char *c_key, //< consumer key - posted plain text
  const char *c_secret, //< consumer secret - used as 1st part of secret-key
  const char *t_key, //< token key - posted plain text in URL
  const char *t_secret //< token secret - used as 2st part of secret-key
);

/**
 * base64 encode digest, free it and return a URL parameter
 * with the oauth_body_hash
 */
char *oauth_body_hash_encode(size_t len, unsigned char *digest) {
  char *sign=encode_base64(len,digest);
  char *sig_url = (char*)malloc(17+strlen(sign));
  sprintf(sig_url,"oauth_body_hash=%s", sign);
  free(sign);
  free(digest);
  return sig_url;
}

char *oauth_sign_url2 (const char *url, char **postargs,
  OAuthMethod method,
  const char *http_method, //< HTTP request method
  const char *c_key, //< consumer key - posted plain text
  const char *c_secret, //< consumer secret - used as 1st part of secret-key
  const char *t_key, //< token key - posted plain text in URL
  const char *t_secret //< token secret - used as 2st part of secret-key
  ) {
  int  argc;
  char **argv = NULL;
  char *rv;

  if (postargs)
    argc = oauth_split_post_parameters(url, &argv, 0);
  else
    argc = oauth_split_url_parameters(url, &argv);

  rv=oauth_sign_array2(&argc, &argv, postargs,
    method, http_method,
    c_key, c_secret, t_key, t_secret);

  free_array(&argc, &argv);
  return(rv);
}

static void oauth_sign_array2_process (int *argcp, char***argvp,
  char **postargs,
  OAuthMethod method,
  const char *http_method, //< HTTP request method
  const char *c_key, //< consumer key - posted plain text
  const char *c_secret, //< consumer secret - used as 1st part of secret-key
  const char *t_key, //< token key - posted plain text in URL
  const char *t_secret //< token secret - used as 2st part of secret-key
  ) {
  char oarg[1024];
  char *query;
  char *okey, *odat, *sign;
  char *http_request_method;

  if (!http_method) {
    http_request_method = strdup(postargs?"POST":"GET");
  } else {
    int i;
    http_request_method = strdup(http_method);
    for (i=0;i<strlen(http_request_method);i++)
      http_request_method[i]=toupper(http_request_method[i]);
  }

  // add required OAuth protocol parameters
  oauth_add_protocol(argcp, argvp, method, c_key, t_key);

  // sort parameters
  qsort(&(*argvp)[1], (*argcp)-1, sizeof(char *), oauth_cmpstringp);

  // serialize URL - base-url
  query= serialize_url_parameters(*argcp, *argvp);

  // generate signature
  okey = catenc(2, c_secret, t_secret);
  odat = catenc(3, http_request_method, (*argvp)[0], query); // base-string
  free(http_request_method);
#ifdef DEBUG_OAUTH
  fprintf (stderr, "\nliboauth: data to sign='%s'\n\n", odat);
  fprintf (stderr, "\nliboauth: key='%s'\n\n", okey);
#endif
  switch(method) {
    case OA_RSA:
      sign = oauth_sign_rsa_sha1(odat,okey); // XXX okey needs to be RSA key!
      break;
    case OA_PLAINTEXT:
      sign = oauth_sign_plaintext(odat,okey);
      break;
    default:
      sign = oauth_sign_hmac_sha1(odat,okey);
  }
#ifdef WIPE_MEMORY
  memset(okey,0, strlen(okey));
  memset(odat,0, strlen(odat));
#endif
  free(odat);
  free(okey);

  // append signature to query args.
  snprintf(oarg, 1024, "oauth_signature=%s",sign);
  add_param_to_array(argcp, argvp, oarg);
  free(sign);
  if(query) free(query);
}

/**
 * splits the given url into a parameter array.
 * (see \ref oauth_serialize_url and \ref oauth_serialize_url_parameters for the reverse)
 *
 * NOTE: Request-parameters-values may include an ampersand character.
 * However if unescaped this function will use them as parameter delimiter.
 * If you need to make such a request, this function since version 0.3.5 allows
 * to use the ASCII SOH (0x01) character as alias for '&' (0x26).
 * (the motivation is convenience: SOH is /untypeable/ and much more
 * unlikely to appear than '&' - If you plan to sign fancy URLs you
 * should not split a query-string, but rather provide the parameter array
 * directly to \ref oauth_serialize_url)
 *
 * @param url the url or query-string to parse.
 * @param argv pointer to a (char *) array where the results are stored.
 *  The array is re-allocated to match the number of parameters and each
 *  parameter-string is allocated with strdup. - The memory needs to be freed
 *  by the caller.
 * @param qesc use query parameter escape (vs post-param-escape) - if set
 *        to 1 all '+' are treated as spaces ' '
 *
 * @return number of parameter(s) in array.
 */
int oauth_split_post_parameters(const char *url, char ***argv, short qesc) {
  int argc=0;
  char *token, *tmp, *t1;
  if (!argv) return 0;
  if (!url) return 0;
  t1=strdup(url);

  // '+' represents a space, in a URL query string
  while ((qesc&1) && (tmp=strchr(t1,'+'))) *tmp=' ';

  tmp=t1;
  while((token=strtok(tmp,"&?"))) {
    if(!strncasecmp("oauth_signature=",token,16)) continue;
    (*argv)=(char**) realloc(*argv,sizeof(char*)*(argc+1));
    while (!(qesc&2) && (tmp=strchr(token,'\001'))) *tmp='&';
    (*argv)[argc]=url_unescape(token, NULL);
    if (argc==0 && strstr(token, ":/")) {
      // HTTP does not allow empty absolute paths, so the URL
      // 'http://example.com' is equivalent to 'http://example.com/' and should
      // be treated as such for the purposes of OAuth signing (rfc2616, section 3.2.1)
      // see http://groups.google.com/group/oauth/browse_thread/thread/c44b6f061bfd98c?hl=en
      char *slash=strstr(token, ":/");
      while (slash && *(++slash) == '/')  ; // skip slashes eg /xxx:[\/]*/
#if 0
      // skip possibly unescaped slashes in the userinfo - they're not allowed by RFC2396 but have been seen.
      // the hostname/IP may only contain alphanumeric characters - so we're safe there.
      if (slash && strchr(slash,'@')) slash=strchr(slash,'@');
#endif
      if (slash && !strchr(slash,'/')) {
#ifdef DEBUG_OAUTH
        fprintf(stderr, "\nliboauth: added trailing slash to URL: '%s'\n\n", token);
#endif
        free((*argv)[argc]);
        (*argv)[argc]= (char*) malloc(sizeof(char)*(2+strlen(token)));
        strcpy((*argv)[argc],token);
        strcat((*argv)[argc],"/");
      }
    }
    if (argc==0 && (tmp=strstr((*argv)[argc],":80/"))) {
        memmove(tmp, tmp+3, strlen(tmp+2));
    }
    tmp=NULL;
    argc++;
  }

  free(t1);
  return argc;
}

int oauth_split_url_parameters(const char *url, char ***argv) {
  return oauth_split_post_parameters(url, argv, 1);
}

char *oauth_sign_array2 (int *argcp, char***argvp,
  char **postargs,
  OAuthMethod method,
  const char *http_method, //< HTTP request method
  const char *c_key, //< consumer key - posted plain text
  const char *c_secret, //< consumer secret - used as 1st part of secret-key
  const char *t_key, //< token key - posted plain text in URL
  const char *t_secret //< token secret - used as 2st part of secret-key
  ) {

  char *result;
  oauth_sign_array2_process(argcp, argvp, postargs, method, http_method, c_key, c_secret, t_key, t_secret);

  // build URL params
  result = serialize_url(*argcp, (postargs?1:0), *argvp);

  if(postargs) {
    *postargs = result;
    result = strdup((*argvp)[0]);
  }

  return result;
}

/**
 * returns plaintext signature for the given key.
 *
 * the returned string needs to be freed by the caller
 *
 * @param m message to be signed
 * @param k key used for signing
 * @return signature string
 */
char *oauth_sign_plaintext (const char *m, const char *k) {
  return(url_escape(k));
}

/**
 * generate a random string between 15 and 32 chars length
 * and return a pointer to it. The value needs to be freed by the
 * caller
 *
 * @return zero terminated random string.
 */
#if !defined HAVE_OPENSSL_HMAC_H && !defined USE_NSS
/* pre liboauth-0.7.2 and possible future versions that don't use OpenSSL or NSS */
char *oauth_gen_nonce() {
  char *nc;
  static int rndinit = 1;
  const char *chars = "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789_";
  unsigned int max = strlen( chars );
  int i, len;

  if(rndinit) {srand(time(NULL)
#ifndef WIN32 // quick windows check.
    * getpid()
#endif
  ); rndinit=0;} // seed random number generator - FIXME: we can do better ;)

  len=15+floor(rand()*16.0/(double)RAND_MAX);
  nc = (char*) malloc((len+1)*sizeof(char));
  for(i=0;i<len; i++) {
    nc[i] = chars[ rand() % max ];
  }
  nc[i]='\0';
  return (nc);
}
#else // OpenSSL or NSS random number generator
#ifdef USE_NSS
  void oauth_init_nss(); //decladed in hash.c
#  include "pk11pub.h"
#  define MY_RAND PK11_GenerateRandom
#  define MY_SRAND  oauth_init_nss();
#else
#  ifdef _GNU_SOURCE
/* Note: the OpenSSL/GPL exemption stated
 * verbosely in hash.c applies to this code as well. */
#  endif
#  include <openssl/rand.h>
#  define MY_RAND RAND_bytes
#  define MY_SRAND ;
#endif
char *oauth_gen_nonce() {
  char *nc;
  unsigned char buf;
  const char *chars = "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789_";
  unsigned int max = strlen(chars);
  int i, len;

  MY_SRAND
  MY_RAND(&buf, 1);
  len=15+(((short)buf)&0x0f);
  nc = (char*) malloc((len+1)*sizeof(char));
  for(i=0;i<len; i++) {
    MY_RAND(&buf, 1);
    nc[i] = chars[ ((short)buf) % max ];
  }
  nc[i]='\0';
  return (nc);
}
#endif

/**
 *
 */
void oauth_add_protocol(int *argcp, char ***argvp,
  OAuthMethod method,
  const char *c_key, //< consumer key - posted plain text
  const char *t_key //< token key - posted plain text in URL
 ){
  char oarg[1024];

  // add OAuth specific arguments
  if (!param_exists(*argvp,*argcp,"oauth_nonce")) {
    char *tmp;
    snprintf(oarg, 1024, "oauth_nonce=%s", (tmp=oauth_gen_nonce()));
    add_param_to_array(argcp, argvp, oarg);
    free(tmp);
  }

  if (!param_exists(*argvp,*argcp,"oauth_timestamp")) {
    snprintf(oarg, 1024, "oauth_timestamp=%li", (long int) time(NULL));
    add_param_to_array(argcp, argvp, oarg);
  }

  if (t_key) {
    snprintf(oarg, 1024, "oauth_token=%s", t_key);
    add_param_to_array(argcp, argvp, oarg);
  }

  snprintf(oarg, 1024, "oauth_consumer_key=%s", c_key);
  add_param_to_array(argcp, argvp, oarg);

  snprintf(oarg, 1024, "oauth_signature_method=%s",
      method==0?"HMAC-SHA1":method==1?"RSA-SHA1":"PLAINTEXT");
  add_param_to_array(argcp, argvp, oarg);

  if (!param_exists(*argvp,*argcp,"oauth_version")) {
    snprintf(oarg, 1024, "oauth_version=1.0");
    add_param_to_array(argcp, argvp, oarg);
  }

#if 0 // oauth_version 1.0 Rev A
  if (!param_exists(argv,argc,"oauth_callback")) {
    snprintf(oarg, 1024, "oauth_callback=oob");
    add_param_to_array(argcp, argvp, oarg);
  }
#endif

}

/**
 * string compare function for oauth parameters.
 *
 * used with qsort. needed to normalize request parameters.
 * see http://oauth.net/core/1.0/#anchor14
 */
int oauth_cmpstringp(const void *p1, const void *p2) {
  char *v1,*v2;
  char *t1,*t2;
  int rv;
  // TODO: this is not fast - we should escape the
  // array elements (once) before sorting.
  v1=url_escape(* (char * const *)p1);
  v2=url_escape(* (char * const *)p2);

  // '=' signs are not "%3D" !
  if ((t1=strstr(v1,"%3D"))) {
    t1[0]='\0'; t1[1]='='; t1[2]='=';
  }
  if ((t2=strstr(v2,"%3D"))) {
    t2[0]='\0'; t2[1]='='; t2[2]='=';
  }

  // compare parameter names
  rv=strcmp(v1,v2);
  if (rv!=0) {
    if (v1) free(v1);
    if (v2) free(v2);
    return rv;
  }

  // if parameter names are equal, sort by value.
  if (t1) t1[0]='=';
  if (t2) t2[0]='=';
  if (t1 && t2)        rv=strcmp(t1,t2);
  else if (!t1 && !t2) rv=0;
  else if (!t1)        rv=-1;
  else                 rv=1;

  if (v1) free(v1);
  if (v2) free(v2);
  return rv;
}

/**
 * search array for parameter key.
 * @param argv length of array to search
 * @param argc parameter array to search
 * @param key key of parameter to check.
 *
 * @return FALSE (0) if array does not contain a parameter with given key, TRUE (1) otherwise.
 */
int param_exists(char **argv, int argc, char *key) {
  int i;
  size_t l= strlen(key);
  for (i=0;i<argc;i++)
    if (strlen(argv[i])>l && !strncmp(argv[i],key,l) && argv[i][l] == '=') return 1;
  return 0;
}

/**
 * add query parameter to array
 *
 * @param argcp pointer to array length int
 * @param argvp pointer to array values
 * @param addparam parameter to add (eg. "foo=bar")
 */
void add_param_to_array(int *argcp, char ***argvp, const char *addparam) {
  (*argvp)=(char**) realloc(*argvp,sizeof(char*)*((*argcp)+1));
  (*argvp)[(*argcp)++]= (char*) strdup(addparam);
}

/**
 * encode strings and concatenate with '&' separator.
 * The number of strings to be concatenated must be
 * given as first argument.
 * all arguments thereafter must be of type (char *)
 *
 * @param len the number of arguments to follow this parameter
 * @param ... string to escape and added (may be NULL)
 *
 * @return pointer to memory holding the concatenated
 * strings - needs to be free(d) by the caller. or NULL
 * in case we ran out of memory.
 */
char *catenc(int len, ...) {
  va_list va;
  int i;
  char *rv = (char*) malloc(sizeof(char));
  *rv='\0';
  va_start(va, len);
  for(i=0;i<len;i++) {
    char *arg = va_arg(va, char *);
    char *enc;
    int len;
    enc = url_escape(arg);
    if(!enc) break;
    len = strlen(enc) + 1 + ((i>0)?1:0);
    if(rv) len+=strlen(rv);
    rv=(char*) realloc(rv,len*sizeof(char));

    if(i>0) strcat(rv, "&");
    strcat(rv, enc);
    free(enc);
  }
  va_end(va);
  return(rv);
}

/**
 * free array args
 *
 * @param argcp pointer to array length int
 * @param argvp pointer to array values to be free()d
 */
void free_array(int *argcp, char ***argvp) {
  int i;
  for (i=0;i<(*argcp);i++) {
    free((*argvp)[i]);
  }
  if(*argvp) free(*argvp);
}

const char* get_param(char** argv, int argc, const char* key)
{
    int i, c;

    c = strlen(key);
    for (i = 0; i < argc; i++) {
        //printf("\"%s\" argv[%d].strlen = %d, key.strlen = %d, eq=\"%c\"\n", argv[i], i, strlen(argv[i]), c, argv[i][c]);
        if (!strncmp(argv[i], key, c) && strlen(argv[i]) > c && argv[i][c] == '=')
            return &argv[i][c + 1];
    }

    return NULL;
}
