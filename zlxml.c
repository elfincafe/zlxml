#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "expat.h"
#include "zlstring.h"
#include "zlxml.h"

static bool zlxml_add_attr(zlXmlElem *elem, zlXmlAttr *attr)
{
	size_t msize = 3;
	size_t size = elem->attr_size;
	size_t count = elem->attr_count;

	// Reallocation
	if (size<=count) {
		zlXmlAttr **tmp = NULL;
		tmp = (zlXmlAttr**)realloc(elem->attrs, sizeof(zlXmlAttr*)*(size+msize));
		if (tmp==NULL) {
			return false;
		}
		elem->attrs = tmp;
		elem->attr_size += msize;
	}

	elem->attrs[count] = attr;
	elem->attr_count++;

	return true;
}

static bool zlxml_add_child (zlXmlElem *child)
{
	zlXmlElem *parent = child->parent;
	if (parent==NULL) {
		return false;
	}
	size_t msize = 5;
	size_t count = parent->child_count;
	size_t size = parent->child_size;

	// Reallocation
	if (size<count+1) {
		zlXmlElem **tmp = NULL;
		tmp = (zlXmlElem**)realloc(parent->children, sizeof(zlXmlElem*)*(size+msize));
		if (tmp==NULL){ 
			return false;
		}
		parent->children = tmp;
		parent->child_size += msize;
	}
	parent->children[count] = child;
	parent->child_count++;

	return true;
}

static void XMLCALL zlxml_handle_declaration(void *worker, const XML_Char *version, const XML_Char *encoding, int standalone)
{
	zlXmlWorker *w = (zlXmlWorker*)worker;

	w->doc = zlxml_new();
	if (version) {
		w->doc->version = strtof(version, NULL);
	}
	if (encoding) {
		strncpy(w->doc->encoding, encoding, strlen(encoding));
	}
	if (standalone) {
		w->doc->standalone = (bool)standalone;
	}
}

static void XMLCALL zlxml_handle_startelem(void *worker, const XML_Char *name, const XML_Char **attrs)
{
	zlXmlWorker *w = (zlXmlWorker*)worker;

	w->depth++;

	zlXmlElem *elem = zlxml_new_elem((char*)name, strlen(name));
	elem->depth = w->depth;
	if (w->doc->root==NULL) {
		w->doc->root = elem;
	}
	w->cur = elem;

	// Parent
	zlXmlElem *prev;
	prev = w->prev;
	while (prev) {
		if (prev->depth==elem->depth-1) {
			w->parent = prev;
			break;
		}
		prev = prev->parent;
	}
	elem->parent = w->parent;
	zlxml_add_child(elem);

	// Attributes
	size_t i = 0;
	while (attrs[i]) {
		zlXmlAttr *attr = zlxml_new_attr((char*)attrs[i], strlen(attrs[i]), (char*)attrs[i+1], strlen(attrs[i+1]));
		zlxml_add_attr(elem, attr);
		i += 2;
	}

	// Post Processing
	w->prev = w->cur;
}

static void XMLCALL zlxml_handle_endelem(void *worker, const char *name)
{
	zlXmlWorker *w = (zlXmlWorker*)worker;
	w->depth--;
}

static void XMLCALL zlxml_handle_data(void *worker, const XML_Char *s, int len)
{
	zlXmlWorker *w = (zlXmlWorker*)worker;

	w->cur->dat_len = len;
	char *tmp = (char*)calloc(1, len+1);
	if (tmp==NULL) {
		return;
	}
	w->cur->data = tmp;
	strncpy(w->cur->data, s, len);
}

static void XMLCALL zlxml_handle_startcdata(void *worker)
{
	
}

static void XMLCALL zlxml_handle_endcdata(void *worker)
{
	
}

/*
static void XMLCALL zlxml_handle_unknown_encoding (void *encodingHandlerData, const XML_Char *name, XML_Encoding *info)
{
}
*/

zlXml *zlxml_new()
{
	zlXml *xml = (zlXml*)calloc(1, sizeof(zlXml));
	if (!xml) {
		return NULL;
	}
	xml->standalone = true;
	strncpy(xml->encoding, "UTF-8", 5);
	xml->version = 1.0;
	xml->root = NULL;

	return xml;
}

zlXml *zlxml_load(char *path)
{
	struct stat st;
	if (stat(path, &st)!=0) {
		return NULL;
	}

	FILE *fp;
	if ((fp=fopen(path, "rb"))==NULL) {
		return NULL;
	}

	char *buf = NULL;
	buf = (char*)calloc(1, st.st_size+1);
	if (buf==NULL) {
		return NULL;
	}
	fread(buf, 1, st.st_size, fp);
	buf[st.st_size] = '\0';
	fclose(fp);

	zlXml *xml = zlxml_parse(buf, st.st_size);
	if (buf!=NULL) {
		free(buf);
		buf = NULL;
	}
	return xml;
}

