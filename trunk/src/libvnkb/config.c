/* config.c:
 *
 * Copyright (C) 1998-2002 Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors:
 *      Nguyen Thai Ngoc Duy <pclouds@vnlinux.org>
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <glade/glade-xml.h>
#include "xvnkb.h"
#include "uksync.h"
#include "keycons.h"
#include "vnkb.h"
#include "eggcellrendererkeys.h"

#ifndef _
#define _(x) dgettext (GETTEXT_PACKAGE, x)
#define N_(x) x
#endif

void vnkb_import_param(Vnkb *vnkb,const char *name,const char *param)
{
  gboolean enable;
  char *sep;

  if (!strcmp(name,"charset")) {
    if (!strcasecmp(param,"VNI"))
      vnkb_set_charset(vnkb,VKC_VNI);
    else if (!strcasecmp(param,"UTF8"))
      vnkb_set_charset(vnkb,VKC_UTF8);
    else if (!strcasecmp(param,"TCVN3"))
      vnkb_set_charset(vnkb,VKC_TCVN);
    else if (!strcasecmp(param,"VISCII"))
      vnkb_set_charset(vnkb,VKC_VISCII);
    else if (!strcasecmp(param,"VPS"))
      vnkb_set_charset(vnkb,VKC_VPS);
    else if (!strcasecmp(param,"VIQR"))
      vnkb_set_charset(vnkb,VKC_VIQR);
    else
      fprintf(stderr,"Unknown charset %s\n",param);

    return;
  }

  if (!strcmp(name,"method")) {
    if (!strcasecmp(param,"VNI"))
      vnkb_set_method(vnkb,VKM_VNI);
    else if (!strcasecmp(param,"VIQR"))
      vnkb_set_method(vnkb,VKM_VIQR);
    else if (!strcasecmp(param,"VIQR*"))
      vnkb_set_method(vnkb,VKM_VIQR_STAR);
    else if (!strcasecmp(param,"TELEX"))
      vnkb_set_method(vnkb,VKM_TELEX);
    else if (!strcasecmp(param,"OFF"))
      vnkb_set_method(vnkb,VKM_OFF);
    else
      fprintf(stderr,"Unknown input method %s\n",param);

    return;
  }

  if (!strcmp(name,"label_mode")) {
    if (!strcasecmp(param,"default"))
      vnkb_set_label_mode(vnkb,VNKB_LABEL_DEFAULT);
    else if (!strcasecmp(param,"custom"))
      vnkb_set_label_mode(vnkb,VNKB_LABEL_CUSTOM);
    else if (!strcasecmp(param,"im"))
      vnkb_set_label_mode(vnkb,VNKB_LABEL_IM);
    else
      fprintf(stderr,"Unknown label mode %s\n",param);
    vnkb_update_label(vnkb);
    return;
  }


  if (!strcmp(name,"xvnkb_spell")) {
    if (!strcasecmp(param,"on"))
      vnkb_set_spelling(vnkb,TRUE);
    else if (!strcasecmp(param,"off"))
      vnkb_set_spelling(vnkb,FALSE);
    else
      fprintf(stderr,"Unknown value %s of setting %s\n",param,name);
    return;
  }

  if (!strcmp(name,"disable_on_exit")) {
    if (!strcasecmp(param,"on"))
      vnkb->disable_on_exit = TRUE;
    else if (!strcasecmp(param,"off"))
      vnkb->disable_on_exit = FALSE;
    else
      fprintf(stderr,"Unknown value %s of setting %s\n",param,name);
    return;
  }

  if (!strncmp(name,"enabled_",strlen("enabled_")))
    enable = TRUE;
  else if (!strncmp(name,"disabled_",strlen("disabled_")))
    enable = FALSE;
  else {
    fprintf(stderr,"Unknown parameter %s\n",name);
    return;
  }
  sep = strchr(name,'_');
  sep ++;
  if (!strcmp(sep,"text")) {
    if (enable)
      vnkb->text_enabled = g_strdup(param);
    else
      vnkb->text_disabled = g_strdup(param);
    vnkb_update_label(vnkb);
    return;
  }
  if (!strcmp(sep,"color")) {
    unsigned int r,g,b;
    GdkColor *color;
    sscanf(param,"%x:%x:%x",&r,&g,&b);
    if (enable)
      color = &vnkb->color_enabled;
    else
      color = &vnkb->color_disabled;

    color->red = r;
    color->green = g;
    color->blue = b;
    vnkb_update_label(vnkb);
    return;
  }
  if (!strcmp(sep,"font")) {
    if (enable)
      vnkb->font_enabled = g_strdup(param);
    else
      vnkb->font_disabled = g_strdup(param);
    vnkb_update_label(vnkb);
    return;
  }
}

void vnkb_load_config(Vnkb *vnkb)
{
  char *home = getenv("HOME");
  char *filename;

  if (home) {
    filename = g_strdup_printf("%s/.vnkb-applet.conf",home);
    if (filename) {
      FILE *fp = fopen(filename,"r");
      if (fp) {
	char buffer[256];
	char *sep;
	char *param;
	int n = 0;

	while (fgets(buffer,sizeof(buffer),fp)) {
	  int l = strlen(buffer);
	  n ++;
	  if (l && buffer[l-1] != '\n') {
	    fprintf(stderr,"Error: line %d in file %s too long\n",n,filename);
	    continue;
	  }

	  if (!l)
	    continue;

	  buffer[l-1] = 0;

	  if (buffer[0] == '#')
	    continue;		/* comment */

	  sep = strchr(buffer,'=');
	  if (!sep) {
	    fprintf(stderr,"Error: line %d in file %s not valid\n",n,filename);
	    continue;
	  }

	  param = sep+1;
	  *sep = 0;

	  vnkb_import_param(vnkb,buffer,param);
	}

	fclose(fp);
      }
      g_free(filename);
    }
  }
}

