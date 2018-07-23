#include <stdbool.h> // bool
#include <stddef.h> // NULL
#include <string.h> // strdup

#include <glib.h>

#include "store/report.h"

static void appendParseReport(StoreParseReport *report, GString *reportString, int level);
static void appendNTimes(GString *string, int n, const char *what);

char *storeGenerateParseReport(StoreParser *parser)
{
	GString *reportString = g_string_new("");
	appendParseReport(parser->state.lastReport, reportString, 0);

	char *result = reportString->str;
	g_string_free(reportString, false);
	return result;
}

static void appendParseReport(StoreParseReport *report, GString *reportString, int level)
{
	if(report == NULL) {
		return;
	}

	appendParseReport(report->previousReport, reportString, level);

	appendNTimes(reportString, level, "\t");
	if(report->success) {
		g_string_append_printf(reportString, "successfully parsed %s at line %d, column %d: %s\n", report->type, report->position.line, report->position.column, report->message);
	} else {
		g_string_append_printf(reportString, "failed to parse %s at line %d, column %d: %s\n", report->type, report->position.line, report->position.column, report->message);
	}

	appendParseReport(report->lastSubReport, reportString, level + 1);
}

static void appendNTimes(GString *string, int n, const char *what)
{
	for(int i = 0; i < n; i++) {
		g_string_append_printf(string, what);
	}
}
