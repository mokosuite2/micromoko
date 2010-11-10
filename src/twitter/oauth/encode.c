#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h> // isxdigit

static char *serialize_url_sep (int argc, int start, char **argv, char *sep, int mod);

/**
 * Base64 encode one byte
 */
char b64_encode(unsigned char u) {
  if(u < 26)  return 'A'+u;
  if(u < 52)  return 'a'+(u-26);
  if(u < 62)  return '0'+(u-52);
  if(u == 62) return '+';
  return '/';
}

/**
 * Decode a single base64 character.
 */
unsigned char b64_decode(char c) {
  if(c >= 'A' && c <= 'Z') return(c - 'A');
  if(c >= 'a' && c <= 'z') return(c - 'a' + 26);
  if(c >= '0' && c <= '9') return(c - '0' + 52);
  if(c == '+')             return 62;
  return 63;
}

/**
 * Return TRUE if 'c' is a valid base64 character, otherwise FALSE
 */
int b64_is_base64(char c) {
  if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
     (c >= '0' && c <= '9') || (c == '+')             ||
     (c == '/')             || (c == '=')) {
    return 1;
  }
  return 0;
}

/**
 * Base64 encode and return size data in 'src'. The caller must free the
 * returned string.
 *
 * @param size The size of the data in src
 * @param src The data to be base64 encode
 * @return encoded string otherwise NULL
 */
char *encode_base64(int size, const unsigned char *src) {
  int i;
  char *out, *p;

  if(!src) return NULL;
  if(!size) size= strlen((char *)src);
  out= (char*) calloc(sizeof(char), size*4/3+4);
  p= out;

  for(i=0; i<size; i+=3) {
    unsigned char b1=0, b2=0, b3=0, b4=0, b5=0, b6=0, b7=0;
    b1= src[i];
    if(i+1<size) b2= src[i+1];
    if(i+2<size) b3= src[i+2];

    b4= b1>>2;
    b5= ((b1&0x3)<<4)|(b2>>4);
    b6= ((b2&0xf)<<2)|(b3>>6);
    b7= b3&0x3f;

    *p++= b64_encode(b4);
    *p++= b64_encode(b5);

    if(i+1<size) *p++= b64_encode(b6);
    else *p++= '=';

    if(i+2<size) *p++= b64_encode(b7);
    else *p++= '=';
  }
  return out;
}

/**
 * Decode the base64 encoded string 'src' into the memory pointed to by
 * 'dest'.
 *
 * @param dest Pointer to memory for holding the decoded string.
 * Must be large enough to receive the decoded string.
 * @param src A base64 encoded string.
 * @return the length of the decoded string if decode
 * succeeded otherwise 0.
 */
int decode_base64(unsigned char *dest, const char *src) {
  if(src && *src) {
    unsigned char *p= dest;
    int k, l= strlen(src)+1;
    unsigned char *buf= (unsigned char*) calloc(sizeof(unsigned char), l);

    /* Ignore non base64 chars as per the POSIX standard */
    for(k=0, l=0; src[k]; k++) {
      if(b64_is_base64(src[k])) {
        buf[l++]= src[k];
      }
    }

    for(k=0; k<l; k+=4) {
      char c1='A', c2='A', c3='A', c4='A';
      unsigned char b1=0, b2=0, b3=0, b4=0;
      c1= buf[k];

      if(k+1<l) c2= buf[k+1];
      if(k+2<l) c3= buf[k+2];
      if(k+3<l) c4= buf[k+3];

      b1= b64_decode(c1);
      b2= b64_decode(c2);
      b3= b64_decode(c3);
      b4= b64_decode(c4);

      *p++=((b1<<2)|(b2>>4) );

      if(c3 != '=') *p++=(((b2&0xf)<<4)|(b3>>2) );
      if(c4 != '=') *p++=(((b3&0x3)<<6)|b4 );
    }
    free(buf);
    dest[p-dest]='\0';
    return(p-dest);
  }
  return 0;
}

/**
 * Escape 'string' according to RFC3986 and
 * http://oauth.net/core/1.0/#encoding_parameters.
 *
 * @param string The data to be encoded
 * @return encoded string otherwise NULL
 * The caller must free the returned string.
 */
char *url_escape(const char *string) {
  size_t alloc, newlen;
  char *ns = NULL, *testing_ptr = NULL;
  unsigned char in;
  size_t strindex=0;
  size_t length;

  if (!string) return strdup("");

  alloc = strlen(string)+1;
  newlen = alloc;

  ns = (char*) malloc(alloc);

  length = alloc-1;
  while(length--) {
    in = *string;

    switch(in){
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    case 'a': case 'b': case 'c': case 'd': case 'e':
    case 'f': case 'g': case 'h': case 'i': case 'j':
    case 'k': case 'l': case 'm': case 'n': case 'o':
    case 'p': case 'q': case 'r': case 's': case 't':
    case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
    case 'A': case 'B': case 'C': case 'D': case 'E':
    case 'F': case 'G': case 'H': case 'I': case 'J':
    case 'K': case 'L': case 'M': case 'N': case 'O':
    case 'P': case 'Q': case 'R': case 'S': case 'T':
    case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
    case '_': case '~': case '.': case '-':
      ns[strindex++]=in;
      break;
    default:
      newlen += 2; /* this'll become a %XX */
      if(newlen > alloc) {
        alloc *= 2;
        testing_ptr = (char*) realloc(ns, alloc);
        ns = testing_ptr;
      }
      snprintf(&ns[strindex], 4, "%%%02X", in);
      strindex+=3;
      break;
    }
    string++;
  }
  ns[strindex]=0;
  return ns;
}