void vnkb_save_config(Vnkb *vnkb)
{
  char *home = getenv("HOME");
  char *filename;

  if (home) {
    filename = g_strdup_printf("%s/.vnkb-applet.conf",home);
    if (filename) {
      FILE *fp = fopen(filename,"w");
      if (fp) {
	char *param;

	switch (vnkb->charset) {
	case VKC_UTF8: param = "UTF8"; break;
	case VKC_TCVN: param = "TCVN3"; break;
	case VKC_VNI: param = "VNI"; break;
	case VKC_VIQR: param ="VIQR"; break;
	case VKC_VISCII: param ="VISCII"; break;
	case VKC_VPS: param ="VPS"; break;
	default: param = NULL;
	}
	if (param)
	  fprintf(fp,"charset=%s\n",param);

	switch (vnkb->method) {
	case VKM_OFF: param = "OFF"; break;
	case VKM_VIQR: param = "VIQR"; break;
	case VKM_VIQR_STAR: param = "VIQR*"; break;
	case VKM_VNI: param = "VNI"; break;
	case VKM_TELEX: param = "TELEX"; break;
	default:param = NULL;
	}
	if (param)
	  fprintf(fp,"method=%s\n",param);

	switch (vnkb->label_mode) {
	case VNKB_LABEL_IM: param = "im"; break;
	case VNKB_LABEL_DEFAULT: param = "default"; break;
	case VNKB_LABEL_CUSTOM: param = "custom"; break;
	default: param = NULL;
	}
	if (param)
	  fprintf(fp,"label_mode=%s\n",param);

	if (vnkb->text_enabled)
	  fprintf(fp,"enabled_text=%s\n",vnkb->text_enabled);
	if (vnkb->text_disabled)
	  fprintf(fp,"disabled_text=%s\n",vnkb->text_disabled);

	if (vnkb->font_enabled)
	  fprintf(fp,"enabled_font=%s\n",vnkb->font_enabled);
	if (vnkb->font_disabled)
	  fprintf(fp,"disabled_font=%s\n",vnkb->font_disabled);

	fprintf(fp,"enabled_color=%x:%x:%x\n",
		(unsigned)vnkb->color_enabled.red,
		(unsigned)vnkb->color_enabled.green,
		(unsigned)vnkb->color_enabled.blue);

	fprintf(fp,"disabled_color=%x:%x:%x\n",
		(unsigned)vnkb->color_disabled.red,
		(unsigned)vnkb->color_disabled.green,
		(unsigned)vnkb->color_disabled.blue);


	fprintf(fp,"disable_on_exit=%s\n",vnkb->disable_on_exit ? "On" : "Off");
	fprintf(fp,"xvnkb_spell=%s\n",vnkb->spelling ? "On" : "Off");

	fclose(fp);
      }
    }
    g_free(filename);
  }
}
