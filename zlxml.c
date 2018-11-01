#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "expat.h"
#include "zlstring.h"
#include "zlxml.h"

static zlXml *xmldoc;
static zlXmlElem *current;
static zlXmlElem *previous;
static zlXmlElem *parent;
static size_t depth;

static bool zlxml_add_attr(zlXmlElem *elem, zlXmlAttr *attr)
{
	// Realloc
	zlXmlAttrContainer *container = elem->attrs;
	size_t size = elem->attrs->size;
	size_t count = elem->attrs->count;
	if (size<=count) {
		zlXmlAttr **tmp = NULL;
		tmp = (zlXmlAttr**)realloc(container->attrs, sizeof(zlXmlAttr)*(elem->attrs->size+5));
		if (!tmp) {
			return false;
		}
		container->attrs = tmp;
		container->size += 5;
	}

	container->attrs[count] = attr;
	container->count++;

	return true;
}

static bool zlxml_add_child (zlXmlElem *elem, zlXmlElem *child)
{
	return true;
}

static void XMLCALL zlxml_handle_declaration(void *user_data, const XML_Char *version, const XML_Char *encoding, int standalone)
{
	xmldoc = zlxml_new();
	if (version) {
		xmldoc->version = strtof(version, NULL);
	}
	if (encoding) {
		strncpy(xmldoc->encoding, encoding, strlen(encoding));
	}
	if (standalone) {
		xmldoc->standalone = (bool)standalone;
	}
}

static void XMLCALL zlxml_handle_startelem(void *user_data, const XML_Char *name, const XML_Char **attrs)
{
	++depth;

	zlXmlElem *elem = zlxml_new_elem(zlstr_new(name, strlen(name)));
	elem->depth = depth;
	if (!xmldoc->root) {
		xmldoc->root = elem;
	}
	current = elem;

	// Parent
	zlXmlElem *prev;
	prev = previous;
	while (prev) {
		if (prev->depth==elem->depth-1) {
			parent = prev;
			break;
		}
		prev = prev->parent;
	}
	elem->parent = parent;

	// Attributes
	size_t i = 0;
	while (attrs[i]) {
		zlXmlAttr *attr = zlxml_new_attr(
			zlstr_new(attrs[i], strlen(attrs[i])),
			zlstr_new(attrs[i+1], strlen(attrs[i+1]))
		);
		zlxml_add_attr(elem, attr);
		i += 2;
	}

	// Post Processing
	previous = current;

zlxml_dump_elem(elem);
}

static void XMLCALL zlxml_handle_endelem(void *user_data, const char *name)
{
//	if (previous && previous->depth==current->depth-1) {
//		current->parent = parent = previous;
//	}
	--depth;
}

static void XMLCALL zlxml_handle_data(void *user_data, const XML_Char *s, int len)
{
	char buf[len+1];
	strncpy(buf, s, len);
}

static void XMLCALL zlxml_handle_startcdata(void *user_data)
{
	
}

static void XMLCALL zlxml_handle_endcdata(void *user_data)
{
	
}

/*
static void XMLCALL zlxml_handle_unknown_encoding (void *encodingHandlerData, const XML_Char *name, XML_Encoding *info)
{
}
*/

zlXml *zlxml_new()
{
	zlXml *xml;
	xml = (zlXml*)calloc(1, sizeof(zlXml));
	if (!xml) {
		return NULL;
	}
	xml->standalone = true;
	strncpy(xml->encoding, "UTF-8", 5);
	xml->version = 1.0;
	xml->root = NULL;
	return xml;
}

zlXml *zlxml_load(zlString *path)
{
	struct stat st;
	if (stat(path->content, &st)!=0) {
		return NULL;
	}

	FILE *fp;
	if ((fp=fopen(path->content, "rb"))==NULL) {
		return NULL;
	}

	char buf[st.st_size+1];
	fread(buf, 1, st.st_size, fp);
	buf[st.st_size] = '\0';
	fclose(fp);
	return zlxml_parse(zlstr_new(buf, st.st_size));
}

zlXml *zlxml_parse(zlString *data)
{
	depth = 0;
	current = NULL;
	previous = NULL;
	current = NULL;

	XML_Parser p = XML_ParserCreate(NULL);
	if (!p) {
		return NULL;
	}

	XML_SetXmlDeclHandler(p, zlxml_handle_declaration);
	XML_SetElementHandler(p, zlxml_handle_startelem, zlxml_handle_endelem);
	XML_SetCdataSectionHandler(p, zlxml_handle_startcdata, zlxml_handle_endcdata);
	XML_SetCharacterDataHandler(p, zlxml_handle_data);
//	XML_SetUnknownEncodingHandler(p, zlxml_handle_unknown_encoding, NULL);

	enum XML_Status status = XML_Parse(p, data->content, zlstr_length(data), 1);
	if (status==XML_STATUS_ERROR) {
		printf("Error <<%d>>\n", status);
	} else if (status==XML_STATUS_OK) {
		printf("Success <<%d>>\n", status);
	} else {
		printf("Extra<<%d>>\n", status);
	}
	zlstr_free(data);


	XML_ParserFree(p);

	return xmldoc;
}

zlXmlElem *zlxml_new_elem(zlString *name)
{
	zlXmlElem *elem = (zlXmlElem*)calloc(1, sizeof(zlXmlElem));
	if (!elem) {
		return NULL;
	}

	elem->name = name;
	elem->depth = depth;
	elem->attrs = NULL;
	elem->children = NULL;
	elem->parent = NULL;
	elem->data = zlstr_new("", 0);

	// Attributes Container
	elem->attrs = (zlXmlAttrContainer*)calloc(1, sizeof(zlXmlAttrContainer));
	if (!elem->attrs) {
		return NULL;
	}
	// Elements Container
	elem->children = (zlXmlElemContainer*)calloc(1, sizeof(zlXmlElemContainer));
	if (!elem->children) {
		return NULL;
	}

	return elem;
}

zlXmlAttr *zlxml_new_attr(zlString *key, zlString *value)
{
	zlXmlAttr *attr = (zlXmlAttr*)calloc(1, sizeof(zlXmlAttr));
	if (!attr) {
		return NULL;
	}
	attr->key = key;
	attr->value = value;
	return attr;
}

void zlxml_free(zlXml *xml)
{
	if (xml->root) {
		zlxml_free_elem(xml->root);
	}
	if (xml) {
		free(xml);
		xml = NULL;
	}
}

void zlxml_free_elem(zlXmlElem *elem)
{
	
}

void zlxml_free_attr(zlXmlAttr *attr)
{
	
}

void zlxml_dump(zlXml *xml)
{
	if (!xml) {
		return;
	}
	printf("zlXml (%p) ----------\n  standalone: %s\n  encoding:   %s\n  version:    %.1f\n  root:       %p\n",
		xml,
		xml->standalone ? "Yes" : "No",
		xml->encoding,
		xml->version,
		xml->root
	);
}
void zlxml_dump_elem(zlXmlElem *elem)
{
	if (!elem) {
		return;
	}

	printf(
		"zlXmlElem (%p) ----------\n  name:     %s\n  depth:    %zu\n  attrs:    %p\n  children: %p\n  parent:   %p\n%s\n",
		elem,
		elem->name->content,
		elem->depth,
		elem->attrs,
		elem->children,
		elem->parent,
		elem->data->content
	);
}

void zlxml_dump_attr(zlXmlAttr *attr)
{
	if (!attr) {
		return;
	}

	printf(
		"zlXmlAttr (%p) ----------\n  key:     %s\n  value:   %s\n\n",
		attr,
		attr->key->content,
		attr->value->content
	);
}

