#include <stddef.h> // NULL
#include <string.h> // strdup

#include "store/report.h"
#include "store/types.h"

static void appendParseReport(StoreParseReport *report, StoreDynamicString reportString, int level);
static void appendNTimes(StoreDynamicString string, int n, const char *what);

char *StoreGenerateParseReport(StoreParser *parser)
{
	StoreDynamicString reportString = StoreCreateDynamicString();
	appendParseReport(parser->state.lastReport, reportString, 0);

	char *result = strdup(StoreReadDynamicString(reportString));
	StoreFreeDynamicString(reportString);
	return result;
}

static void appendParseReport(StoreParseReport *report, StoreDynamicString reportString, int level)
{
	if(report == NULL) {
		return;
	}

	appendParseReport(report->previousReport, reportString, level);

	appendNTimes(reportString, level, "\t");
	if(report->success) {
		StoreAppendDynamicString(reportString, "successfully parsed %s at line %d, column %d: %s\n", report->type, report->position.line, report->position.column, report->message);
	} else {
		StoreAppendDynamicString(reportString, "failed to parse %s at line %d, column %d: %s\n", report->type, report->position.line, report->position.column, report->message);
	}

	appendParseReport(report->lastSubReport, reportString, level + 1);
}

static void appendNTimes(StoreDynamicString string, int n, const char *what)
{
	for(int i = 0; i < n; i++) {
		StoreAppendDynamicString(string, what);
	}
}
