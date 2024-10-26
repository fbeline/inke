#include "ireplace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "ds.h"
#include "globals.h"
#include "isearch.h"
#include "prompt.h"

static void ireplace_awns(void);
static void replace(void);

static void replace(void) {
  const char *answer = prompt_text();

  if (strlen(answer) == 0) {
    // TODO: replace occurence
    ireplace_awns();
    return;
  }
  if (answer[0] == '!') {
    buffer_t *bp = buffer_get();
    line_t *lp = bp->lp;
    u32 counter = 0;
    while(lp != NULL) {
      counter += dsreplace_all(lp->ds, g_replace.query, g_replace.with);
      lp = lp->next;
    }
    set_status_message("Replaced %d occurrences", counter);
  }
  g_flags &= ~(MCMD | MSEARCH);
  g_flags |= MINSERT;
}

static void ireplace_not_found(void) {
  g_flags &= ~(MCMD | MSEARCH);
  g_flags |= MINSERT;
  char message[256];
  snprintf(message, 256, "0 occurrences of %s", g_replace.query);
  set_status_message(message);
}

static void ireplace_awns(void) {
  if (g_replace.with == NULL) {
    g_replace.with = strdup(prompt_text());
  }

  g_flags |= MSEARCH;
  g_isearch = (isearch_t) {.lp = NULL, .qlen = 0, .x = 0};
  isearch_search(buffer_get(), g_replace.query, 1);

  if (g_isearch.lp == NULL) {
    ireplace_not_found();
    return;
  }

  char prompt[256];
  snprintf(prompt,
           256,
           "Query replacing: %s with %s. [!]ALL : ",
           g_replace.query,
           g_replace.with);

  prompt_init(prompt);
  g_cmd_func = replace;
}

static void ireplace_with(void) {
  const char *query = prompt_text();
  g_replace.query = strdup(query);
  prompt_init("Query replace with: ");
  g_cmd_func = ireplace_awns;
}

static void ireplace_init(void) {
  if (g_replace.query != NULL) free(g_replace.query);
  if (g_replace.with != NULL) free(g_replace.with);

  g_replace.n = 0;
  g_replace.query = NULL;
  g_replace.with = NULL;
}

void ireplace_start(buffer_t *B) {
  ireplace_init();
  prompt_init("Query replace: ");
  g_flags &= ~MINSERT;
  g_flags |= MCMD;
  g_cmd_func = ireplace_with;
  g_cmd_complete_func = NULL;
}
