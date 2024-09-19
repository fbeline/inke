#include "ds.h"

#include <string.h>

#define BLOCK_SIZE 16

static size_t dscalcalloc(size_t len) {
  len = (len + BLOCK_SIZE) & ~(BLOCK_SIZE - 1);
  if (len == 0) len = BLOCK_SIZE;

  return len;
}

static ds_t *dsrealloc(ds_t *ds, size_t len) {
  ds->alloc = dscalcalloc(len);
  if ((ds->buf = realloc(ds->buf, ds->alloc)) == NULL)
    exit(1);

  return ds;
}

ds_t *dsnewlen(size_t len) {
  ds_t *ds;

  if ((ds = (ds_t*)malloc(sizeof(ds_t))) == NULL)
    exit(1);

  ds->alloc = dscalcalloc(len);
  ds->len = 0;

  if ((ds->buf = malloc(ds->alloc)) == NULL)
    exit(1);

  ds->buf[0] = '\0';

  return ds;
}

ds_t *dsnew(const char *init) {
  size_t len = strlen(init);
  ds_t *ds = dsnewlen(len);
  strncpy(ds->buf, init, len);

  return ds;
}

ds_t *dsempty(void) {
  return dsnewlen(0);
}

ds_t *dscat(ds_t *ds, const char *t) {
  size_t tlen = strlen(t);
  ds->len += tlen;

  if (ds->len >= ds->alloc)
    ds = dsrealloc(ds, ds->len);

  strncat(ds->buf, t, tlen);
  ds->buf[ds->len] = '\0';

  return ds;
}

ds_t *dscatds(ds_t *ds, const ds_t *t) {
  return dscat(ds, t->buf);
}

void dsfree(ds_t *ds) {
  free(ds->buf);
  free(ds);
}
