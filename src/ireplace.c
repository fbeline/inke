#include "ireplace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "prompt.h"

static void replace(void) {
  // const char *answer = prompt_text();
  // TODO: IMPL REPLACE RESPONSE
  // !     : replace all
  // ENTER : replace current goes to next
  g_flags &= ~MCMD;
  g_flags |= MINSERT;
}

static void ireplace_awns(void) {
  const char *with = prompt_text();
  g_replace.with = strdup(with);

  char prompt[256];
  snprintf(prompt,
           256,
           "Query replacing: %s with %s. [!]ALL : ",
           g_replace.query,
           g_replace.with);

  prompt_init(prompt);
  g_cmd_func = replace;

  // TODO: find and highligh the first occurence
}

static void ireplace_with(void) {
  const char *query = prompt_text();
  g_replace.query = strdup(query);
  prompt_init("Query replace with: ");
  g_cmd_func = ireplace_awns;
}

static void ireplace_init(void) {
  g_replace.n = 0;
  if (g_replace.query != NULL) free(g_replace.query);
  if (g_replace.with != NULL) free(g_replace.with);
}

void ireplace_start(buffer_t *B) {
  ireplace_init();
  prompt_init("Query replace: ");
  g_flags &= ~MINSERT;
  g_flags |= MCMD;
  g_cmd_func = ireplace_with;
  g_cmd_complete_func = NULL;
}
