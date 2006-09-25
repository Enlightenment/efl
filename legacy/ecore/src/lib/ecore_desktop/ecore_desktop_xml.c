/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#define MIN(a, b) ((a) > (b)) ? (b) : (a)

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/mman.h>

#include <ctype.h>

#include "Ecore_Desktop.h"
#include "ecore_desktop_private.h"

static void ecore_desktop_xml_dump(Ecore_Desktop_Xml *xml, int level);
static Ecore_Desktop_Xml *ecore_desktop_xml_parse(char **data, int *size);
static int  ecore_desktop_xml_tag_parse(char **data, int *size, char **tag);
static void ecore_desktop_xml_attributes_parse(char **data, int *size, Ecore_Desktop_Xml_Attribute ***attributes);
static void ecore_desktop_xml_text_parse(char **data, int *size, char **text);
static int  ecore_desktop_xml_tag_empty(char **data, int *size);
static int  ecore_desktop_xml_tag_close(char **data, int *size, char *tag);

static int error = 0;

Ecore_Desktop_Xml *
ecore_desktop_xml_new(const char *file)
{
   Ecore_Desktop_Xml *xml = NULL;
   int size, fd = -1;
   char *data = (void *)-1;

   if (!file) return NULL;
   size = ecore_file_size(file);
   if (size <= 0) goto error;
   fd = open(file, O_RDONLY);
   if (fd == -1) goto error;
   data = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
   if (data == (void *)-1) goto error;

   error = 0;
   xml = ecore_desktop_xml_parse(&data, &size);
   if (error) goto error;
   ecore_desktop_xml_dump(xml, 0);

   munmap(data, size);
   close(fd);
   return xml;

error:
   fprintf(stderr, "[ecore_desktop]: could not parse xml file\n");
   if (data != (void *)-1) munmap(data, size);
   if (fd != -1) close(fd);
   if (xml) ecore_desktop_xml_del(xml);
   return NULL;
}

void
ecore_desktop_xml_del(Ecore_Desktop_Xml *xml)
{
   if (!xml) return;
   if (xml->children)
     {
	int i;
	for (i = 0; i < xml->children_count; i++)
	  {
	     ecore_desktop_xml_del(xml->children[i]);
	  }
	free(xml->children);
     }
   if (xml->tag) free(xml->tag);
   if (xml->attributes)
     {
	Ecore_Desktop_Xml_Attribute **curr;
	curr = xml->attributes;
	while (*curr)
	  {
	     free((*curr)->key);
	     free((*curr)->value);
	     free(*curr);
	     curr++;
	  }
	free(xml->attributes);
     }
   if (xml->text) free(xml->text);
   free(xml);
}

static void
ecore_desktop_xml_dump(Ecore_Desktop_Xml *xml, int level)
{
   int i;

   for (i = 0; i < level; i++) printf("\t");
   printf("<%s", xml->tag);
   if (xml->attributes)
     {
	Ecore_Desktop_Xml_Attribute **curr;
	for (curr = xml->attributes; *curr; curr++)
	  {
	     printf(" %s=\"%s\"", (*curr)->key, (*curr)->value);
	  }
     }
   if (xml->children)
     {
	printf(">\n");
	for (i = 0; i < xml->children_count; i++)
	  {
	     ecore_desktop_xml_dump(xml->children[i], level + 1);
	  }
	for (i = 0; i < level; i++) printf("\t");
	printf("</%s>\n", xml->tag);
     }
   else if (xml->text)
     {
	printf(">%s</%s>\n", xml->text, xml->tag);
     }
   else
     {
	printf("/>\n");
     }
}

