#include "sqlInt.h"

struct sql_ephemeral_space *
sql_ephemeral_new(const struct sql_space_info *info)
{
	struct sql_ephemeral_space *eph_space =
		sql_xmalloc(sizeof(struct sql_ephemeral_space));
	eph_space->field_count = info->field_count;
	eph_space->header = info->types;
	eph_space->tuple_count = 0;
	eph_space->max_tuple_count = 0;
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
		printf("HEllo %d %d\n", space->tuple_sizes[i], size);
		if (space->tuple_sizes[i] == size) {
			printf("Here\n");
			if (memcmp(tuple, space->tuples[i], size) == 0)
				return i;
		}
	}
	return -1;
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
