%{
	#include <stdio.h>
	#include <string.h>
	#include "store.h"
	#include "parse.h"
	#include "parser.h"
	#include "memory.h"
/*
	#define YYDEBUG 1
	#define YYFPRINTF fprintf
	int yydebug = 1;
*/
%}

%locations
%define api.pure
%parse-param {StoreParser *parser}
%lex-param {StoreParser *parser}

%{
	void yyerror(YYLTYPE *lloc, StoreParser *parser, char *error);
%}

%token <string> STORE_TOKEN_STRING
%token <integer> STORE_TOKEN_INTEGER
%token <float_number> STORE_TOKEN_FLOAT_NUMBER
%type <value> array
%type <value> list
%type <value> value
%type <node> node
%destructor { freeStore($$); } array list value
%destructor { freeStore($$->value); free($$->key); free($$); } node
%destructor { free($$); } STORE_TOKEN_STRING

%%
root:		%empty // empty string
			{
				parser->store = createStoreArrayValue(NULL);
			}
		|	array
			{
				@$.last_line = @1.last_line;
				@$.last_column = @1.last_column;
				parser->store = $1;
			}
;

node:	STORE_TOKEN_STRING '=' value
		{
			@$.last_line = @1.last_line;
			@$.last_column = @1.last_column;
			$$ = StoreAllocateMemoryType(StoreNode);
			$$->key = $1;
			$$->value = $3;
		}
;

value:		STORE_TOKEN_STRING
			{
				@$.last_line = @1.last_line;
				@$.last_column = @1.last_column;
				$$ = createStoreStringValue($1);
				free($1);
			}
		|	STORE_TOKEN_INTEGER
			{
				@$.last_line = @1.last_line;
				@$.last_column = @1.last_column;
				$$ = createStoreIntegerValue($1);
			}
		|	STORE_TOKEN_FLOAT_NUMBER
			{
				@$.last_line = @1.last_line;
				@$.last_column = @1.last_column;
				$$ = createStoreFloatNumberValue($1);
			}
		|	'(' ')'
			{
				@$.last_line = @2.last_line;
				@$.last_column = @2.last_column;
				$$ = createStoreListValue(NULL);
			}
		|	'(' list ')'
			{
				@$.last_line = @3.last_line;
				@$.last_column = @3.last_column;
				$$ = $2;
			}
		|	'{' '}'
			{
				@$.last_line = @2.last_line;
				@$.last_column = @2.last_column;
				$$ = createStoreArrayValue(NULL);
			}
		|	'{' array '}'
			{
				@$.last_line = @3.last_line;
				@$.last_column = @3.last_column;
				$$ = $2;
			}
;

list:		value // first element of the list
			{
				@$.last_line = @1.last_line;
				@$.last_column = @1.last_column;
				$$ = createStoreListValue(NULL);
				g_queue_push_head($$->content.list, $1);
			}
		|	list value // the list continues
			{
				@$.last_line = @2.last_line;
				@$.last_column = @2.last_column;
				g_queue_push_tail($1->content.list, $2);
				$$ = $1;
			}
;

array:		node // first element of the array
			{
				@$.last_line = @1.last_line;
				@$.last_column = @1.last_column;
				$$ = createStoreArrayValue(NULL);
				g_hash_table_insert($$->content.array, $1->key, $1->value);
				free($1);
			}
		|	array node // the array continues
			{
				@$.last_line = @2.last_line;
				@$.last_column = @2.last_column;
				g_hash_table_insert($1->content.array, $2->key, $2->value);
				free($2);
				$$ = $1;
			}
;
%%

void yyerror(YYLTYPE *lloc, StoreParser *parser, char *error)
{
	logError("Store parse error at line %d, column %d: %s", lloc->last_line, lloc->last_column - 1, error);
}