static Ecore_Desktop_Xml *
ecore_desktop_xml_parse(char **data, int *size)
{
   Ecore_Desktop_Xml *xml, *sub_xml;
   char *tag = NULL;

   /* parse this tag */
   if (!ecore_desktop_xml_tag_parse(data, size, &(tag))) return NULL;
   xml = E_NEW(Ecore_Desktop_Xml, 1);
   if (!xml)
     {
	free(tag);
	return NULL;
     }
   xml->tag = tag;
   ecore_desktop_xml_attributes_parse(data, size, &(xml->attributes));
   /* Check wether element is empty */
   if (ecore_desktop_xml_tag_empty(data, size))
     {
	xml->done = 1;
       	return xml;
     }
   ecore_desktop_xml_text_parse(data, size, &(xml->text));
   /* Check wether element is closed */
   if (ecore_desktop_xml_tag_close(data, size, xml->tag))
     {
	xml->done = 1;
       	return xml;
     }

   while ((sub_xml = ecore_desktop_xml_parse(data, size)))
     {
	if (xml->children_count + 1 >= xml->children_size)
	  {
	     xml->children = realloc(xml->children,
				     (xml->children_size + 10 ) * sizeof(Ecore_Desktop_Xml));
	     if (!xml->children)
	       {
		  ecore_desktop_xml_del(xml);
		  return NULL;
	       }
	     memset(xml->children + xml->children_size, 0, 10);
	     xml->children_size += 10;
	  }
	xml->children[(xml->children_count)++] = sub_xml;
	if (error)
	  {
	     ecore_desktop_xml_del(xml);
	     return NULL;
	  }
     }
   ecore_desktop_xml_tag_close(data, size, xml->tag);

   return xml;
}

int
ecore_desktop_xml_tag_parse(char **data, int *size, char **tag)
{
   const char *start = NULL, *end = NULL;
   int buf_size;

   /* Search for tag */
   while (*size > 1)
     {
	/* Check for tag start */
	if (**data == '<')
	  {
	     /* Check for end tag */
	     if (*(*data + 1) == '/') return 0;
	     /* Check for comment or ... */
	     else if ((*(*data + 1) != '!') && (*(*data + 1) != '?'))
	       {
		  (*size)--;
		  (*data)++;
		  start = *data;
		  break;
	       }
	  }
	(*size)--;
	(*data)++;
     }
   if (!start)
     {
	fprintf(stderr, "[ecore_desktop]: missing start tag\n");
	error = 1;
	return 0;
     }
   while (*size > 0)
     {
	if (!isalpha(**data))
	  {
	     end = *data;
	     break;
	  }
	(*size)--;
	(*data)++;
     }
   if (!end)
     {
	fprintf(stderr, "[ecore_desktop]: no end of tag\n");
	error = 1;
       	return 0;
     }
   buf_size = end - start + 1;
   if (buf_size <= 1)
     {
	fprintf(stderr, "[ecore_desktop]: no tag name\n");
	error = 1;
       	return 0;
     }
   *tag = malloc(buf_size);
   memcpy(*tag, start, buf_size - 1);
   (*tag)[buf_size - 1] = 0;
   return 1;
}

