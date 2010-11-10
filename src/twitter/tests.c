#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oauth.h>
#include <readline/readline.h>
#include <readline/history.h>

char* get_param(char** argv, int argc, const char* key)
{
    int i, c;

    c = strlen(key);
    for (i = 0; i < argc; i++) {
        //printf("\"%s\" argv[%d].strlen = %d, key.strlen = %d, eq=\"%c\"\n", argv[i], i, strlen(argv[i]), c, argv[i][c]);
        if (!strncmp(argv[i], key, c) && strlen(argv[i]) > c && argv[i][c] == '=')
            return strdup(&argv[i][c + 1]);
    }

    return NULL;
}

/*
 * a example requesting and parsing a request-token from an OAuth service-provider
 * using the oauth-HTTP POST function.
 */
void request_token_example_post(void) {
#if 0
  const char *request_token_uri = "http://oauth-sandbox.mediamatic.nl/module/OAuth/request_token";
  const char *req_c_key         = "17b09ea4c9a4121145936f0d7d8daa28047583796"; //< consumer key
  const char *req_c_secret      = "942295b08ffce77b399419ee96ac65be"; //< consumer secret
#else
  const char *request_token_uri = "https://api.twitter.com/oauth/request_token";
  const char *access_token_uri  = "https://api.twitter.com/oauth/access_token";
  const char *authorize_uri     = "https://api.twitter.com/oauth/authorize";
  const char *req_c_key         = "dY49wvIY7386ET15vCRVQ"; //< consumer key
  const char *req_c_secret      = "D8wKnvbTQqxJmxBC0JyHVY6LpHdpTBbJwyISlrUg8"; //< consumer secret
#endif
  char *res_t_key    = NULL; //< replied key
  char *res_t_secret = NULL; //< replied secret

  char *postarg = NULL;
  char *req_url;
  char *reply;

  req_url = oauth_sign_url2(request_token_uri, &postarg, OA_HMAC, NULL, req_c_key, req_c_secret, NULL, NULL);

  printf("request URL:%s\n\n", req_url);
  reply = oauth_http_post2(req_url, postarg, NULL);
  if (!reply)
    printf("HTTP request for an oauth request-token failed.\n");
  else {
    //parse reply - example:
    //"oauth_token=2a71d1c73d2771b00f13ca0acb9836a10477d3c56&oauth_token_secret=a1b5c00c1f3e23fb314a0aa22e990266"
    int rc;
    char **rv = NULL;
    printf("HTTP-reply: %s\n", reply);
    rc = oauth_split_url_parameters(reply, &rv);
    char* key = get_param(rv, rc, "oauth_token");
    char* secret = get_param(rv, rc, "oauth_token_secret");
    printf("key: \"%s\"\n", key);
    printf("secret: \"%s\"\n", secret);
    printf("URI: %s?oauth_token=%s\n", authorize_uri, key);
    char* cmd;
    asprintf(&cmd, "xdg-open \"%s?oauth_token=%s\"", authorize_uri, key);
    system(cmd);
    free(cmd);

    free(req_url);
    free(reply);
    free(postarg);
    postarg = NULL;

    char* pin = readline("PIN: ");

    char* uri = NULL;
    asprintf(&uri, "%s?oauth_verifier=%s", access_token_uri, pin);
    req_url = oauth_sign_url2(uri, &postarg, OA_HMAC, NULL, req_c_key, req_c_secret, key, secret);
    reply = oauth_http_post(req_url, postarg);
    printf("HTTP-reply: %s\n", reply);

    free(req_url);
    free(pin);
    free(uri);
    oauth_free_array(&rc, &rv);

    rv = NULL;
    rc = oauth_split_url_parameters(reply, &rv);
    free(reply);

    key = get_param(rv, rc, "oauth_token");
    secret = get_param(rv, rc, "oauth_token_secret");
    printf("key: \"%s\"\n", key);
    printf("secret: \"%s\"\n", secret);

    uri = NULL;
    char* status = readline("tweet> ");
    postarg = NULL;
    asprintf(&uri, "http://api.twitter.com/1/statuses/update.xml?status=%s", oauth_url_escape(status));
    req_url = oauth_sign_url2(uri, &postarg, OA_HMAC, NULL, req_c_key, req_c_secret, key, secret);
    free(uri);

    reply = oauth_http_post(req_url, postarg);
    printf("HTTP-reply: %s\n", reply);

    if(rv) free(rv);
  }

  if(req_url) free(req_url);
  if(postarg) free(postarg);
  if(reply) free(reply);
  if(res_t_key) free(res_t_key);
  if(res_t_secret) free(res_t_secret);
}

int main(int argc, char* argv[])
{
    request_token_example_post();
    return 0;
}
