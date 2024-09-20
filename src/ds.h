#pragma once

#include <stdlib.h>

typedef struct ds_s {
  size_t len;
  size_t alloc;
  char *buf;
} ds_t;

ds_t *dsnewlen(size_t len);
ds_t *dsnew(const char *init);
ds_t *dsempty(void);
ds_t *dscat(ds_t *ds, const char *t);
ds_t *dscatds(ds_t *ds, const ds_t *t);
ds_t *dssplice(ds_t *ds, size_t idx, const char *t);
void dsfree(ds_t *ds);
