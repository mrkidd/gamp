#ifndef _GAMP_GAP_METADATA_H_
#define _GAMP_GAP_METADATA_H_

#include <gst/gst.h>

typedef struct
{
	GstElement *pipeline;
	GstElement *sink;

	char *uri;

	char *title;
	char *artist;
	long duration;

	char *type;
	gboolean handoff;
	gboolean eos;
	GError *error;
} GAPMetaData;

void gap_metadata_load (GAPMetaData *md, const char *uri);

#endif /* _GAMP_GAP_METADATA_ */
