/*
 * Copyright 2011 Nicolas Aguirre <aguirre.nicolas@gmail.com>
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, version 2.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Public Licence with this file;
 * if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <Azy.h>
#include <Elementary.h>

static Evas_Object *win, *bg, *gl, *bx;
static Elm_Genlist_Item_Class itc1;
static Elm_Genlist_Item_Class itc_group;
static Elm_Genlist_Item_Class itc_folder;
static Elm_Gengrid_Item_Class grid_itc;

typedef struct _Rss_Item Rss_Item;

struct _Rss_Item
{
   const char *image;
   const char *title;
   const char *description;
   Elm_Gengrid_Item *git;
};

typedef struct _Rss_Ressource Rss_Ressource;

struct _Rss_Ressource
{
   const char *host;
   const char *uri;
};

static Rss_Ressource rss_ressources[] = {
  {"http://www.engadget.com", "/rss.xml"},
  {"http://feeds.arstechnica.com", "/arstechnica/index/"},
  {"http://feeds2.feedburner.com", "/LeJournalduGeek"},
  {"http://www.linuxdevices.com", "/backend/headlines.rss"},
  {"http://www.lemonde.fr", "/rss/une.xml"},
  {"http://rss.feedsportal.com", "/c/499/f/413823/index.rss"},
  NULL
};

static Eina_Error
ret_(void *data , int type , Azy_Content *content)
{
   Azy_Rss *ret;
   Azy_Rss_Item *it;
   Evas_Object *gl = data;
   Elm_Genlist_Item *gli, *gli_group, *gli_folder;
   Eina_List *l;
   int i;

   printf("RET\n");

   if (azy_content_error_is_set(content))
     {
        printf("Error encountered: %s\n", azy_content_error_message_get(content));
        return azy_content_error_code_get(content);
     }

   ret = azy_content_return_get(content);

   if(!ret)
     return 0;

   gli_group = elm_genlist_item_append(gl, &itc_group,
				       ret,
				       NULL,
				       ELM_GENLIST_ITEM_GROUP,
				       NULL,
				       NULL);
   elm_genlist_item_display_only_set(gli_group, EINA_TRUE);
   
   gli = elm_genlist_item_append(gl, &itc1,
				 ret,
				 gli_group/* parent */,
				 ELM_GENLIST_ITEM_NONE,
				 NULL/* func */,
				 it);
   return AZY_ERROR_NONE;
}

