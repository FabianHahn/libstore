#include <stdlib.h> // NULL EXIT_FAILURE EXIT_SUCCESS malloc free
#include <stdio.h> // SEEK_END SEEK_SET printf FILE fopen fseek ftell fread fclose fflush stdout puts

#include <store/parser.h>
#include <store/report.h>
#include <store/store.h>

static char *readFile(const char *filename);
static char *getStoreInfo(Store *store);

int main(int argc, char *argv[])
{
	if(argc <= 1) {
		printf("Usage: %s <input file>+\n", argv[0]);
		return EXIT_FAILURE;
	}

	StoreParser *parser = storeCreateParser();

	int successful = 0;
	for(int i = 1; i < argc; i++) {
		const char *filename = argv[i];
		char *contents = readFile(filename);
		if(contents == NULL) {
			printf("Failed to read file %d out of %d from '%s'.\n", i, argc - 1, filename);
			continue;
		}

		printf("Parsing file %d out of %d from '%s': ", i, argc - 1, filename);
		fflush(stdout);
		storeResetParser(parser);
		Store *store = storeParse(parser, contents);
		free(contents);

		char *report = storeGenerateParseReport(parser, 10);
		puts(report);
		free(report);

		if(store == NULL) {
			continue;
		}

		storeFree(store);
		successful++;
	}

	storeFreeParser(parser);

	printf("Successfully parsed %d out of %d file(s) (%.2f%%) overall.\n", successful, argc - 1, 100.0f * (float) successful / (argc - 1));

	if(successful == argc - 1) {
		return EXIT_SUCCESS;
	} else {
		return EXIT_FAILURE;
	}
}

static char *readFile(const char *filename)
{
	FILE *file = fopen(filename, "rb");
	if(file == NULL) {
		return NULL;
	}

	if(fseek(file, 0, SEEK_END) != 0) {
		fclose(file);
		return NULL;
	}

	long length = ftell(file);
	if(length < 0) {
		fclose(file);
		return NULL;
	}

	if(fseek(file, 0, SEEK_SET) != 0) {
		fclose(file);
		return NULL;
	}

	char *contents = malloc((length + 1) * sizeof(char));
	if(fread(contents, 1, length, file) != length) {
		free(contents);
		fclose(file);
		return NULL;
	}

	contents[length] = '\0';
	fclose(file);
	return contents;
}