zlXml *zlxml_parse(char *data, size_t len)
{

	XML_Parser p = XML_ParserCreate(NULL);
	XML_SetXmlDeclHandler(p, zlxml_handle_declaration);
	XML_SetElementHandler(p, zlxml_handle_startelem, zlxml_handle_endelem);
	XML_SetCdataSectionHandler(p, zlxml_handle_startcdata, zlxml_handle_endcdata);
	XML_SetCharacterDataHandler(p, zlxml_handle_data);
//	XML_SetUnknownEncodingHandler(p, zlxml_handle_unknown_encoding, NULL);

	// worker
	zlXmlWorker *worker = (zlXmlWorker*)calloc(1, sizeof(zlXmlWorker));
	if (worker==NULL) {
		return NULL;
	}
	worker->depth = 0;
	worker->doc = NULL;
	worker->cur = NULL;
	worker->prev = NULL;
	worker->parent = NULL;
	XML_SetUserData(p, worker);

	enum XML_Status status = XML_Parse(p, data, len, 1);
	if (status==XML_STATUS_ERROR) {
		printf("Error <<%d>>\n", status);
	} else if (status==XML_STATUS_OK) {
		printf("Success <<%d>>\n", status);
	} else {
		printf("Extra<<%d>>\n", status);
	}

	XML_ParserFree(p);
	
	return worker->doc;
}

zlXmlElem *zlxml_new_elem (char *name, size_t len)
{
	zlXmlElem *elem = (zlXmlElem*)calloc(1, sizeof(zlXmlElem));
	if (!elem) {
		return NULL;
	}

	elem->name = (char*)calloc(1, len+1);
	if (elem->name==NULL) {
		return NULL;
	}
	strncpy(elem->name, name, len);
	elem->name[len] = '\0';
	elem->depth = 0;
	elem->attr_size = 0;
	elem->attr_count = 0;
	elem->attrs = NULL;
	elem->child_size = 0;
	elem->child_count = 0;
	elem->children = NULL;
	elem->parent = NULL;
	elem->dat_len = 0;
	elem->data = NULL;

	return elem;
}

zlXmlAttr *zlxml_new_attr (char *key, size_t len_k, char *value, size_t len_v)
{
	zlXmlAttr *attr = (zlXmlAttr*)calloc(1, sizeof(zlXmlAttr));
	if (!attr) {
		return NULL;
	}
	attr->key = (char*)calloc(1, len_k+1);
	if (attr->key==NULL) {
		return NULL;
	}
	strncpy(attr->key, key, len_k);
	attr->key[len_k] = '\0';
	attr->value = (char*)calloc(1, len_v+1);
	if (attr->value==NULL) {
		return NULL;
	}
	strncpy(attr->value, value, len_v);
	attr->value[len_v] = '\0';

	return attr;
}

void zlxml_free(zlXml *xml)
{
	if (xml->root) {
		zlxml_free_elem(xml->root);
		xml->root = NULL;
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
		"zlXmlElem (%p) ----------\n  name:        %s\n  depth:       %zu\n  attr_size:   %zu\n  attr_count:  %zu\n  attrs:       %p\n  child_size:  %zu\n  child_count: %zu\n  children:    %p\n  parent:      %p\n  dat_len:    %zu\n  data:     %s\n",
		elem,
		elem->name,
		elem->depth,
		elem->attr_size,
		elem->attr_count,
		elem->attrs,
		elem->child_size,
		elem->child_count,
		elem->children,
		elem->parent,
		elem->dat_len,
		elem->data
	);
	size_t i;
	for (i=0; i<elem->attr_count; i++) {
		printf(
			"  zlXmlAttr (%p) ----------\n    key:       %s\n    value:     %s\n",
			elem->attrs[i],
			elem->attrs[i]->key,
			elem->attrs[i]->value
		);
	}
	for (i=0; i<elem->child_count; i++) {
		printf(
			"  zlXmlElem (%p) ----------\n    name:      %s\n",
			elem->children[i],
			elem->children[i]->name
		);
	}
	printf("\n");
}

void zlxml_dump_attr(zlXmlAttr *attr)
{
	if (!attr) {
		return;
	}

	printf(
		"zlXmlAttr (%p) ----------\n  key:     %s\n  value:   %s\n\n",
		attr,
		attr->key,
		attr->value
	);
}

