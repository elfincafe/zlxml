#ifndef _ZLXML_H
#define _ZLXML_H

#include <stdlib.h>
#include <stdbool.h>
#include "zlstring.h"

struct _zlXmlAttr;
struct _zlXmlAttrContainer;
struct _zlXmlElem;
struct _zlXmlElemContainer;

typedef struct _zlXmlAttr
{
	zlString *key;
	zlString *value;
} zlXmlAttr;

typedef struct _zlXmlAttrContainer
{
	size_t size;
	size_t count;
	struct _zlXmlAttr **attrs;
} zlXmlAttrContainer;

typedef struct _zlXmlElem
{
	zlString *name;
	size_t depth;
	struct _zlXmlAttrContainer *attrs;
	struct _zlXmlElemContainer *children;
	struct _zlXmlElem *parent;
	zlString *data;
} zlXmlElem;

typedef struct _zlXmlElemContainer
{
	size_t size;
	size_t count;
	struct _zlXmlElem **elems;
} zlXmlElemContainer;

typedef struct _zlXml
{
	bool standalone;
	char encoding[16];
	float version;
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