static void
ecore_desktop_xml_attributes_parse(char **data, int *size,
				   Ecore_Desktop_Xml_Attribute ***attributes)
{
   Ecore_Desktop_Xml_Attribute attr[10];
   int i, count = 0;

   while (*size > 0)
     {
	if (**data == '>')
	  {
	     (*size)++;
	     (*data)--;
	     break;
	  }
	else if ((count < 10) && (isalpha(**data)))
	  {
	     /* beginning of key */
	     const char *start = NULL, *end = NULL;
	     int buf_size;

	     attr[count].key = NULL;
	     attr[count].value = NULL;

	     start = *data;
	     while ((*size > 0) && ((isalpha(**data)) || (**data == '_')))
	       {
		  (*size)--;
		  (*data)++;
	       }
	     end = *data;
	     buf_size = end - start + 1;
	     attr[count].key = malloc(buf_size);
	     memcpy(attr[count].key, start, buf_size - 1);
	     attr[count].key[buf_size - 1] = 0;

	     /* search for '=', key/value seperator */
	     start = NULL;
	     while (*size > 0)
	       {
		  if (**data == '=')
		    {
		       start = *data;
		       break;
		    }
		  (*size)--;
		  (*data)++;
	       }
	     if (!start)
	       {
		  fprintf(stderr, "[ecore_desktop]: missing value for attribute!\n");
		  error = 1;
		  while (count >= 0)
		    {
		       if (attr[count].key) free(attr[count].key);
		       if (attr[count].value) free(attr[count].value);
		       count--;
		    }
		  return;
	       }
	     /* search for '"', beginning of value */
	     start = NULL;
	     while (*size > 0)
	       {
		  if (**data == '"')
		    {
		       start = *data;
		       break;
		    }
		  (*size)--;
		  (*data)++;
	       }
	     if (!start)
	       {
		  fprintf(stderr, "[ecore_desktop]: erroneous value for attribute!\n");
		  error = 1;
		  while (count >= 0)
		    {
		       if (attr[count].key) free(attr[count].key);
		       if (attr[count].value) free(attr[count].value);
		       count--;
		    }
		  return;
	       }
	     /* skip '"' */
	     start++;
	     (*size)--;
	     (*data)++;
	     /* search for '"', end of value */
	     end = NULL;
	     while (*size > 0)
	       {
		  if (**data == '"')
		    {
		       end = *data;
		       break;
		    }
		  (*size)--;
		  (*data)++;
	       }
	     if (!end)
	       {
		  fprintf(stderr, "[ecore_desktop]: erroneous value for attribute!\n");
		  error = 1;
		  while (count >= 0)
		    {
		       if (attr[count].key) free(attr[count].key);
		       if (attr[count].value) free(attr[count].value);
		       count--;
		    }
		  return;
	       }

	     buf_size = end - start + 1;
	     attr[count].value = malloc(buf_size);
	     memcpy(attr[count].value, start, buf_size - 1);
	     attr[count].value[buf_size - 1] = 0;

	     count++;
	  }

	(*size)--;
	(*data)++;
     }
   *attributes = E_NEW(Ecore_Desktop_Xml_Attribute *, count + 1);
   for (i = 0; i < count; i++)
     {
	(*attributes)[i] = malloc(sizeof(Ecore_Desktop_Xml_Attribute));
	(*attributes)[i]->key = attr[i].key;
	(*attributes)[i]->value = attr[i].value;
     }
}

static void
ecore_desktop_xml_text_parse(char **data, int *size, char **text)
{
   const char *start = NULL, *end = NULL;
   int buf_size;
   /* skip leading whitespace */
   while (*size > 0)
     {
	if (!isspace(**data))
	  {
	    start = *data;
	    break;
	  }
	(*size)--;
	(*data)++;
     }
   if (!start) return;
   /* find next tag */
   while (*size > 0)
     {
	if (**data == '<')
	  {
	    end = *data;
	    break;
	  }
	(*size)--;
	(*data)++;
     }
   if (!end) return;
   /* skip trailing whitespace */
   while (isspace(*(end - 1))) end--;
   /* copy text */
   buf_size = end - start + 1;
   if (buf_size <= 1) return;
   *text = malloc(buf_size);
   memcpy(*text, start, buf_size - 1);
   (*text)[buf_size - 1] = 0;
}

static int
ecore_desktop_xml_tag_empty(char **data, int *size)
{
   while (*size > 1)
     {
	if (**data == '/')
	  {
	     (*size)--;
	     (*data)++;
	     if (**data == '>')
	       {
		  (*size)--;
		  (*data)++;
		  return 1;
	       }
	  }
	else if (**data == '>')
	  {
	     (*size)--;
	     (*data)++;
	     return 0;
	  }
	(*size)--;
	(*data)++;
     }
   fprintf(stderr, "[ecore_desktop]: missing end of tag\n");
   error = 1;
   return 1;
}

static int
ecore_desktop_xml_tag_close(char **data, int *size, char *tag)
{
   while (*size > 1)
     {
	if (**data == '<')
	  {
	     if (*(*data + 1) == '/')
	       {
		  (*size) -= 2;
		  (*data) += 2;
		  if (strlen(tag) > *size)
		    {
		       fprintf(stderr, "[ecore_desktop]: wrong end tag\n");
		       error = 1;
		       return 1;
		    }
		  else
		    {
		       char *tmp;
		       tmp = *data;
		       while ((*tag) && (*tmp == *tag))
			 {
			    tmp++;
			    tag++;
			 }
		       if (*tag)
			 {
			    fprintf(stderr, "[ecore_desktop]: wrong end tag\n");
			    error = 1;
			    return 1;
			 }
		    }
		  return 1;
	       }
	     else return 0;
	  }
	(*size)--;
	(*data)++;
     }
   return 0;
}
