/*
*   Copyright (c) 2001-2002, Nick Hibma <n_hibma@van-laarhoven.org>
*
*   This source code is released for free distribution under the terms of the
*   GNU General Public License version 2 or (at your option) any later version.
*
*   This module contains functions for generating tags for YACC language files.
*/

/*
*   INCLUDE FILES
*/
#include "general.h"  /* must always come first */

#include <string.h>
#include "parse.h"
#include "routines.h"
#include "read.h"
#include "mio.h"
#include "promise.h"


static boolean not_in_grammar_rules = TRUE;
static tagRegexTable yaccTagRegexTable [] = {
	{"^([A-Za-z][A-Za-z_0-9]+)[ \t]*:", "\\1",
	 "l,label,labels", NULL, &not_in_grammar_rules },
};

struct cStart {
	unsigned long input;
	unsigned long source;
};

static  void change_section (const char *line __unused__,
			     const regexMatch *matches __unused__,
			     unsigned int count __unused__,
			     void *data __unused__)
{
	not_in_grammar_rules = !not_in_grammar_rules;

	if (not_in_grammar_rules)
	{
		unsigned long c_start;
		unsigned long c_source_start;
		unsigned long c_end;

		readLineFromInputFile ();
		c_start = getInputLineNumber ();
		c_source_start = getSourceLineNumber();

		/* Skip the lines for finding the EOF. */
		while (readLineFromInputFile ())
			;

		c_end = getInputLineNumber ();

		makePromise ("C", c_start, 0, c_end, 0, c_source_start);
	}
}

static void enter_c_prologue (const char *line __unused__,
			      const regexMatch *matches __unused__,
			      unsigned int count __unused__,
			      void *data)
{
	struct cStart *cstart = data;


	readLineFromInputFile ();
	cstart->input  = getInputLineNumber ();
	cstart->source = getSourceLineNumber ();
}

static void leave_c_prologue (const char *line __unused__,
			      const regexMatch *matches __unused__,
			      unsigned int count __unused__,
			      void *data)
{
	struct cStart *cstart = data;
	unsigned long c_end;

	c_end = getInputLineNumber ();
	makePromise ("C", cstart->input, 0, c_end, 0, cstart->source);
}


static void initializeYaccParser (langType language)
{
	/*
	   %{ ...
		C language
	   %}
		TOKE DEFINITIONS
	   %%
		SYNTAX
	   %%
		C language
	*/

	static struct cStart cStart;

	memset (&cStart, 0, sizeof (cStart));

	addCallbackRegex (language, "^%\\{", "{exclusive}", enter_c_prologue, NULL, &cStart);
	addCallbackRegex (language, "^%\\}", "{exclusive}", leave_c_prologue, NULL, &cStart);

	not_in_grammar_rules = TRUE;
	addCallbackRegex (language, "^%%", "{exclusive}", change_section, NULL, NULL);
}

extern parserDefinition* YaccParser (void)
{
	static const char *const extensions [] = { "y", NULL };
	parserDefinition* const def = parserNew ("YACC");
	def->extensions = extensions;
	def->initialize = initializeYaccParser;
	def->method     = METHOD_NOT_CRAFTED|METHOD_REGEX;
	def->tagRegexTable = yaccTagRegexTable;
	def->tagRegexCount = ARRAY_SIZE (yaccTagRegexTable);
	return def;
}

/* vi:set tabstop=4 shiftwidth=4: */
