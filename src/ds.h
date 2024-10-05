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
ds_t *dsncat(ds_t *ds, const char *t, size_t n);
ds_t *dscat(ds_t *ds, const char *t);
ds_t *dscatds(ds_t *ds, const ds_t *t);
ds_t *dssplice(ds_t *ds, size_t idx, const char *t);
ds_t *dsichar(ds_t *ds, size_t idx, const char ch);
ds_t *dsrtrim(ds_t *ds);
ds_t *dsrealloc(ds_t *ds, size_t len);
void dsfree(ds_t *ds);
