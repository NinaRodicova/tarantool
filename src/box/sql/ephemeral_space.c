#include "sqlInt.h"
#include "mem.h"
#include "msgpuck/msgpuck.h"

struct sql_ephemeral_space *
sql_ephemeral_new(const struct sql_space_info *info)
{
	struct sql_ephemeral_space *eph_space =
		sql_xmalloc(sizeof(struct sql_ephemeral_space));
	eph_space->field_count = info->field_count;
	eph_space->header = info->types;
	eph_space->tuple_count = 0;
	eph_space->max_tuple_count = 0;
	eph_space->i_cur = 0;
	return eph_space;
}

int
sql_ephemeral_insert(struct sql_ephemeral_space *space, const char *tuple,
		     const char *tuple_end)
{
	if (space->max_tuple_count == 0) {
		space->tuples = sql_xmalloc(sizeof(char *));
		space->tuple_sizes = sql_xmalloc(sizeof(uint32_t));
		space->max_tuple_count++;
	} else if (space->max_tuple_count == space->tuple_count) {
		space->max_tuple_count *= 2;
		space->tuples = sql_xrealloc(space->tuples,
					    space->max_tuple_count *
					    sizeof(char *));
		space->tuple_sizes = sql_xrealloc(space->tuple_sizes,
					    space->max_tuple_count *
					    sizeof(char *));
	}
	uint32_t size_tuple = tuple_end - tuple;
	space->tuples[space->tuple_count] = sql_xmalloc(size_tuple);
	memcpy(space->tuples[space->tuple_count], tuple, size_tuple);
	space->tuple_sizes[space->tuple_count] = size_tuple;
	space->tuple_count++;
	return 0;
}

int
sql_ephemeral_find(struct sql_ephemeral_space *space, const char *tuple,
		   const char *tuple_end)
{
	uint32_t size = tuple_end - tuple;
	for (uint32_t i = 0; i < space->tuple_count; i++) {
		if (space->tuple_sizes[i] == size) {
			if (memcmp(tuple, space->tuples[i], size) == 0)
				return i;
		}
	}
	return -1;
}

int
sql_ephemeral_first(struct sql_ephemeral_space *eph_space, int *res)
{
	if (eph_space == NULL)
		return -1;
	if (eph_space->tuple_count == 0)
		*res = 1;
	else
		*res = 0;
	eph_space->i_cur = 0;
	return 0;
}

int
sql_ephemeral_next(struct sql_ephemeral_space *eph_space, int *res)
{
	if (eph_space == NULL)
		return -1;
	eph_space->i_cur++;
	if (eph_space->i_cur == eph_space->tuple_count)
		*res = 1;
	else
		*res = 0;
	return 0;
}

int
sql_ephemeral_prev(struct sql_ephemeral_space *eph_space, int *res)
{
	if (eph_space == NULL)
		return -1;
	eph_space->i_cur--;
	if (eph_space->i_cur >= eph_space->tuple_count)
		*res = 1;
	else
		*res = 0;
	return 0;
}

enum field_type
sql_ephemeral_get_type(struct sql_ephemeral_space *eph_space, uint32_t fieldno)
{
	if (fieldno >= eph_space->field_count)
		return field_type_MAX;
	return eph_space->header[fieldno];
}

/**
 * Tuples of a lite ephemeral space are stored as plain MessagePack
 * arrays (see sql_ephemeral_insert()). To fetch field fieldno the
 * array is decoded from its start, skipping the preceding fields.
 */
int
sql_ephemeral_get_cell(struct sql_ephemeral_space *eph_space, uint32_t fieldno,
			Mem *dest_mem)
{
	if (eph_space == NULL || eph_space->i_cur >= eph_space->tuple_count)
		return -1;
	if (fieldno >= eph_space->field_count)
		return 0;
	const char *data = eph_space->tuples[eph_space->i_cur];
	uint32_t field_count = mp_decode_array(&data);
	assert(field_count == eph_space->field_count);
	(void)field_count;
	for (uint32_t i = 0; i < fieldno; i++)
		mp_next(&data);
	uint32_t len;
	return mem_from_mp(dest_mem, data, &len);
}

void
sql_ephemeral_delete(struct sql_ephemeral_space *eph_space)
{
	//sql_xfree(eph_space->header);
	for (uint32_t i = 0; i < eph_space->tuple_count; i++) {
		sql_xfree(eph_space->tuples[i]);
	}
	if (eph_space->max_tuple_count > 0) {
		sql_xfree(eph_space->tuples);
		sql_xfree(eph_space->tuple_sizes);
	}
	sql_xfree(eph_space);
}
