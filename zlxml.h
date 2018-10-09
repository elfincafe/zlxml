#ifndef _ZLXML_H
#define _ZLXML_H

#include <stdlib.h>
#include <stdbool.h>
#include "zlstring.h"


typedef struct _zlXmlAttr
{
	zlString *key;
	zlString *value;
} zlXmlAttr;

typedef struct _zlXmlElem
{
	zlString *name;
	size_t attr_count;
	zlXmlAttr **attrs;
	size_t child_count;
	struct _zlXmlElem **elems;
	struct _zlXmlElem *parent;
} zlXmlElem;

typedef struct _zlXml
{
	bool standalone;
	zlString *encoding;
	zlString *version;
	zlXmlElem *root;
} zlXml;


zlXml *zlxml_new();
zlXml *zlxml_load(zlString*);
zlXml *zlxml_parse(zlString*);
zlXmlElem *zlxml_new_elem();
zlXmlAttr *zlxml_new_attr(zlString*, zlString*);

void zlxml_free(zlXml*);
void zlxml_free_elem(zlXmlElem*);
void zlxml_free_attr(zlXmlAttr*);
void zlxml_dump(zlXml*);
void zlxml_dump_elem(zlXmlElem*);
void zlxml_dump_attr(zlXmlAttr*);

#endif // _ZLXML_H
