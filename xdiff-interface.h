#ifndef XDIFF_INTERFACE_H
#define XDIFF_INTERFACE_H

#include "cache.h"
#include "xdiff/xdiff.h"

/*
 * xdiff isn't equipped to handle content over a gigabyte;
 * we make the cutoff 1GB - 1MB to give some breathing
 * room for constant-sized additions (e.g., merge markers)
 */
#define MAX_XDIFF_SIZE (1024UL * 1024 * 1023)

/*
 * The xdiff_emit_{line,hunk}_fn consumers can return -1 to abort
 * early, or 0 to continue processing. Note that doing so is an
 * all-or-nothing affair, as returning -1 will return all the way to
 * the top-level, e.g. the xdi_diff_outf() call to generate the diff.
 *
 * Thus returning -1 from a hunk header callback means you won't be
 * getting any more hunks, or diffs, and likewise returning from a
 * line callback means you won't be getting anymore lines.
 *
 * We may extend the interface in the future to understand other more
 * granular return values, but for now use it carefully, or consider
 * e.g. using discard_hunk_line() if you say just don't care about
 * hunk headers.
 *
 * Note that just returning -1 will make your early return
 * indistinguishable from an error internal to xdiff. See "diff_grep"
 * in diffcore-pickaxe.c for a trick to work around this, i.e. using
 * the "consume_callback_data" to note the desired early return.
 */
typedef int (*xdiff_emit_line_fn)(void *, char *, unsigned long);
typedef int (*xdiff_emit_hunk_fn)(void *data,
				  long old_begin, long old_nr,
				  long new_begin, long new_nr,
				  const char *func, long funclen);

int xdi_diff(mmfile_t *mf1, mmfile_t *mf2, xpparam_t const *xpp, xdemitconf_t const *xecfg, xdemitcb_t *ecb);
int xdi_diff_outf(mmfile_t *mf1, mmfile_t *mf2,
		  xdiff_emit_hunk_fn hunk_fn,
		  xdiff_emit_line_fn line_fn,
		  void *consume_callback_data,
		  xpparam_t const *xpp, xdemitconf_t const *xecfg);
int read_mmfile(mmfile_t *ptr, const char *filename);
void read_mmblob(mmfile_t *ptr, const struct object_id *oid);
int buffer_is_binary(const char *ptr, unsigned long size);

void xdiff_set_find_func(xdemitconf_t *xecfg, const char *line, int cflags);
void xdiff_clear_find_func(xdemitconf_t *xecfg);
int git_xmerge_config(const char *var, const char *value, void *cb);
extern int git_xmerge_style;

/*
 * Can be used as a no-op hunk_fn for xdi_diff_outf(), since a NULL
 * one just sends the hunk line to the line_fn callback).
 */
int discard_hunk_line(void *priv,
		      long ob, long on, long nb, long nn,
		      const char *func, long funclen);

/*
 * Compare the strings l1 with l2 which are of size s1 and s2 respectively.
 * Returns 1 if the strings are deemed equal, 0 otherwise.
 * The `flags` given as XDF_WHITESPACE_FLAGS determine how white spaces
 * are treated for the comparison.
 */
int xdiff_compare_lines(const char *l1, long s1,
			const char *l2, long s2, long flags);

/*
 * Returns a hash of the string s of length len.
 * The `flags` given as XDF_WHITESPACE_FLAGS determine how white spaces
 * are treated for the hash.
 */
unsigned long xdiff_hash_string(const char *s, size_t len, long flags);

#endif
