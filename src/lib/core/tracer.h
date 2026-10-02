/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright 2010-2026, Tarantool AUTHORS, please see AUTHORS file.
 */

#pragma once

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

/** A single span being tracked. */
struct span {
	/** This span's own id, 16 hex characters. */
	char span_id[16];
	/** Span name. */
	char *name;
	/**
	 * W3C traceparent of the parent context. All zero bytes
	 * when the span is root.
	 */
	char traceparent[55];
	/** Span start time. */
	double start_time;
	/** Span end time. */
	double end_time;
};

/**
 * Initialize a span: store its name, own id, and the parent
 * traceparent (all-zero traceparent means the span is root), then
 * record the start time.
 */
void
start_span(struct span *span, const char *name,
	   char span_id[16], char traceparent[55]);

/** Record the span's end time. */
void
end_span(struct span *span);

/** Record the information about span. */
int
write_span(struct span *span);

#if defined(__cplusplus)
} /* extern "C" */
#endif /* defined(__cplusplus) */
