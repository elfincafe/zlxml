#ifndef _ZLXML_H
#define _ZLXML_H

#include <stdlib.h>
#include <stdbool.h>
#include <expat.h>
#include "zlstring.h"

typedef struct _zlXmlAttr
{
	char *key;
	char *value;
} zlXmlAttr;

typedef struct _zlXmlElem
{
	char *name;
	size_t depth;
	size_t attr_size;
	size_t attr_count;
	struct _zlXmlAttr **attrs;
	size_t child_size;
	size_t child_count;
	struct _zlXmlElem **children;
	struct _zlXmlElem *parent;
	size_t dat_len;
	char *data;
} zlXmlElem;

typedef struct _zlXml
{
	bool standalone;
	char encoding[16];
	float version;
	zlXmlElem *root;
} zlXml;

typedef struct _zlXmlWorker
{
	zlXml *doc;
	zlXmlElem *cur;
	zlXmlElem *prev;
	zlXmlElem *parent;
	size_t depth;
} zlXmlWorker;

zlXml *zlxml_new();
zlXml *zlxml_load(char*);
zlXml *zlxml_parse(char*, size_t);
zlXmlElem *zlxml_new_elem(char*, size_t);
zlXmlAttr *zlxml_new_attr(char*, size_t, char*, size_t);

void zlxml_free(zlXml*);
void zlxml_free_elem(zlXmlElem*);
void zlxml_free_attr(zlXmlAttr*);
void zlxml_dump(zlXml*);
void zlxml_dump_elem(zlXmlElem*);
void zlxml_dump_attr(zlXmlAttr*);

#endif // _ZLXML_H
