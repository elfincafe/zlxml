#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "zlstring.h"
#include "zlxml.h"


zlXml *zlxml_new()
{
	zlXml *xml;
	xml = (zlXml*)calloc(1, sizeof(zlXml));
	xml->standalone = false;
	xml->encoding = zlstr_new("", 0);
	xml->version = zlstr_new("", 0);
	return xml;
}

zlXml *zlxml_load(zlString *path)
{
	struct stat st;
	if (stat(path->content, &st)!=0) {
		return NULL;
	}

	FILE *fp;
	if ((fp=fopen(path->content, "rb")==NULL) {
	}

	char buf[st.st_size+1];
	fread(buf, 1, st.st_size, fp);
	buf[st.st_size] = '\0';
printf("[%s]\n", buf);
return NULL;
}

zlXml *zlxml_parse(zlString *data)
{
	zlXml *xml = NULL;
	return xml;
}

zlXmlElem *zlxml_new_elem(zlString *tag);
zlXmlAttr *zlxml_new_attr(zlString *key, zlString *value);

void zlxml_free(zlXml *xml)
{
	if (xml->encoding) {
		zlstr_free(xml->encoding);
	}
	if (xml->version) {
		zlstr_free(xml->version);
	}
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
	printf("zlXml ----------\n  standalone: %s\n  encoding:   %s\n  version:    %s\n  root:       %p\n",
		xml->standalone ? "true" : "false",
		xml->encoding->content,
		xml->version->content,
		xml->root
	);
}
void zlxml_dump_elem(zlXmlElem *elem);
void zlxml_dump_attr(zlXmlAttr *attr);