static Eina_Bool
disconnected(void *data , int type , Azy_Client *ev)
{
   printf("%s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
connected(void *data , int type , Azy_Client *cli)
{
   Azy_Client_Call_Id id;

   printf("Connected %p\n", cli);

   if (!azy_client_current(cli))
     {
        id = azy_client_blank(cli, AZY_NET_TYPE_GET, NULL, NULL, NULL);
        EINA_SAFETY_ON_TRUE_RETURN_VAL(!id, ECORE_CALLBACK_CANCEL);
        //azy_client_callback_free_set(cli, id, (Ecore_Cb)azy_rss_free);
     }
   else
     {
	printf("Nor current cli?!\n");
     }

   return ECORE_CALLBACK_RENEW;
}


static void
_gl_selected(void *data , Evas_Object *obj , void *event_info)
{
   printf("selected: %p\n", event_info);
}

static void
_gl_clicked(void *data , Evas_Object *obj , void *event_info)
{
   printf("clicked: %p\n", event_info);
}

static void
_gl_longpress(void *data , Evas_Object *obj , void *event_info)
{
   printf("longpress %p\n", event_info);
}

static Elm_Genlist_Item_Class itc1;
static Elm_Genlist_Item_Class itc1_group;

char *gl_label_get(void *data, Evas_Object *obj , const char *part )
{
   return NULL;
}


char *gl_group_label_get(void *data, Evas_Object *obj , const char *part )
{
   const char *title;
   char *buf;
   Azy_Rss *rss = data;

   title = azy_rss_title_get(rss);
   if (title)
     return strdup(title);
   else
     return NULL;
}

char *gl_folder_label_get(void *data, Evas_Object *obj , const char *part )
{
   char buf[256];
   snprintf(buf, sizeof(buf), "Folder #%i",  (int)(long)data);
   return strdup(buf);
}


static char *
_label_get(void *data, Evas_Object *obj, const char *part)
{
   char tmp[4096];
   Rss_Item *it = data;

   if (it->title)
     {
	snprintf(tmp, sizeof(tmp), "<b><h1>%s</b></h1>", it->title);
	return strdup(tmp);
     }
   else
     return NULL;
}

static Evas_Object *
_icon_get(void *data, Evas_Object *obj, const char *part)
{
   Evas_Object *ic= NULL;
   Rss_Item *it = data;

   if (strcmp(part, "elm.swallow.icon"))
     return NULL;

   if (it->image)
     {
	ic = elm_icon_add(obj);
	elm_icon_file_set(ic, it->image, NULL);
	elm_icon_fill_outside_set(ic, EINA_TRUE);
	evas_object_show(ic);
     }

   return ic;
}

int _is_html_tag (char *start)
{
   char *html_tags[] = { "html", "body", "s", "b", "u", "i", "a", "strike",
			 "font", "br", "sub", "div", "img", "p", "h6", "em", NULL };
   int i;

   for (i = 0; html_tags[i]; i++)
     {
	if (*start == '/')
	  start++;
	if (!strncasecmp(html_tags[i], start, strlen(html_tags[i])) &&
	    !isalpha(*(start + strlen(html_tags[i]))))
	  return 1;
     }

   return 0;
}

int _is_html_img_tag(char *start)
{
   if (*start == '/')
     start++;
   if (!strncasecmp("img", start, strlen("img")) &&
       !isalpha(*(start + strlen("img"))))
     return 1;

   return 0;
}

/* stollen from nexus project : http://sourceforge.net/projects/nexus/ */
static const char *
_strip_html (char *msg)
{
   char *p, *n, *tkey, *tvalue;
   char **tmp, *conv[] = {
     "quot",	"\"",
     "amp",	"&",
     "lt",	"<",
     "gt",	">",
     "nbsp", " ",
     "ouml", "ö",
     NULL
   };

   const char *img_link = NULL;

   /* Remove all known HTML tags */
   for (p = msg; *p; p++)
     {
	while (*p == '<' && _is_html_tag(p + 1) &&
	       (n = strchr(p, '>')))
	  {
	     if (!img_link && _is_html_img_tag(p + 1))
	       {
		  char *a, *b;
		  char *buf = strndup(p + 1, n - p);
		  a = strstr(buf, "src=\"");
		  if (a)
		    {
		       a+= strlen("src=\"");
		       b = strstr(a, "\"");
		       if (b)
			 {
			    img_link = eina_stringshare_add_length(a, b-a);
			    free(buf);
			 }
		    }
	       }
	     /* Replace <br> with \n */
	     if (!strncasecmp(p, "<br>", strlen("<br>")))
	       {
		  *p = '\n';
		  strcpy(p + 1, n + 1);
		  break;
	       }
	     strcpy(p, n + 1);
	  }
     }

   /* /\* Change all &*; to charaters we can use *\/ */
   /* for (p = msg; *p; p++) */
   /*   { */
   /* 	char *key; */

   /* 	/\* Check to see if we start with a '&', end with a ';', and they are */
   /* 	 * within a reasonable distance *\/ */
   /* 	if (*p == '&' && (n = strchr(p, ';')) && p - n < 6) */
   /* 	  { */
   /* 	     key = p + 1; */
   /* 	     *n = 0; */

   /* 	     for (i = 0; conv[i]; i++) */
   /* 	       { */
   /* 		  if (!strcmp(key, conv[i])) */
   /* 		    { */
   /* 		       *p = *conv[i]; */
   /* 		    } */

   /* 	       } */
   /* 	     /\* Look for this key in our table *\/ */
   /* 	     for(tmp = conv; *tmp; tkey = *tmp++, tvalue = *tmp++) */
   /* 	       { */
   /* 		  if(!strcmp(key, tkey)) */
   /* 		    { */
   /* 		       *p = *tvalue; */
   /* 		       strcpy(p + 1, n + 1); */
   /* 		       break; */
   /* 		    } */
   /* 	       } */
   /* 	  } */
   /*   } */
   return img_link;
}


static const char *
_find_http_image(char *txt)
{
   const char *http_img;

   //printf("%s\n", txt);
   http_img = _strip_html(txt);

   return http_img;
}

static void
_http_img_dl_cb(void *data, const char *file, int status)
{
   Elm_Gengrid_Item *git;

   elm_gengrid_item_update(git);
}

Evas_Object *gl_icon_get(void *data , Evas_Object *obj, const char *part)
{
   char buf[PATH_MAX];
   Evas_Object *ic;
   Azy_Rss *rss = data;
   Eina_List *l;
   Azy_Rss_Item *it;
   Elm_Gengrid_Item *git;
   int i = 0;

   if (strcmp("elm.swallow.icon", part))
     return NULL;

   ic = elm_gengrid_add(obj);
   elm_gengrid_item_size_set(ic, 160, 160);
   elm_gengrid_multi_select_set(ic, EINA_FALSE);
   elm_gengrid_bounce_set(ic, EINA_TRUE, EINA_FALSE);
   evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ic, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_gengrid_horizontal_set(ic, EINA_TRUE);

   grid_itc.item_style     = "default";
   grid_itc.func.label_get = _label_get;
   grid_itc.func.icon_get  = _icon_get;
   grid_itc.func.state_get = NULL;
   grid_itc.func.del       = NULL;

   EINA_LIST_FOREACH(azy_rss_items_get(rss), l, it)
     {
	const char *http_image;
	Rss_Item *rss_item;
	char *tmp;

	rss_item = calloc(1, sizeof(Rss_Item));

	rss_item->git = elm_gengrid_item_append(ic, &grid_itc, rss_item, NULL, NULL);
	tmp = strdup(azy_rss_item_desc_get(it));
	http_image = _find_http_image(tmp);
	rss_item->description = eina_stringshare_add(tmp);
	rss_item->title = azy_rss_item_title_get(it);
	free(tmp);

	if (http_image)
	  {
	     char dir[4096];
	     snprintf(dir, sizeof(dir), "%s/%s", efreet_cache_home_get(), azy_rss_title_get(rss));
	     ecore_file_mkdir(dir);
	     rss_item->image = eina_stringshare_printf("%s/%d.jpg", dir, i);
	     ecore_file_download(http_image, rss_item->image, _http_img_dl_cb, NULL, rss_item->git, NULL);
	  }

	i++;
     }

   return ic;
}
Eina_Bool gl_state_get(void *data , Evas_Object *obj , const char *part )
{
   return EINA_FALSE;
}
void gl_del(void *data , Evas_Object *obj )
{
}

static void
gl_sel(void *data, Evas_Object *obj, void *event_info)
{

}

int
main(int argc, char **argv)
{
   Azy_Client *cli;
   Elm_Theme *theme;
   int i;

   eina_init();
   ecore_init();
   azy_init();

   //eina_log_domain_level_set("azy", EINA_LOG_LEVEL_DBG);
   //eina_log_domain_level_set("ecore_con", EINA_LOG_LEVEL_DBG);

   elm_init(argc, argv);

   theme = elm_theme_new();
   elm_theme_overlay_add(theme, "./default.edj");


   win = elm_win_add(NULL, "RSS Reader", ELM_WIN_BASIC);
   elm_win_title_set(win, "Genlist");
   elm_win_autodel_set(win, 1);

   elm_object_theme_set(win, theme);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   gl = elm_genlist_add(win);
   evas_object_smart_callback_add(gl, "selected", _gl_selected, NULL);
   evas_object_smart_callback_add(gl, "clicked", _gl_clicked, NULL);
   evas_object_smart_callback_add(gl, "longpressed", _gl_longpress, NULL);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, gl);
   evas_object_show(gl);

   itc1.item_style     = "default";
   itc1.func.label_get = gl_label_get;
   itc1.func.icon_get  = gl_icon_get;
   itc1.func.state_get = gl_state_get;
   itc1.func.del       = gl_del;

   itc_folder.item_style     = "default";
   itc_folder.func.label_get = gl_folder_label_get;
   itc_folder.func.icon_get  = NULL;
   itc_folder.func.state_get = NULL;
   itc_folder.func.del       = gl_del;

   itc_group.item_style     = "group_index";
   itc_group.func.label_get = gl_group_label_get;
   itc_group.func.icon_get  = NULL;
   itc_group.func.state_get = NULL;
   itc_group.func.del       = gl_del;

   for (i = 0; rss_ressources[i].host; i++)
     {
	 cli = azy_client_new();
	 printf("Add cli %p\n", cli);
	 azy_client_host_set(cli,  rss_ressources[i].host, 80);
	 azy_client_connect(cli, EINA_FALSE);
	 azy_net_uri_set(azy_client_net_get(cli), rss_ressources[i].uri );
	 azy_net_version_set(azy_client_net_get(cli), 0);
     }

   ecore_event_handler_add(AZY_CLIENT_CONNECTED, (Ecore_Event_Handler_Cb)connected, NULL);
   ecore_event_handler_add(AZY_CLIENT_RETURN, (Ecore_Event_Handler_Cb)ret_, gl);
   ecore_event_handler_add(AZY_CLIENT_DISCONNECTED, (Ecore_Event_Handler_Cb)disconnected, NULL);

   evas_object_resize(win, 480, 800);
   evas_object_show(win);

   elm_run();

   azy_client_free(cli);
   elm_shutdown();
   azy_shutdown();
   ecore_shutdown();
   eina_shutdown();

   return 0;
}