#ifndef ISXDIGIT
# define ISXDIGIT(x) (isxdigit((int) ((unsigned char)x)))
#endif

/**
 * Parse RFC3986 encoded 'string' back to  unescaped version.
 *
 * @param string The data to be unescaped
 * @param olen unless NULL the length of the returned string is stored there.
 * @return decoded string or NULL
 * The caller must free the returned string.
 */
char *url_unescape(const char *string, size_t *olen) {
  size_t alloc, strindex=0;
  char *ns = NULL;
  unsigned char in;
  long hex;

  if (!string) return NULL;
  alloc = strlen(string)+1;
  ns = (char*) malloc(alloc);

  while(--alloc > 0) {
    in = *string;
    if(('%' == in) && ISXDIGIT(string[1]) && ISXDIGIT(string[2])) {
      char hexstr[3]; // '%XX'
      hexstr[0] = string[1];
      hexstr[1] = string[2];
      hexstr[2] = 0;
      hex = strtol(hexstr, NULL, 16);
      in = (unsigned char)hex; /* hex is always < 256 */
      string+=2;
      alloc-=2;
    }
    ns[strindex++] = in;
    string++;
  }
  ns[strindex]=0;
  if(olen) *olen = strindex;
  return ns;
}

/**
 * build a url query string from an array.
 *
 * @param argc the total number of elements in the array
 * @param start element in the array at which to start concatenating.
 * @param argv parameter-array to concatenate.
 * @return url string needs to be freed by the caller.
 *
 */
char *serialize_url (int argc, int start, char **argv) {
  return serialize_url_sep( argc, start, argv, "&", 0);
}

/**
 * encode query parameters from an array.
 *
 * @param argc the total number of elements in the array
 * @param start element in the array at which to start concatenating.
 * @param argv parameter-array to concatenate.
 * @param sep separator for parameters (usually "&")
 * @param mod - bitwise modifiers:
 *   1: skip all values that start with "oauth_"
 *   2: skip all values that don't start with "oauth_"
 *   4: add double quotation marks around values (use with sep=", " to generate HTTP Authorization header).
 * @return url string needs to be freed by the caller.
 */
static char *serialize_url_sep (int argc, int start, char **argv, char *sep, int mod) {
  char  *tmp, *t1;
  int i;
  int  first=0;
  int seplen=strlen(sep);
  char *query = (char*) malloc(sizeof(char));
  *query='\0';
  for(i=start; i< argc; i++) {
    int len = 0;
    if ((mod&1)==1 && (strncmp(argv[i],"oauth_",6) == 0 || strncmp(argv[i],"x_oauth_",8) == 0) ) continue;
    if ((mod&2)==2 && (strncmp(argv[i],"oauth_",6) != 0 && strncmp(argv[i],"x_oauth_",8) != 0) && i!=0) continue;

    if (query) len+=strlen(query);

    if (i==start && i==0 && strstr(argv[i], ":/")) {
      tmp=strdup(argv[i]);
      len+=strlen(tmp);
    } else if(!(t1=strchr(argv[i], '='))) {
    // see http://oauth.net/core/1.0/#anchor14
    // escape parameter names and arguments but not the '='
      tmp=strdup(argv[i]);
      tmp=(char*) realloc(tmp,(strlen(tmp)+2)*sizeof(char));
      strcat(tmp,"=");
      len+=strlen(tmp);
    } else {
      *t1=0;
      tmp = url_escape(argv[i]);
      *t1='=';
      t1 = url_escape((t1+1));
      tmp=(char*) realloc(tmp,(strlen(tmp)+strlen(t1)+2+(mod&4?2:0))*sizeof(char));
      strcat(tmp,"=");
      if (mod&4) strcat(tmp,"\"");
      strcat(tmp,t1);
      if (mod&4) strcat(tmp,"\"");
      free(t1);
      len+=strlen(tmp);
    }
    len+=seplen+1;
    query=(char*) realloc(query,len*sizeof(char));
    strcat(query, ((i==start||first)?"":sep));
    first=0;
    strcat(query, tmp);
    if (i==start && i==0 && strstr(tmp, ":/")) {
      strcat(query, "?");
      first=1;
    }
    free(tmp);
  }
  return (query);
}

/**
 * build a query parameter string from an array.
 *
 * This function is a shortcut for \ref oauth_serialize_url (argc, 1, argv).
 * It strips the leading host/path, which is usually the first
 * element when using oauth_split_url_parameters on an URL.
 *
 * @param argc the total number of elements in the array
 * @param argv parameter-array to concatenate.
 * @return url string needs to be freed by the caller.
 */
char *serialize_url_parameters (int argc, char **argv) {
  return serialize_url(argc, 1, argv);
}
