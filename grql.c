#include "main.h"

#define BUFFSIZE 256
void finish_obj();
void finish_thar();
void finish_flat();
void finish_blaze();
void finish_mask();

gint flat_ow_check;
extern gboolean debug_flg;

void check_ql_finish(typHLOG *hl){
  if(debug_flg){
    fprintf(stderr,"!!!CheckQL Finish...\n");
  }
  switch(hl->ql_loop){
  case QL_THAR:
    finish_thar(hl);
    tree_update_frame(hl);
    break;
  case QL_FLAT:
    finish_flat(hl);
    tree_update_frame(hl);
    break;
  case QL_OBJECT:
  case QL_OBJECT_BATCH:
    finish_obj(hl);
    check_reduced_spectra(hl);
    tree_update_frame(hl);
    update_seimei_log(hl, hl->ql_i);
    break;
  case QL_SPLOT:
    break;
  case QL_MASK:
    finish_mask(hl);
    break;
  case QL_BLAZE:
    finish_blaze(hl);
    break;
  } 
}


gboolean check_ql(gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;

  if(access(hl->ql_lock,F_OK)==0){
    hl->check_ql1=TRUE;
    switch(hl->ql_loop){
    case QL_OBJECT_BATCH:
      check_reduced_spectra(hl);
      break;
    }
    return(TRUE);
  }
  else{
    if(!hl->check_ql1){
      return(TRUE);
    }
    else{
      check_ql_finish(hl);
      
      hl->ql_timer=-1;
      hl->check_ql1=FALSE;
      return(FALSE);
    }
  }
}


void check_db_dir(gchar *wdir){
  gchar *db_dir, *com;
  gint sret;
  db_dir=g_strconcat(wdir,
		     G_DIR_SEPARATOR_S,
		     "database",
		     NULL);
  if(access(db_dir, F_OK)!=0){
    com=g_strconcat("mkdir ",
		    db_dir,
		    NULL);
    sret=system(com);
    g_free(com);
  }
  
  g_free(db_dir);
}
void get_flat_scnm(typHLOG *hl){
  gchar *c;
  gint ret;

  c=hl->flat_red[hl->iraf_hdsql_r];

  if(!c){
    ret=FLAT_EX_NO;
  }
  else{
    if(g_strstr_len(c,-1,".sc.nm")){
      ret=FLAT_EX_SCNM;
    }
    else if(g_strstr_len(c,-1,".sc.fl")){
      ret=FLAT_EX_SCFL;
    }
    else if(g_strstr_len(c,-1,".sc")){
      ret=FLAT_EX_SC;
    }
    else if(c){
    ret=FLAT_EX_1;
    }
    else{
      ret=FLAT_EX_NO;
    }
  }

  hl->ex_flat_red[hl->iraf_hdsql_r]=ret;
  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->combo_flat_red),
			   ret);
}

void my_file_chooser_add_filter (GtkWidget *dialog, const gchar *name, ...)
{
  GtkFileFilter *filter;
  gchar *name_tmp;
  va_list args;
  gchar *pattern, *ptncat=NULL, *ptncat2=NULL;

  filter=gtk_file_filter_new();

  va_start(args, name);
  while(1){
    pattern=va_arg(args, gchar*);
    if(!pattern) break;
    gtk_file_filter_add_pattern(filter, pattern);
    if(!ptncat){
      ptncat=g_strdup(pattern);
    }
    else{
      if(ptncat2) g_free(ptncat2);
      ptncat2=g_strdup(ptncat);
      if(ptncat) g_free(ptncat);
      ptncat=g_strconcat(ptncat2,",",pattern,NULL);
    }
  }
  va_end(args);

  name_tmp=g_strconcat(name," [",ptncat,"]",NULL);
  gtk_file_filter_set_name(filter, name_tmp);
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filter);
  if(name_tmp) g_free(name_tmp);
  if(ptncat) g_free(ptncat);
  if(ptncat2) g_free(ptncat2);
}


void set_ap_label(typHLOG *hl){
  gchar *tmp;

  if(hl->ql_ap){
    tmp=g_strdup_printf("<span font_family=\"monospace\">%s.fits</span>",
			hl->ql_ap);
  }
  else{
    tmp=g_strdup("<span color=\"#999999\"><i>(Not set)</i></span>");
  }

  gtk_label_set_markup(GTK_LABEL(hl->label_edit_ap), tmp);
  g_free(tmp);
}

void set_flat_label(typHLOG *hl){
  gchar *tmp;

  if(hl->ql_flat){
    tmp=g_strdup_printf("<span font_family=\"monospace\">%s.fits</span>",
			hl->ql_flat);
  }
  else{
    tmp=g_strdup("<span color=\"#999999\"><i>(Not set)</i></span>");
  }

  gtk_label_set_markup(GTK_LABEL(hl->label_edit_flat), tmp);
  g_free(tmp);
}

void set_thar_label(typHLOG *hl){
  gchar *tmp;

  if(hl->ql_thar1d){
    tmp=g_strdup_printf("<span font_family=\"monospace\">%s.fits</span>",
			hl->ql_thar1d);
  }
  else{
    tmp=g_strdup("<span color=\"#999999\"><i>(Not set)</i></span>");
  }

  gtk_label_set_markup(GTK_LABEL(hl->label_edit_thar), tmp);
  g_free(tmp);
}

void set_mask_label(typHLOG *hl){
  gchar *tmp;

  if(hl->ql_mask){
    tmp=g_strdup_printf("<span font_family=\"monospace\">%s.fits</span>",
			hl->ql_mask);
  }
  else{
    tmp=g_strdup("<span color=\"#999999\"><i>(Not set)</i></span>");
  }

  gtk_label_set_markup(GTK_LABEL(hl->label_edit_mask), tmp);
  g_free(tmp);
}

void set_blaze_label(typHLOG *hl){
  gchar *tmp;

  if(hl->ql_blaze){
    tmp=g_strdup_printf("<span font_family=\"monospace\">%s.fits</span>",
			hl->ql_blaze);
  }
  else{
    tmp=g_strdup("<span color=\"#999999\"><i>(Not set)</i></span>");
  }

  gtk_label_set_markup(GTK_LABEL(hl->label_edit_blaze), tmp);
  g_free(tmp);
}


gchar *get_refname(gchar *fp_file){
  gchar *bname=NULL, *ret, *c;

  bname=g_path_get_basename(fp_file);
  c=(char *)strrchr(bname,'.');
  ret=g_strndup(bname,strlen(bname)-strlen(c));
  if(bname) g_free(bname);

  return(ret);
}

gchar *get_refname_db(gchar *fp_file){
  gchar *bname=NULL, *ret, *c;

  bname=g_path_get_basename(fp_file);
  ret=g_strdup(bname+2);

  return(ret);
}

void set_setname(typHLOG *hl, gint i_sel){
  gchar *bin, *tbin, *setup;

  setup=g_strdup_printf("GAOES-RV_setup");

  if(hl->setname_red[hl->iraf_hdsql_r]) 
    g_free(hl->setname_red[hl->iraf_hdsql_r]);

  bin=g_strdup_printf("%dx%d",
		      1,1);
  hl->setname_red[hl->iraf_hdsql_r]=g_strconcat(setup,
						bin,
						NULL);
  tbin=g_strdup_printf("?x%d", 1);
  hl->tharname_red[hl->iraf_hdsql_r]=g_strconcat(setup,
						     tbin,
						     NULL);

  g_free(bin);
  g_free(tbin);
  g_free(setup);
}

void set_ql_frame_label(typHLOG *hl, GtkWidget *w, gboolean frame_flag){
  gchar *tmp;

  if(hl->ap_red[hl->iraf_hdsql_r]){
    tmp=g_strconcat(FRAME_QL_LABEL,
		    "  :  <b>",
		    hl->setname_red[hl->iraf_hdsql_r],
		    "</b>",
		    NULL);
  }
  else{
    tmp=g_strdup(FRAME_QL_LABEL);
  }

  if(frame_flag){
    gtkut_frame_set_label(w, tmp);
  }
  else{
    gtk_label_set_markup(GTK_LABEL(w), tmp);
  }

  g_free(tmp);
}

gchar *get_work_dir(typHLOG *hl){
  gchar *ret, *pdir, *dbase;
  
  pdir=g_strdup(hl->ddir);
  ret=g_strconcat(pdir,
		  G_DIR_SEPARATOR_S,
		  "ql",
		  NULL);
  g_free(pdir);
  if(access(ret, F_OK)!=0){
    mkdir(ret, (S_IRWXU|S_IRWXG|S_IRWXO));
  }

  dbase=g_strconcat(ret,
		    G_DIR_SEPARATOR_S,
		    "database",
		    NULL);
  if(access(dbase, F_OK)!=0){
    mkdir(dbase, (S_IRWXU|S_IRWXG|S_IRWXO));
  }
  g_free(dbase);

  return(ret);
}


gchar *get_share_dir(typHLOG *hl){
  gchar *ret, *homed;

  homed=g_strdup(g_get_home_dir());
  ret=g_strconcat(homed,
		  G_DIR_SEPARATOR_S,
		  SHARE_DIR,
		  NULL);
  g_free(homed);
  if(access(ret, F_OK)!=0){
    fprintf(stderr, "Cannot access to the share data directory \"%s\" .\n",
	    ret);
    exit(-1);
  }
  return(ret);
}

void check_reference_data(typHLOG *hl){
  gchar *ap_fits0, *ec_fits0, *ap_db0, *ec_db0;
  gchar *ap_fits, *ec_fits, *ap_db, *ec_db;
  gchar *fl_fits0, *fl_fits, *ec2_fits0, *ec2_fits;
  gchar *ms_fits, *ms_fits0, *bz_fits, *bz_fits0;

  // Ap
  ap_fits=g_strconcat(hl->wdir,
		      G_DIR_SEPARATOR_S,
		      GAOES_AP,
		      ".fits",
		      NULL);
  if(access(ap_fits, F_OK)!=0){
    ap_fits0=g_strconcat(hl->sdir,
			 G_DIR_SEPARATOR_S,
			 GAOES_AP,
			 ".fits",
			 NULL);
    copy_file(ap_fits0, ap_fits);
    g_free(ap_fits0);
  }
  g_free(ap_fits);

  // Ap database
  ap_db=g_strconcat(hl->wdir,
		    G_DIR_SEPARATOR_S,
		    "database",
		    G_DIR_SEPARATOR_S,
		    "ap",
		    GAOES_AP,
		    NULL);
  if(access(ap_db, F_OK)!=0){
    ap_db0=g_strconcat(hl->sdir,
		       G_DIR_SEPARATOR_S,
		       "database",
		       G_DIR_SEPARATOR_S,
		       "ap",
		       GAOES_AP,
		       NULL);
    copy_file(ap_db0, ap_db);
    g_free(ap_db0);
  }
  g_free(ap_db);

  // Flat
  fl_fits=g_strconcat(hl->wdir,
		      G_DIR_SEPARATOR_S,
		      GAOES_FLAT,
		      ".fits",
		      NULL);
  if(access(fl_fits, F_OK)!=0){
    fl_fits0=g_strconcat(hl->sdir,
			 G_DIR_SEPARATOR_S,
			 GAOES_FLAT,
			 ".fits",
			 NULL);
    copy_file(fl_fits0, fl_fits);
    g_free(fl_fits0);
  }
  g_free(fl_fits);

  // ThAr 1D
  ec_fits=g_strconcat(hl->wdir,
		      G_DIR_SEPARATOR_S,
		      GAOES_THAR1D,
		      ".fits",
		      NULL);
  if(access(ec_fits, F_OK)!=0){
    ec_fits0=g_strconcat(hl->sdir,
			 G_DIR_SEPARATOR_S,
			 GAOES_THAR1D,
			 ".fits",
			 NULL);
    copy_file(ec_fits0, ec_fits);
    g_free(ec_fits0);
  }
  g_free(ec_fits);

  // ThAr 1D datavase
  ec_db=g_strconcat(hl->wdir,
		    G_DIR_SEPARATOR_S,
		    "database",
		    G_DIR_SEPARATOR_S,
		    "ec",
		    GAOES_THAR1D,
		    NULL);
  if(access(ec_db, F_OK)!=0){
    ec_db0=g_strconcat(hl->sdir,
		       G_DIR_SEPARATOR_S,
		       "database",
		       G_DIR_SEPARATOR_S,
		       "ec",
		       GAOES_THAR1D,
		       NULL);
    copy_file(ec_db0, ec_db);
    g_free(ec_db0);
  }
  g_free(ec_db);

  // ThAr 2D
  ec2_fits=g_strconcat(hl->wdir,
		      G_DIR_SEPARATOR_S,
		      GAOES_THAR2D,
		      ".fits",
		      NULL);
  if(access(ec2_fits, F_OK)!=0){
    ec2_fits0=g_strconcat(hl->sdir,
			 G_DIR_SEPARATOR_S,
			 GAOES_THAR2D,
			 ".fits",
			 NULL);
    copy_file(ec2_fits0, ec2_fits);
    g_free(ec2_fits0);
  }
  g_free(ec2_fits);

  // Mask
  ms_fits=g_strconcat(hl->wdir,
		      G_DIR_SEPARATOR_S,
		      GAOES_MASK,
		      ".fits",
		      NULL);
  if(access(ms_fits, F_OK)!=0){
    ms_fits0=g_strconcat(hl->sdir,
			 G_DIR_SEPARATOR_S,
			 GAOES_MASK,
			 ".fits",
			 NULL);
    copy_file(ms_fits0, ms_fits);
    g_free(ms_fits0);
  }
  g_free(ms_fits);
  
  // Blaze
  bz_fits=g_strconcat(hl->wdir,
		      G_DIR_SEPARATOR_S,
		      GAOES_BLAZE,
		      ".fits",
		      NULL);
  if(access(bz_fits, F_OK)!=0){
    bz_fits0=g_strconcat(hl->sdir,
			 G_DIR_SEPARATOR_S,
			 GAOES_BLAZE,
			 ".fits",
			 NULL);
    copy_file(bz_fits0, bz_fits);
    g_free(bz_fits0);
  }
  g_free(bz_fits);
}


void prepare_pyraf(typHLOG *hl){
  gchar *dest, *src;

  // grql.py
  dest=g_strconcat(hl->wdir,
		   G_DIR_SEPARATOR_S,
		   GAOES_PY_GRQL,
		   NULL);
  if(access(dest, F_OK)!=0){
    src=g_strconcat(hl->sdir,
		    G_DIR_SEPARATOR_S,
		    GAOES_PY_GRQL,
		    NULL);
    copy_file(src, dest);
    chmod(dest, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH|S_IXUSR|S_IXGRP|S_IXOTH);
    g_free(src);
  }
  g_free(dest);
		      
  // grql_batch.py
  dest=g_strconcat(hl->wdir,
		   G_DIR_SEPARATOR_S,
		   GAOES_PY_GRQL_BATCH,
		   NULL);
  if(access(dest, F_OK)!=0){
    src=g_strconcat(hl->sdir,
		    G_DIR_SEPARATOR_S,
		    GAOES_PY_GRQL_BATCH,
		    NULL);
    copy_file(src, dest);
    chmod(dest, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH|S_IXUSR|S_IXGRP|S_IXOTH);
    g_free(src);
  }
  g_free(dest);
		      
  // gaoes_flat.py
  dest=g_strconcat(hl->wdir,
		   G_DIR_SEPARATOR_S,
		   GAOES_PY_FLAT,
		   NULL);
  if(access(dest, F_OK)!=0){
    src=g_strconcat(hl->sdir,
		    G_DIR_SEPARATOR_S,
		    GAOES_PY_FLAT,
		    NULL);
    copy_file(src, dest);
    chmod(dest, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH|S_IXUSR|S_IXGRP|S_IXOTH);
    g_free(src);
  }
  g_free(dest);
		      
  // gaoes_comp.py
  dest=g_strconcat(hl->wdir,
		   G_DIR_SEPARATOR_S,
		   GAOES_PY_COMP,
		   NULL);
  if(access(dest, F_OK)!=0){
    src=g_strconcat(hl->sdir,
		    G_DIR_SEPARATOR_S,
		    GAOES_PY_COMP,
		    NULL);
    copy_file(src, dest);
    chmod(dest, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH|S_IXUSR|S_IXGRP|S_IXOTH);
    g_free(src);
  }
  g_free(dest);
		      
  // gaoes_comp_obj.py
  dest=g_strconcat(hl->wdir,
		   G_DIR_SEPARATOR_S,
		   GAOES_PY_COMP_OBJ,
		   NULL);
  if(access(dest, F_OK)!=0){
    src=g_strconcat(hl->sdir,
		    G_DIR_SEPARATOR_S,
		    GAOES_PY_COMP_OBJ,
		    NULL);
    copy_file(src, dest);
    chmod(dest, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH|S_IXUSR|S_IXGRP|S_IXOTH);
    g_free(src);
  }
  g_free(dest);
		      
  // gaoes_mkmask.py
  dest=g_strconcat(hl->wdir,
		   G_DIR_SEPARATOR_S,
		   GAOES_PY_MASK,
		   NULL);
  if(access(dest, F_OK)!=0){
    src=g_strconcat(hl->sdir,
		    G_DIR_SEPARATOR_S,
		    GAOES_PY_MASK,
		    NULL);
    copy_file(src, dest);
    chmod(dest, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH|S_IXUSR|S_IXGRP|S_IXOTH);
    g_free(src);
  }
  g_free(dest);
		      
  // gaoes_mkblaze.py
  dest=g_strconcat(hl->wdir,
		   G_DIR_SEPARATOR_S,
		   GAOES_PY_BLAZE,
		   NULL);
  if(access(dest, F_OK)!=0){
    src=g_strconcat(hl->sdir,
		    G_DIR_SEPARATOR_S,
		    GAOES_PY_BLAZE,
		    NULL);
    copy_file(src, dest);
    chmod(dest, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH|S_IXUSR|S_IXGRP|S_IXOTH);
    g_free(src);
  }
  g_free(dest);

  // gaoes_mkref.py
  dest=g_strconcat(hl->wdir,
		   G_DIR_SEPARATOR_S,
		   GAOES_PY_MKREF,
		   NULL);
  if(access(dest, F_OK)!=0){
    src=g_strconcat(hl->sdir,
		    G_DIR_SEPARATOR_S,
		    GAOES_PY_MKREF,
		    NULL);
    copy_file(src, dest);
    chmod(dest, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH|S_IXUSR|S_IXGRP|S_IXOTH);
    g_free(src);
  }
  g_free(dest);

  // splot.py
  dest=g_strconcat(hl->wdir,
		   G_DIR_SEPARATOR_S,
		   GAOES_PY_SPLOT,
		   NULL);
  if(access(dest, F_OK)!=0){
    src=g_strconcat(hl->sdir,
		    G_DIR_SEPARATOR_S,
		    GAOES_PY_SPLOT,
		    NULL);
    copy_file(src, dest);
    chmod(dest, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH|S_IXUSR|S_IXGRP|S_IXOTH);
    g_free(src);
  }
  g_free(dest);
  
  // login.cl
  dest=g_strconcat(hl->wdir,
		   G_DIR_SEPARATOR_S,
		   "login.cl",
		   NULL);
  if(access(dest, F_OK)!=0){
    src=g_strconcat(hl->udir,
		    G_DIR_SEPARATOR_S,
		    "login.cl",
		    NULL);
    copy_file(src, dest);
    chmod(dest, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    g_free(src);
  }
  g_free(dest);
}


gchar *get_logincl_dir(typHLOG *hl){
  gchar *ret, *homed;
  
  ret=g_strdup(g_get_home_dir());
  g_free(homed);
  if(access(ret, F_OK)!=0){
    fprintf(stderr, "Cannot access to your login.cl directory \"%s\" .\n",
	    ret);
    exit(-1);
  }
  return(ret);
}





void clip_copy(GtkWidget *widget, gpointer gdata){
  GtkWidget *entry;
  GtkClipboard* clipboard=gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  const gchar *c;

  entry=(GtkWidget *)gdata;

  c = gtk_entry_get_text(GTK_ENTRY(entry));
  gtk_clipboard_set_text (clipboard, c, strlen(c));
}


gboolean close_popup(gpointer data)
{
  GtkWidget *dialog;

  dialog=(GtkWidget *)data;

  gtk_main_quit();
  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);

  return(FALSE);
}

gboolean check_file(gpointer data)
{
  typHLOG *hl=(typHLOG *)data;

  if(access(hl->file_wait, F_OK)==0){
    gtk_main_quit();
    return(FALSE);
  }
  else{
    return(TRUE);
  }
}

gboolean destroy_popup(GtkWidget *w, GdkEvent *event, gint *data)
{
  g_source_remove(*data);
  gtk_main_quit();
  return(FALSE);
}

void popup_message(GtkWidget *parent, gchar* stock_id,gint delay, ...){
  va_list args;
  gchar *msg1;
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *button;
  GtkWidget *pixmap;
  GtkWidget *hbox;
  GtkWidget *vbox;
  gint timer;

  va_start(args, delay);

  if(delay>0){
    dialog = gtk_dialog_new();
  }
  else{
    dialog = gtk_dialog_new_with_buttons("GAOES-RV Log Editor : Message",
					 GTK_WINDOW(parent),
					 GTK_DIALOG_MODAL,
#ifdef USE_GTK3
					 "_OK",GTK_RESPONSE_OK,
#else
					 GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
					 NULL);
  }
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(parent));
  gtk_window_set_title(GTK_WINDOW(dialog),"GAOES-RV Log Editor : Message");

#if !GTK_CHECK_VERSION(2,21,8)
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog),FALSE);
#endif

  if(delay>0){
    timer=g_timeout_add(delay*1000, (GSourceFunc)close_popup,
			(gpointer)dialog);
    g_signal_connect(dialog,"delete-event",G_CALLBACK(destroy_popup), &timer);
  }

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name (stock_id,
				       GTK_ICON_SIZE_DIALOG);
#else
  pixmap=gtk_image_new_from_stock (stock_id,
				   GTK_ICON_SIZE_DIALOG);
#endif

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);

  while(1){
    msg1=va_arg(args,gchar*);
    if(!msg1) break;
   
    label=gtkut_label_new(msg1);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(vbox),
		       label,TRUE,TRUE,0);
  }

  va_end(args);

  gtk_widget_show_all(dialog);

  if(delay>0){
    gtk_main();
  }
  else{
    gtk_dialog_run(GTK_DIALOG(dialog));
    if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  }
}


gboolean wait_for_file(typHLOG *hl, gchar *msg){
  gchar *tmp;
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *button;
  GtkWidget *pixmap;
  GtkWidget *hbox;
  GtkWidget *vbox;
  gint timer;

  dialog = gtk_dialog_new();

  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hl->w_top));
  gtk_window_set_title(GTK_WINDOW(dialog),"GAOES-RV Log Editor : Message");
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);

  timer=g_timeout_add(1000, 
		      (GSourceFunc)check_file,
		      (gpointer)hl);
  g_signal_connect(dialog,"delete-event",G_CALLBACK(destroy_popup), &timer);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name ("dialog-information", 
				       GTK_ICON_SIZE_DIALOG);
#else
  pixmap=gtk_image_new_from_stock (GTK_STOCK_DIALOG_INFO,
				   GTK_ICON_SIZE_DIALOG);
#endif

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);

  label=gtkut_label_new(msg);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),
		     label,TRUE,TRUE,0);

  tmp=g_strconcat("    <span font_family=\"monospace\">",
		  hl->file_wait,
		  "</span>",
		  NULL);
  label=gtkut_label_new(tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),
		     label,TRUE,TRUE,0);

  gtk_widget_show_all(dialog);

  gtk_main();

  if(GTK_IS_WIDGET(dialog)){
    gtk_window_set_modal(GTK_WINDOW(dialog),FALSE);
    gtk_widget_destroy(dialog);

    return(TRUE);
  }
  else{
    return(FALSE);
  }
}


void clear_ql(typHLOG *hl){
  gboolean ret;
  
  hl->ql_err++;
  
  if(hl->ql_err<QL_ERROR_THRESHOLD){
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  "Another reduction process is still running.",
		  " ",
		  "Please close it before creating a new reduction session.",
		  NULL);
  }
  else{
    ret=popup_dialog(hl->w_top, 
#ifdef USE_GTK3
		     "dialog-question", 
#else
		     GTK_STOCK_DIALOG_QUESTION,
#endif
		     "Another reduction process is still running.",
		     " ",
		     "<b>Do you want to remove the lock file?</b>:",
		     NULL);

    if(ret){
      hl->ql_err=0;

      if(access(hl->ql_lock,F_OK)==0){
	if(hl->ql_timer>0){
	  g_source_remove(hl->ql_timer);
	  hl->ql_timer=-1;
	}
	
	unlink(hl->ql_lock);
	
	popup_message(hl->w_top, 
#ifdef USE_GTK3
		      "dialog-information", 
#else
		      GTK_STOCK_DIALOG_INFO,
#endif
		      POPUP_TIMEOUT,
		      "The QL lock-file has been Removed ",
		      " ",
		      "Please run a new reduction process.",
		  NULL);
      }
    }
    else{
      hl->ql_err++;
    }
  }
}


gboolean popup_dialog(GtkWidget *parent, gchar* stock_id, ...){
  va_list args;
  gchar *msg1;
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *button;
  GtkWidget *pixmap;
  GtkWidget *hbox;
  GtkWidget *vbox;
  gboolean ret=FALSE;

  va_start(args, stock_id);

  dialog = gtk_dialog_new_with_buttons("GAOES-RV Log Editor : Dialog",
				       GTK_WINDOW(parent),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_No",GTK_RESPONSE_NO,
				       "_Yes",GTK_RESPONSE_YES,
#else
				       GTK_STOCK_NO,GTK_RESPONSE_NO,
				       GTK_STOCK_YES,GTK_RESPONSE_YES,
#endif
				       NULL);
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(parent));
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_NO); 

#if !GTK_CHECK_VERSION(2,21,8)
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog),FALSE);
#endif

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name (stock_id,
				       GTK_ICON_SIZE_DIALOG);
#else
  pixmap=gtk_image_new_from_stock (stock_id,
				   GTK_ICON_SIZE_DIALOG);
#endif

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);

  while(1){
    msg1=va_arg(args,gchar*);
    if(!msg1) break;
   
    label=gtkut_label_new(msg1);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(vbox),
		       label,TRUE,TRUE,0);
  }

  va_end(args);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES) {
    ret=TRUE;
  }

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);

  return(ret);
}


gboolean write_dialog(typHLOG *hl, gchar* stock_id, gchar* file_str){
  gchar *tmp;
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *button;
  GtkWidget *pixmap;
  GtkWidget *hbox;
  GtkWidget *vbox;
  gboolean ret=FALSE;

  dialog = gtk_dialog_new_with_buttons("GAOES-RV Log Editor : Dialog",
				       GTK_WINDOW(hl->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_Cancel",GTK_RESPONSE_CANCEL,
				       "_OK",GTK_RESPONSE_OK,
#else
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
				       NULL);
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hl->w_top));
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name (stock_id,
				       GTK_ICON_SIZE_DIALOG);
#else
  pixmap=gtk_image_new_from_stock (stock_id,
				   GTK_ICON_SIZE_DIALOG);
#endif

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);

  tmp=g_strdup_printf("Create a file for %s : ", file_str);
  label=gtk_label_new(tmp);
  g_free(tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),
		     label,TRUE,TRUE,0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(vbox),
		     hbox,TRUE,TRUE,0);

  entry = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);
  gtk_entry_set_width_chars(GTK_ENTRY(entry),40);
  gtk_entry_set_text(GTK_ENTRY(entry), hl->file_write);
  g_signal_connect (entry,
		    "changed",
		    G_CALLBACK(cc_get_entry),
		    &hl->file_write);

  label=gtkut_label_new("<span font_family=\"monospace\">.fits</span>");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),
		     label,TRUE,TRUE,0);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    ret=TRUE;
  }

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);

  return(ret);
}


gboolean ow_check(typHLOG *hl, gchar* file_in){
  gchar *tmp;
  gchar *filename;
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *button;
  GtkWidget *pixmap;
  GtkWidget *hbox;
  GtkWidget *vbox;
  gboolean ret=FALSE;

  filename=g_strconcat(hl->wdir,
		       G_DIR_SEPARATOR_S,
		       file_in, 
		       ".fits",
		       NULL);
  if(access(filename, F_OK)!=0){
    g_free(filename);
    return(TRUE);
  }

  dialog = gtk_dialog_new_with_buttons("GAOES-RV Log Editor : Dialog",
				       GTK_WINDOW(hl->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_No",GTK_RESPONSE_NO,
				       "_Yes",GTK_RESPONSE_YES,
#else
				       GTK_STOCK_NO,GTK_RESPONSE_NO,
				       GTK_STOCK_YES,GTK_RESPONSE_YES,
#endif
				       NULL);
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hl->w_top));
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_NO); 

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name ("dialog-warning", 
				       GTK_ICON_SIZE_DIALOG);
#else
  pixmap=gtk_image_new_from_stock (GTK_STOCK_DIALOG_WARNING,
				   GTK_ICON_SIZE_DIALOG);
#endif

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);

  label=gtk_label_new("The file");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),
		     label,TRUE,TRUE,0);
  
  tmp=g_strdup_printf("    <span font_family=\"monospace\">%s</span>", 
		      filename);
  label=gtkut_label_new(tmp);
  g_free(tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),
		     label,TRUE,TRUE,0);

  label=gtk_label_new("already exists.");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),
		     label,TRUE,TRUE,0);

  label=gtk_label_new(" ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),
		     label,TRUE,TRUE,0);

  label=gtk_label_new("Do you want to overwrite it?");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),
		     label,TRUE,TRUE,0);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES) {
    unlink(filename);
    ret=TRUE;
  }

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  //while(g_main_iteration(FALSE));

  g_free(filename);
  return(ret);
}


gchar *get_indir(typHLOG *hl, gint i){
  gchar*ret;

  if(hl->frame[i].idnum < 10000){
    ret=g_strconcat(hl->ddir,
		    G_DIR_SEPARATOR_S,
		    "HDSA0000",
		    NULL);
  }
  else if(hl->frame[i].idnum < 100000){
    ret=g_strconcat(hl->ddir,
		    G_DIR_SEPARATOR_S,
		    "HDSA000",
		    NULL);
  }
  else{
    ret=g_strconcat(hl->ddir,
		    G_DIR_SEPARATOR_S,
		    "HDSA00",
		    NULL);
  }

  return(ret);
}



gint get_cnt(typHLOG *hl, glong i_file){
  gchar *cnt_file;
  FILE *fp;
  gint ret=-1, fret;

  cnt_file=g_strdup_printf("%s%sG%08ld_cnt",
			   hl->wdir,
			   G_DIR_SEPARATOR_S,
			   hl->frame[i_file].idnum);
  if((fp=fopen(cnt_file, "r"))!=NULL){
    fret=fscanf(fp, "%d", &ret);
    fclose(fp);
  }

  g_free(cnt_file);
  return(ret);
}

void iraf_obj(typHLOG *hl, gint i_sel, glong i_file){
  gchar *tmp; 

  switch(ql_ext_check(hl)){
  case -1:
    clear_ql(hl);
    return;
    break;
    
  case 0:
  case 1:
    hl->ql_err=0;
    break;
  }

  tmp=g_strdup_printf("%s \'cd %s;%s %s%s%s %s \"%08ld\" %s %s %s %s %s %d %d %d %d %d %s %s %d\'",
		      hl->ql_terminal,
		      hl->wdir,
		      hl->ql_python,
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      GAOES_PY_GRQL,
		      hl->ql_lock,
		      hl->frame[i_sel].idnum,
		      hl->ddir,
		      hl->ql_ap,
		      hl->ql_flat,
		      hl->ql_thar1d,
		      hl->ql_thar2d,
		      hl->ql_st_x,
		      hl->ql_ed_x,
		      hl->ql_ge_line,
		      hl->ql_ge_stx,
		      hl->ql_ge_edx,
		      hl->ql_blaze,
		      hl->ql_mask,
		      hl->ql_line);

  if(debug_flg){
    fprintf(stderr,"!!!Open PyRAF terminal\n%s\n",tmp);
  }

  hl->ql_i=i_sel;
  hl->ql_loop=QL_OBJECT;
  hl->ql_timer=g_timeout_add(1000, (GSourceFunc)check_ql,
  			     (gpointer)hl);
  hl->frame[i_sel].qlr=QLR_NOW;
  tree_update_frame(hl);
  ql_ext_play(hl,tmp);
  
  g_free(tmp);
}

void iraf_obj_splot(typHLOG *hl, gint i_sel, gchar *spec_file){
  gchar *tmp; 

  switch(ql_ext_check(hl)){
  case -1:
    clear_ql(hl);
    return;
    break;
    
  case 0:
  case 1:
    hl->ql_err=0;
    break;
  }

  tmp=g_strdup_printf("%s \'cd %s;%s %s%s%s %s %s %d\'",
		      hl->ql_terminal,
		      hl->wdir,
		      hl->ql_python,
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      GAOES_PY_SPLOT,
		      hl->ql_lock,
		      spec_file,
		      hl->ql_line);

  if(debug_flg){
    fprintf(stderr,"!!!Open PyRAF terminal\n%s\n",tmp);
  }
  
  hl->ql_i=i_sel;
  hl->ql_loop=QL_SPLOT;
  hl->ql_timer=g_timeout_add(1000, (GSourceFunc)check_ql,
  			     (gpointer)hl);
  ql_ext_play(hl,tmp);
  
  g_free(tmp);
}

void iraf_obj_batch(typHLOG *hl, gint i_sel, glong i_file, gchar *obj_in){
  gchar *tmp;
  gint sret;

  tmp=g_strdup_printf("echo %08ld >> %s",
		      i_file,
		      obj_in);

  sret=system(tmp);
  g_free(tmp);
}

void ql_obj_foreach (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;
  gint i;
  glong i_file;
  gchar *tmp;
    
  gtk_tree_model_get (model, iter, COLUMN_FRAME_NUMBER, &i, -1);

  
  if(hl->ql_line<0){
    tmp=g_strdup_printf("G%08ldocs_ecfw",hl->frame[i].idnum);
    if(ow_check(hl, tmp)){
      iraf_obj(hl, i, hl->frame[i].idnum);
    }
  }
  else if(hl->ql_line==0){
    tmp=g_strdup_printf("G%08ldocs_ecfw_1d",hl->frame[i].idnum);
    if(ow_check(hl, tmp)){
      iraf_obj(hl, i, hl->frame[i].idnum);
    }
    else{
      iraf_obj_splot(hl, i, tmp);
    }
  }
  else{
    tmp=g_strdup_printf("G%08ldocs_ecfw",hl->frame[i].idnum);
    if(ow_check(hl, tmp)){
      iraf_obj(hl, i, hl->frame[i].idnum);
    }
    else{
      iraf_obj_splot(hl, i, tmp);
    }
  }
  g_free(tmp);
}


void ql_obj_batch_foreach (GtkTreeModel *model, GtkTreePath *path, 
			   GtkTreeIter *iter, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;
  gint i;
  glong i_file;
  gchar *obj_in;

  gtk_tree_model_get (model, iter, COLUMN_FRAME_NUMBER, &i, -1);
  
  obj_in=g_strdup_printf("%s%sobj.in",
			 hl->wdir,
			 G_DIR_SEPARATOR_S);

  iraf_obj_batch(hl, i, hl->frame[i].idnum, obj_in);
  g_free(obj_in);
}

void make_obj_batch(typHLOG *hl, gchar *obj_in){
  gchar *tmp;
  gint i_sel;

  switch(ql_ext_check(hl)){
  case -1:
    clear_ql(hl);
    return;
    break;
    
  case 0:
  case 1:
    hl->ql_err=0;
    break;
  }

  tmp=g_strdup_printf("%s \'cd %s;%s %s%s%s %s %s %s %s %s %s %s %d %d %d %d %d %s %s\'",
		      hl->ql_terminal,
		      hl->wdir,
		      hl->ql_python,
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      GAOES_PY_GRQL_BATCH,
		      hl->ql_lock,
		      obj_in,
		      hl->ddir,
		      hl->ql_ap,
		      hl->ql_flat,
		      hl->ql_thar1d,
		      hl->ql_thar2d,
		      hl->ql_st_x,
		      hl->ql_ed_x,
		      hl->ql_ge_line,
		      hl->ql_ge_stx,
		      hl->ql_ge_edx,
		      hl->ql_blaze,
		      hl->ql_mask);

  if(debug_flg){
    fprintf(stderr,"!!!Open PyRAF terminal\n%s\n",tmp);
  }
  
  hl->ql_loop=QL_OBJECT_BATCH;
  hl->ql_timer=g_timeout_add(1000, (GSourceFunc)check_ql,
  			     (gpointer)hl);
  ql_ext_play(hl,tmp);
  g_free(tmp); 
}


void ql_obj_red(GtkWidget *w, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  gchar *tmp, *obj_in;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hl->frame_tree));
  gint i_rows;

  i_rows=gtk_tree_selection_count_selected_rows (selection);

  if(i_rows==1){
    gtk_tree_selection_selected_foreach (selection, ql_obj_foreach, (gpointer)hl);
  }
  else{
    obj_in=g_strdup_printf("%s%sobj.in",
			   hl->wdir,
			   G_DIR_SEPARATOR_S);
    if(access(obj_in, F_OK)==0){
      unlink(obj_in);
    }
    gtk_tree_selection_selected_foreach (selection, ql_obj_batch_foreach, (gpointer)hl);

    if(access(obj_in, F_OK)==0){
      make_obj_batch(hl, obj_in);
    }
    else{
      popup_message(hl->w_top, 
#ifdef USE_GTK3
		    "dialog-error", 
#else
		    GTK_STOCK_DIALOG_ERROR,
#endif
		    -1,
		    obj_in,
		    " ",
		    "Failed to create a Object list file.",
		    NULL);
    }
    g_free(obj_in);
  }
}





///////////////// Flat ///////////////////


void iraf_flat(typHLOG *hl, gint i_sel, glong i_file, gchar *flat_in){
  gchar *tmp;
  gchar *ap_bak, *src, *dest;
  gint sret;

  if(strcmp(hl->frame[i_sel].type,"FLAT")!=0){
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  "The selected frame is not \"<b>FLAT</b>\".",
		  "Aborting...\".",
		  NULL);
    return;
  }

  if(!hl->ql_flat_new){
    hl->ql_flat_new=g_strdup_printf("Flat.%08ld", i_file);
  }
  if(!hl->ql_ap_new){
    hl->ql_ap_new=g_strdup_printf("Ap.%08ld", i_file);

    // Check Redo
    if(strcmp(hl->ql_ap_new,hl->ql_ap)==0){
      // Remove fits
      src=g_strconcat(hl->wdir,
		      G_DIR_SEPARATOR_S,
		      hl->ql_ap,
		      ".fits",
		      NULL);
      if(access(src, F_OK)==0){
	unlink(src);
      }
      g_free(src);

      // Remove ec.fits
      src=g_strconcat(hl->wdir,
		      G_DIR_SEPARATOR_S,
		      hl->ql_ap,
		      ".ec.fits",
		      NULL);
      if(access(src, F_OK)==0){
	unlink(src);
      }
      g_free(src);

      // Remove database
      src=g_strconcat(hl->wdir,
		      G_DIR_SEPARATOR_S,
		      "database",
		      G_DIR_SEPARATOR_S,
		      "ap",
		      hl->ql_ap,
		      NULL);
      if(access(src, F_OK)==0){
	unlink(src);
      }
      g_free(src);

      // Use default ap
      g_free(hl->ql_ap);
      hl->ql_ap=g_strdup(GAOES_AP);     
    }
  }
  
  
  hl->frame[i_sel].cal=QLCAL_FLAT0;
  
  tmp=g_strdup_printf("echo %08ld >> %s",
		      i_file,
		      flat_in);

  sret=system(tmp);
  g_free(tmp);
}


void finish_flat(typHLOG *hl){
  gchar *tmp, *tmp_ap, *tmp_flat;
  gint i;

  if(debug_flg){
    fprintf(stderr,"Checking Finish of Flat...\n");
  }

  tmp=g_strconcat(hl->wdir,
		  G_DIR_SEPARATOR_S,
		  hl->ql_flat_new,
		  ".sc.nm.fits",
		  NULL);

  if(access(tmp, F_OK)==0){
    g_free(tmp);
    tmp=g_strconcat(hl->ql_flat_new,
		    ".sc.nm",
		    NULL);
    if(hl->ql_flat) g_free(hl->ql_flat);
    hl->ql_flat=g_strdup(tmp);
    
    tmp_flat=g_strdup_printf(" <b>New Flat</b> : %s", hl->ql_flat);
    
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-information", 
#else
		  GTK_STOCK_DIALOG_INFO,
#endif
		  POPUP_TIMEOUT*2,
		  "A new Flat frame has been successfully created and set for QL.",
		  " ",
		  tmp_flat,
		  NULL);
    g_free(tmp_flat);

    for(i=0;i<hl->num;i++){
      if(hl->frame[i].cal==QLCAL_FLAT0){
	hl->frame[i].cal=QLCAL_FLAT;
      }
      else if(hl->frame[i].cal==QLCAL_FLAT){
	hl->frame[i].cal=QLCAL_NONE;
      }
    }
  }
  else{
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR,
#endif
		  -1,
		  tmp,
		  " ",
		  "Failed to create a new Flat frame.",
		  NULL);
    for(i=0;i<hl->num;i++){
      if(hl->frame[i].cal==QLCAL_FLAT0){
	hl->frame[i].cal=QLCAL_NONE;
      }
    }
  }
  
  g_free(tmp);

  tmp=g_strconcat(hl->wdir,
		  G_DIR_SEPARATOR_S,
		  hl->ql_ap_new,
		  ".fits",
		  NULL);

  if(access(tmp, F_OK)==0){
    if(hl->ql_ap) g_free(hl->ql_ap);
    hl->ql_ap=g_strdup(hl->ql_ap_new);

    tmp_ap=g_strdup_printf(" <b>New Aperture</b> : %s", hl->ql_ap);
    
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-information", 
#else
		  GTK_STOCK_DIALOG_INFO,
#endif
		  POPUP_TIMEOUT*2,
		  "A new Aperture reference has been successfully created and set for QL.",
		  " ",
		  tmp_ap,
		  NULL);
    g_free(tmp_ap);
  }
  else{
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR,
#endif
		  -1,
		  tmp,
		  " ",
		  "Failed to create a new Aperture frame.",
		  NULL);
  }
  
  g_free(tmp);
  
}

void make_flat(typHLOG *hl, gchar *flat_in){
  gchar *tmp;
  gint i_sel;

  switch(ql_ext_check(hl)){
  case -1:
    clear_ql(hl);
    return;
    break;
    
  case 0:
  case 1:
    hl->ql_err=0;
    break;
  }

  tmp=g_strdup_printf("%s \'cd %s;%s %s%s%s %s %s %s %s %s %s %d %d\'",
		      hl->ql_terminal,
		      hl->wdir,
		      hl->ql_python,
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      GAOES_PY_FLAT,
		      hl->ql_lock,
		      flat_in,
		      hl->ddir,
		      hl->ql_flat_new,
		      hl->ql_ap,
		      hl->ql_ap_new,
		      hl->ql_st_x,
		      hl->ql_ed_x);

  if(debug_flg){
    fprintf(stderr,"!!!Open PyRAF terminal\n%s\n",tmp);
  }

  hl->ql_loop=QL_FLAT;
  
  hl->ql_timer=g_timeout_add(1000, (GSourceFunc)check_ql,
  			     (gpointer)hl);
  tree_update_frame(hl);
  ql_ext_play(hl, tmp);
  
  g_free(tmp); 
}

void ql_flat_foreach (GtkTreeModel *model, GtkTreePath *path, 
		      GtkTreeIter *iter, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;
  gint i, i_file;
  gchar *flat_in;

  gtk_tree_model_get (model, iter, COLUMN_FRAME_NUMBER, &i, -1);
  
  flat_in=g_strdup_printf("%s%sflat.in",
			  hl->wdir,
			  G_DIR_SEPARATOR_S);

  iraf_flat(hl, i, hl->frame[i].idnum, flat_in);
  g_free(flat_in);
}

void ql_flat_red(GtkWidget *w, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  gchar *flat_in;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hl->frame_tree));
  gint i_rows;

  i_rows=gtk_tree_selection_count_selected_rows (selection);

  if(hl->ql_ap_new) g_free(hl->ql_ap_new);
  hl->ql_ap_new=NULL;
  if(hl->ql_flat_new) g_free(hl->ql_flat_new);
  hl->ql_flat_new=NULL;

  flat_in=g_strdup_printf("%s%sflat.in",
			  hl->wdir,
			  G_DIR_SEPARATOR_S);
  if(access(flat_in, F_OK)==0){
    unlink(flat_in);
  }
  
  gtk_tree_selection_selected_foreach (selection, ql_flat_foreach, (gpointer)hl);

  if(access(flat_in, F_OK)==0){
    make_flat(hl, flat_in);
  }
  else{
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR,
#endif
		  -1,
		  flat_in,
		  " ",
		  "Failed to create a Flat list file.",
		  NULL);
  }
  g_free(flat_in);
}


void iraf_flat_auto(typHLOG *hl){
  gchar *flat_in;
  gchar *ap_bak, *src, *dest;
  gint i, i_sel;
  glong i_file;
  FILE *fp;

  i_file=hl->frame[hl->auto_flat[0]].idnum;

  if(!hl->ql_flat_new){
    hl->ql_flat_new=g_strdup_printf("Flat.%08ld", i_file);
  }
  if(!hl->ql_ap_new){
    hl->ql_ap_new=g_strdup_printf("Ap.%08ld", i_file);

    // Check Redo
    if(strcmp(hl->ql_ap_new,hl->ql_ap)==0){
      // Remove fits
      src=g_strconcat(hl->wdir,
		      G_DIR_SEPARATOR_S,
		      hl->ql_ap,
		      ".fits",
		      NULL);
      if(access(src, F_OK)==0){
	unlink(src);
      }
      g_free(src);

      // Remove ec.fits
      src=g_strconcat(hl->wdir,
		      G_DIR_SEPARATOR_S,
		      hl->ql_ap,
		      ".ec.fits",
		      NULL);
      if(access(src, F_OK)==0){
	unlink(src);
      }
      g_free(src);

      // Remove database
      src=g_strconcat(hl->wdir,
		      G_DIR_SEPARATOR_S,
		      "database",
		      G_DIR_SEPARATOR_S,
		      "ap",
		      hl->ql_ap,
		      NULL);
      if(access(src, F_OK)==0){
	unlink(src);
      }
      g_free(src);

      // Use default ap
      g_free(hl->ql_ap);
      hl->ql_ap=g_strdup(GAOES_AP);     
    }
  }
  
  flat_in=g_strdup_printf("%s%sflat.in",
			  hl->wdir,
			  G_DIR_SEPARATOR_S);

  if(access(flat_in, F_OK)==0){
    unlink(flat_in);
  }
  fp=fopen(flat_in,"w");
  
  for(i=0;i<AUTO_FLAT_NUM;i++){
    i_sel=hl->auto_flat[i];
    hl->frame[i_sel].cal=QLCAL_FLAT0;

    fprintf(fp,"%08ld\n",hl->frame[i_sel].idnum);
  }

  fclose(fp);

  if(access(flat_in, F_OK)==0){
    make_flat(hl, flat_in);
  }
  else{
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR,
#endif
		  -1,
		  flat_in,
		  " ",
		  "Failed to create a Flat list file.",
		  NULL);
  }
  g_free(flat_in);
}


///////////////// ThAr /////////////////

void finish_thar(typHLOG *hl){
  gchar *tmp;
  gchar *tmp_1d, *tmp_2d;
  gint i;

  if(debug_flg){
    fprintf(stderr,"Checking Finish of ThAr...\n");
  }
  
  tmp=g_strconcat(hl->wdir,
		  G_DIR_SEPARATOR_S,
		  hl->ql_thar_new,
		  ".center.fits",
		  NULL);

  if(access(tmp, F_OK)==0){
    if(hl->ql_thar2d) g_free(hl->ql_thar2d);
    hl->ql_thar2d=g_strdup(hl->ql_thar_new);
    if(hl->ql_thar1d) g_free(hl->ql_thar1d);
    hl->ql_thar1d=g_strdup_printf("%s.center",hl->ql_thar_new);

    tmp_2d=g_strdup_printf(" <b>New 2D Comparison</b> : %s", hl->ql_thar2d);
    tmp_1d=g_strdup_printf(" <b>New 1D Comparison</b> : %s", hl->ql_thar1d);
    
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-information", 
#else
		  GTK_STOCK_DIALOG_INFO,
#endif
		  POPUP_TIMEOUT*2,
		  "New Comarison frames have been successfully created and set for QL.",
		  " ",
		  tmp_2d,
		  tmp_1d,
		  NULL);
    g_free(tmp_2d);
    g_free(tmp_1d);
    
    for(i=0;i<hl->num;i++){
      if(hl->frame[i].cal==QLCAL_COMP0){
	hl->frame[i].cal=QLCAL_COMP; 
      }
      else if(hl->frame[i].cal==QLCAL_COMP){
	hl->frame[i].cal=QLCAL_NONE; 
      }
    }
  }
  else{
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR,
#endif
		  -1,
		  tmp,
		  " ",
		  "Failed to create a new Comparison frame.",
		  NULL);
    for(i=0;i<hl->num;i++){
      if(hl->frame[i].cal==QLCAL_COMP0){
	hl->frame[i].cal=QLCAL_NONE;
      }
    }
  }    
}

void finish_obj(typHLOG *hl){
}

void finish_mask(typHLOG *hl){
  gchar *tmp, *tmp_mask;
  
  tmp=g_strconcat(hl->wdir,
		  G_DIR_SEPARATOR_S,
		  hl->ql_mask_new,
		  ".fits",
		  NULL);

  if(access(tmp, F_OK)==0){
    if(hl->ql_mask) g_free(hl->ql_mask);
    hl->ql_mask=g_strdup(hl->ql_mask_new);

    tmp_mask=g_strdup_printf(" <b>New Mask</b> : %s", hl->ql_mask);
    
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-information", 
#else
		  GTK_STOCK_DIALOG_INFO,
#endif
		  POPUP_TIMEOUT*2,
		  "A new Mask image has been successfully created and set for QL.",
		  " ",
		  tmp_mask,
		  NULL);
    g_free(tmp_mask);
  }
  else{
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR,
#endif
		  -1,
		  tmp,
		  " ",
		  "Failed to create a new Mask image.",
		  NULL);
  }    
}

void finish_blaze(typHLOG *hl){
  gchar *tmp, *tmp_blaze;
  
  tmp=g_strconcat(hl->wdir,
		  G_DIR_SEPARATOR_S,
		  hl->ql_blaze_new,
		  ".fits",
		  NULL);

  if(access(tmp, F_OK)==0){
    if(hl->ql_blaze) g_free(hl->ql_blaze);
    hl->ql_blaze=g_strdup(hl->ql_blaze_new);

    tmp_blaze=g_strdup_printf(" <b>New Blaze function</b> : %s", hl->ql_blaze);
    
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-information", 
#else
		  GTK_STOCK_DIALOG_INFO,
#endif
		  POPUP_TIMEOUT*2,
		  "A new Blaze function has been successfully created and set for QL.",
		  " ",
		  tmp_blaze,
		  NULL);
    g_free(tmp_blaze);
  }
  else{
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR,
#endif
		  -1,
		  tmp,
		  " ",
		  "Failed to create a new Blaze function.",
		  NULL);
  }    
}

  
void iraf_thar(typHLOG *hl, gint i_sel, glong i_file){
  gchar *tmp, *tmp_thar1d;
  gchar *thar_bak, *src, *dest;

  switch(ql_ext_check(hl)){
  case -1:
    clear_ql(hl);
    return;
    break;
    
  case 0:
  case 1:
    hl->ql_err=0;
    break;
  }

  if(strcmp(hl->frame[i_sel].type,"COMPARISON")!=0){
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  "The selected frame is not \"<b>COMPARISON</b>\".",
		  NULL);
    return;
  }
  
  if(hl->ql_thar_new) g_free(hl->ql_thar_new);
  hl->ql_thar_new=g_strdup_printf("ThAr.%08ld", hl->frame[i_sel].idnum);
  tmp_thar1d=g_strdup_printf("%s.center",hl->ql_thar_new);

  // Check Redo
  if(strcmp(tmp_thar1d,hl->ql_thar1d)==0){
    // The case New and Current are same
    // Remove fits
    src=g_strconcat(hl->wdir,
		    G_DIR_SEPARATOR_S,
		    hl->ql_thar1d,
		    ".fits",
		    NULL);
    if(access(src, F_OK)==0){
      unlink(src);
    }
    g_free(src);
    
    // Remove database
    src=g_strconcat(hl->wdir,
		    G_DIR_SEPARATOR_S,
		    "database",
		    G_DIR_SEPARATOR_S,
		    "ec",
		    hl->ql_thar1d,
		    NULL);
    if(access(src, F_OK)==0){
      unlink(src);
    }
    g_free(src);
    
    // Use default ec
    g_free(hl->ql_thar1d);
    hl->ql_thar1d=g_strdup(GAOES_THAR1D);
  }
  g_free(tmp_thar1d);
  
  hl->frame[i_sel].cal=QLCAL_COMP0;

  tmp=g_strdup_printf("%s \'cd %s;%s %s%s%s %s \"%08ld\" %s %s %s %s\'",
		      hl->ql_terminal,
		      hl->wdir,
		      hl->ql_python,
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      GAOES_PY_COMP,
		      hl->ql_lock,
		      hl->frame[i_sel].idnum,
		      hl->ddir,
		      hl->ql_thar_new,
		      hl->ql_ap,
		      hl->ql_thar1d);

  if(debug_flg){
    fprintf(stderr,"!!!Open PyRAF terminal\n%s\n",tmp);
  }


  hl->ql_loop=QL_THAR;
  hl->ql_timer=g_timeout_add(1000, (GSourceFunc)check_ql,
  			     (gpointer)hl);
  tree_update_frame(hl);
  ql_ext_play(hl,tmp);
  g_free(tmp);

}


void iraf_thar_obj(typHLOG *hl, gint i_sel, glong i_file){
  gchar *tmp, *tmp_thar1d;
  gchar *thar_bak, *src, *dest;

  switch(ql_ext_check(hl)){
  case -1:
    clear_ql(hl);
    return;
    break;
    
  case 0:
  case 1:
    hl->ql_err=0;
    break;
  }

  if(strcmp(hl->frame[i_sel].type,"COMPARISON")!=0){
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  "The selected frame is not \"<b>COMPARISON</b>\".",
		  NULL);
    return;
  }
  
  if(hl->ql_thar_new) g_free(hl->ql_thar_new);
  hl->ql_thar_new=g_strdup_printf("ThAr.%08ld", hl->frame[i_sel].idnum);
  tmp_thar1d=g_strdup_printf("%s.center",hl->ql_thar_new);

  // Check Redo
  if(strcmp(tmp_thar1d,hl->ql_thar1d)==0){
    // The case New and Current are same
    // Remove fits
    src=g_strconcat(hl->wdir,
		    G_DIR_SEPARATOR_S,
		    hl->ql_thar1d,
		    ".fits",
		    NULL);
    if(access(src, F_OK)==0){
      unlink(src);
    }
    g_free(src);
    
    // Remove database
    src=g_strconcat(hl->wdir,
		    G_DIR_SEPARATOR_S,
		    "database",
		    G_DIR_SEPARATOR_S,
		    "ec",
		    hl->ql_thar1d,
		    NULL);
    if(access(src, F_OK)==0){
      unlink(src);
    }
    g_free(src);
    
    // Use default ec
    g_free(hl->ql_thar1d);
    hl->ql_thar1d=g_strdup(GAOES_THAR1D);
  }
  g_free(tmp_thar1d);
  
  hl->frame[i_sel].cal=QLCAL_COMP0;

  tmp=g_strdup_printf("%s \'cd %s;%s %s%s%s %s \"%08ld\" %s %s %s %s %s %d %d %d %d %d %s %s %d\'",
		      hl->ql_terminal,
		      hl->wdir,
		      hl->ql_python,
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      GAOES_PY_COMP_OBJ,
		      hl->ql_lock,
		      hl->frame[i_sel].idnum,
		      hl->ddir,
		      hl->ql_thar_new,
		      hl->ql_ap,
		      hl->ql_thar1d,
		      hl->ql_flat,
		      hl->ql_st_x,
		      hl->ql_ed_x,
		      hl->ql_ge_line,
		      hl->ql_ge_stx,
		      hl->ql_ge_edx,
		      hl->ql_blaze,
		      hl->ql_mask,
		      -1);


  if(debug_flg){
    fprintf(stderr,"!!!Open PyRAF terminal\n%s\n",tmp);
  }


  hl->ql_loop=QL_THAR;
  hl->ql_timer=g_timeout_add(1000, (GSourceFunc)check_ql,
  			     (gpointer)hl);
  tree_update_frame(hl);
  ql_ext_play(hl,tmp);
  g_free(tmp);

}


void iraf_mask(typHLOG *hl, gint i_sel, glong i_file){
  gchar *tmp, *tmp2; 

  switch(ql_ext_check(hl)){
  case -1:
    clear_ql(hl);
    return;
    break;
    
  case 0:
  case 1:
    hl->ql_err=0;
    break;
  }

  tmp=g_strdup_printf("%s%sG%08ldocs_ecfw.fits",
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      hl->frame[i_sel].idnum);
  
  if(access(tmp,F_OK)!=0){
    tmp2=g_strdup_printf("    <b>%s</b>",tmp);
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  "Cannot access to a reduced spectrum,",
		  tmp2,
		  " ",
		  "Please reduce the data at once before creating a Mask image.",
		  NULL);
    g_free(tmp2);
    g_free(tmp);
    return;
  }
  g_free(tmp);

  if(hl->ql_mask_new) g_free(hl->ql_mask_new);
  hl->ql_mask_new=g_strdup_printf("Mask.%08ld", hl->frame[i_sel].idnum);

  tmp=g_strdup_printf("%s \'cd %s;%s %s%s%s %s G%08ldocs_ecfw %s\'",
		      hl->ql_terminal,
		      hl->wdir,
		      hl->ql_python,
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      GAOES_PY_MASK,
		      hl->ql_lock,
		      hl->frame[i_sel].idnum,
		      hl->ql_mask_new);

  if(debug_flg){
    fprintf(stderr,"!!!Open PyRAF terminal\n%s\n",tmp);
  }
  
  hl->ql_loop=QL_MASK;
  hl->ql_timer=g_timeout_add(1000, (GSourceFunc)check_ql,
  			     (gpointer)hl);
  ql_ext_play(hl,tmp);
  
  g_free(tmp);

}


void iraf_blaze(typHLOG *hl, gint i_sel, glong i_file){
  gchar *tmp, *tmp2; 

  switch(ql_ext_check(hl)){
  case -1:
    clear_ql(hl);
    return;
    break;
    
  case 0:
  case 1:
    hl->ql_err=0;
    break;
  }

  if(strcmp(hl->frame[i_sel].type,"FLAT")!=0){
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  "The selected frame is not \"<b>FLAT</b>\".",
		  NULL);
    return;
  }
  
  tmp=g_strdup_printf("%s%sG%08ldocs_ecfw.fits",
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      hl->frame[i_sel].idnum);
  
  if(access(tmp,F_OK)!=0){
    tmp2=g_strdup_printf("    <b>%s</b>",tmp);
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  "Cannot access to a reduced spectrum,",
		  tmp2,
		  " ",
		  "Please reduce the data at once before creating a Blaze function.",
		  NULL);
    g_free(tmp2);
    g_free(tmp);
    return;
  }
  g_free(tmp);

  if(hl->ql_blaze_new) g_free(hl->ql_blaze_new);
  hl->ql_blaze_new=g_strdup_printf("cBlaze.%08ld", hl->frame[i_sel].idnum);

  tmp=g_strdup_printf("%s \'cd %s;%s %s%s%s %s G%08ldocs_ecfw %s %s\'",
		      hl->ql_terminal,
		      hl->wdir,
		      hl->ql_python,
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      GAOES_PY_BLAZE,
		      hl->ql_lock,
		      hl->frame[i_sel].idnum,
		      hl->ql_blaze_new,
		      hl->ql_mask);

  if(debug_flg){
    fprintf(stderr,"!!!Open PyRAF terminal\n%s\n",tmp);
  }
  
  hl->ql_loop=QL_BLAZE;
  hl->ql_timer=g_timeout_add(1000, (GSourceFunc)check_ql,
  			     (gpointer)hl);
  ql_ext_play(hl,tmp);
  
  g_free(tmp);

}


void ql_thar_foreach (GtkTreeModel *model, GtkTreePath *path, 
		    GtkTreeIter *iter, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;
  gint i;
  glong i_file;
  gchar *c;
    
  gtk_tree_model_get (model, iter, COLUMN_FRAME_NUMBER, &i, -1);

  iraf_thar(hl, i, i_file);
}

void ql_mask_foreach (GtkTreeModel *model, GtkTreePath *path, 
		    GtkTreeIter *iter, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;
  gint i;
  glong i_file;
  gchar *c;
    
  gtk_tree_model_get (model, iter, COLUMN_FRAME_NUMBER, &i, -1);

  iraf_mask(hl, i, i_file);
}


void ql_blaze_foreach (GtkTreeModel *model, GtkTreePath *path, 
		    GtkTreeIter *iter, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;
  gint i;
  glong i_file;
  gchar *c;
    
  gtk_tree_model_get (model, iter, COLUMN_FRAME_NUMBER, &i, -1);

  iraf_blaze(hl, i, i_file);
}


void ql_thar_red(GtkWidget *w, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  gchar *tmp;
  gchar *cmdline;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hl->frame_tree));
  gint i_rows;
  gint tmp_scr=hl->scr_flag;

  i_rows=gtk_tree_selection_count_selected_rows (selection);

  if(i_rows==1){
    gtk_tree_selection_selected_foreach (selection, ql_thar_foreach, (gpointer)hl);
  }
  else{
    tmp=g_strdup_printf("<b>%d</b> rows are selected in the table.", 
			i_rows);
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  tmp,
		  " ",
		  "Please select only one row to make Wavelength reference file.",
		  NULL);
  }

  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->scr_combo),
			   tmp_scr);
}

void ql_mask(GtkWidget *w, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  gchar *tmp;
  gchar *cmdline;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hl->frame_tree));
  gint i_rows;
  gint tmp_scr=hl->scr_flag;

  i_rows=gtk_tree_selection_count_selected_rows (selection);

  if(i_rows==1){
    gtk_tree_selection_selected_foreach (selection, ql_mask_foreach, (gpointer)hl);
  }
  else{
    tmp=g_strdup_printf("<b>%d</b> rows are selected in the table.", 
			i_rows);
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  tmp,
		  " ",
		  "Please select only one row to make a new Mask image.",
		  NULL);
  }

  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->scr_combo),
			   tmp_scr);
}


void ql_blaze(GtkWidget *w, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  gchar *tmp;
  gchar *cmdline;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hl->frame_tree));
  gint i_rows;
  gint tmp_scr=hl->scr_flag;

  i_rows=gtk_tree_selection_count_selected_rows (selection);

  if(i_rows==1){
    gtk_tree_selection_selected_foreach (selection, ql_blaze_foreach, (gpointer)hl);
  }
  else{
    tmp=g_strdup_printf("<b>%d</b> rows are selected in the table.", 
			i_rows);
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  tmp,
		  " ",
		  "Please select only one row to make a new Blaze function.",
		  NULL);
  }

  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->scr_combo),
			   tmp_scr);
}



////////////// Find ///////////////////

void grlog_OpenFile(typHLOG *hl, guint mode){
  GtkWidget *fdialog;
  gchar *tmp;
  gchar *fp_file=NULL;
  gchar **tgt_file;
  GtkFileChooserAction caction;

  switch(mode){
  case OPEN_AP:
    tmp=g_strdup("GAOES-RV Log Editor : Select an Aperture File");
    //if(hl->ap_red[hl->iraf_hdsql_r]){
    if(hl->ql_ap){
      fp_file=g_strconcat(hl->wdir,
			  G_DIR_SEPARATOR_S,
			  hl->ql_ap,
			  ".fits",
			  NULL);
    }
    break;

  case OPEN_FLAT:
    tmp=g_strdup("GAOES-RV Log Editor : Select a Flat Image File");
    //if(hl->flat_red[hl->iraf_hdsql_r]){
    if(hl->ql_flat){
      fp_file=g_strconcat(hl->wdir,
			  G_DIR_SEPARATOR_S,
			  hl->ql_flat,
			  ".fits",
			  NULL);
    }
    break;

  case OPEN_THAR:
    tmp=g_strdup("GAOES-RV Log Editor : Select a ThAr File");
    //if(hl->thar_red[hl->iraf_hdsql_r]){
    if(hl->ql_thar1d){
      fp_file=g_strconcat(hl->wdir,
			  G_DIR_SEPARATOR_S,
			  hl->ql_thar1d,
			  ".fits",
			  NULL);
    }
    break;

  case OPEN_MASK:
    tmp=g_strdup("GAOES-RV Log Editor : Select a Mask File");
    if(hl->ql_mask){
      fp_file=g_strconcat(hl->wdir,
			  G_DIR_SEPARATOR_S,
			  hl->ql_mask,
			  ".fits",
			  NULL);
    }
    break;

  case OPEN_BLAZE:
    tmp=g_strdup("GAOES-RV Log Editor : Select a Blaze Function File");
    if(hl->ql_blaze){
      fp_file=g_strconcat(hl->wdir,
			  G_DIR_SEPARATOR_S,
			  hl->ql_blaze,
			  ".fits",
			  NULL);
    }
    break;

  case OPEN_LOG:
    tmp=g_strdup("GAOES-RV Log Editor : Select Observation Log file");
    break;

  case SAVE_LOG:
    tmp=g_strdup("GAOES-RV Log Editor : Input Observation Log file to be saved");
    break;
  }

  tgt_file=&fp_file;

  switch(mode){
  case SAVE_LOG:
    caction=GTK_FILE_CHOOSER_ACTION_SAVE;
    break;

  default:
    caction=GTK_FILE_CHOOSER_ACTION_OPEN;
    break;
  }
  fdialog = gtk_file_chooser_dialog_new(tmp,
					GTK_WINDOW(hl->w_top),
					caction,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					(mode==SAVE_LOG)? "_Save" : "_Open", GTK_RESPONSE_ACCEPT,
#else
					  GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  g_free(tmp);
  
  switch(mode){
  case SAVE_LOG:
    *tgt_file=g_strdup_printf("%s%sgrlog_%04d-%02d-%02d.csv",
			      hl->wdir,G_DIR_SEPARATOR_S,
			      hl->fr_year,hl->fr_month,hl->fr_day);
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
					 to_utf8(g_path_get_dirname(*tgt_file)));
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(g_path_get_basename(*tgt_file)));
    break;

  default:
    gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
    if(access(*tgt_file,F_OK)==0){
      switch(mode){
      default:
	gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				       to_utf8(*tgt_file));
	gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
					  to_utf8(*tgt_file));
      break;
      }
    }
    else{
      gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
					   hl->wdir);
    }

    switch(mode){
    case OPEN_AP:
      my_file_chooser_add_filter(fdialog,"Aperture Reference File",
				 "Ap*.fits",NULL);
      break;
      
    case OPEN_FLAT:
      my_file_chooser_add_filter(fdialog,"Normalized Flat Image File",
				 "Flat*.sc.nm.fits",NULL);
      break;
      
    case OPEN_THAR:
      my_file_chooser_add_filter(fdialog,"1D Wavelength Reference File",
				 "ThAr*.center.fits",NULL);
      break;
      
    case OPEN_MASK:
      my_file_chooser_add_filter(fdialog,"Mask File",
				 "Mask*.fits",NULL);
      break;
      
    case OPEN_BLAZE:
      my_file_chooser_add_filter(fdialog,"Blaze Function File",
				 "cBlaze*.fits",NULL);
      break;
      
    case OPEN_LOG:
      my_file_chooser_add_filter(fdialog,
				 "Obs Log Text File", "*.csv",
				 NULL);
      break;

    default:
      break;
    }
    my_file_chooser_add_filter(fdialog,"All File","*", NULL);
    break;
  }

  gtk_widget_show_all(fdialog);

  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *db_file, *tmp;
    gchar *dest_file, *fits_file;
    gchar *cpp, *basename0, *basename1, *thar_2d;
    gchar *tgt_dir=NULL;
    gchar *cp_dest;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      if(*tgt_file) g_free(*tgt_file);
      *tgt_file=g_strdup(dest_file);
      tgt_dir=g_path_get_dirname(*tgt_file);

      switch(mode){
      case OPEN_AP:
	basename0=get_refname(*tgt_file);
	db_file=g_strdup_printf("%s%sdatabase%sap%s",
				tgt_dir,
				G_DIR_SEPARATOR_S,
				G_DIR_SEPARATOR_S,
				basename0);
	
	if(access(db_file, F_OK)==0){
	  if(strcmp(hl->wdir,tgt_dir)!=0){ // Different dir COPY!
	    cp_dest=g_strdup_printf("%s%s%s",
				    hl->wdir,
				    G_DIR_SEPARATOR_S,
				    g_path_get_basename(*tgt_file));
	    copy_file(*tgt_file,cp_dest);
	    g_free(cp_dest);
	    cp_dest=g_strdup_printf("%s%sdatabase%sap%s",
				    hl->wdir,
				    G_DIR_SEPARATOR_S,
				    G_DIR_SEPARATOR_S,
				    basename0);
	    copy_file(db_file,cp_dest);
	    g_free(cp_dest);
	  }
	  
	  if(hl->ql_ap) g_free(hl->ql_ap);
	  hl->ql_ap=g_strdup(basename0);
	  set_ap_label(hl);
	}
	else{
	  popup_message(hl->w_top, 
#ifdef USE_GTK3
			"dialog-warning",
#else
			GTK_STOCK_DIALOG_WARNING, 
#endif
			-1,
			"<b>Error</b>: File cannot be used for Aperture reference.",
			" ",
			*tgt_file,
			NULL);
	}
	g_free(basename0);
	g_free(db_file);
	break;

      case OPEN_FLAT:
	if(strcmp(hl->wdir,tgt_dir)!=0){ // Different dir COPY!
	  cp_dest=g_strdup_printf("%s%s%s",
				  hl->wdir,
				  G_DIR_SEPARATOR_S,
				  g_path_get_basename(*tgt_file));
	  copy_file(*tgt_file,cp_dest);
	  g_free(cp_dest);
	}
	
	if(hl->ql_flat) g_free(hl->ql_flat);
	hl->ql_flat=get_refname(*tgt_file);
	set_flat_label(hl);
	break;

      case OPEN_THAR:
	basename0=get_refname(*tgt_file);
	basename1=get_refname(basename0);
	thar_2d=g_strdup_printf("%s%s%s.fits",
				tgt_dir,
				G_DIR_SEPARATOR_S,
				basename1);
	db_file=g_strdup_printf("%s%sdatabase%sec%s",
				tgt_dir,
				G_DIR_SEPARATOR_S,
				G_DIR_SEPARATOR_S,
				basename0);
	if((access(db_file, F_OK)==0)&&(access(thar_2d, F_OK)==0)){
	  if(strcmp(hl->wdir,tgt_dir)!=0){ // Different dir COPY!
	    cp_dest=g_strdup_printf("%s%s%s",
				    hl->wdir,
				    G_DIR_SEPARATOR_S,
				    g_path_get_basename(*tgt_file));
	    copy_file(*tgt_file,cp_dest);
	    g_free(cp_dest);
	    cp_dest=g_strdup_printf("%s%s%s.fits",
				    hl->wdir,
				    G_DIR_SEPARATOR_S,
				    basename1);
	    copy_file(thar_2d,cp_dest);
	    g_free(cp_dest);
	    cp_dest=g_strdup_printf("%s%sdatabase%sec%s",
				    hl->wdir,
				    G_DIR_SEPARATOR_S,
				    G_DIR_SEPARATOR_S,
				    basename0);
	    copy_file(db_file,cp_dest);
	    g_free(cp_dest);
	  }
	  
	  if(hl->ql_thar1d) g_free(hl->ql_thar1d);
	  hl->ql_thar1d=g_strdup(basename0);
	  if(hl->ql_thar2d) g_free(hl->ql_thar2d);
	  hl->ql_thar2d=g_strdup(basename1);
	  set_thar_label(hl);
	}
	else{
	  popup_message(hl->w_top, 
#ifdef USE_GTK3
			"dialog-warning",
#else
			GTK_STOCK_DIALOG_WARNING, 
#endif
			-1,
			"<b>Error</b>: File cannot be used for 2D Wavelength reference.",
			" ",
			*tgt_file,
			NULL);
	}
	g_free(basename0);
	g_free(basename1);
	g_free(thar_2d);
	g_free(db_file);
	break;

      case OPEN_MASK:
	if(strcmp(hl->wdir,tgt_dir)!=0){ // Different dir COPY!
	  cp_dest=g_strdup_printf("%s%s%s",
				  hl->wdir,
				  G_DIR_SEPARATOR_S,
				  g_path_get_basename(*tgt_file));
	  copy_file(*tgt_file,cp_dest);
	  g_free(cp_dest);
	}
	
	if(hl->ql_mask) g_free(hl->ql_mask);
	hl->ql_mask=get_refname(*tgt_file);
	set_mask_label(hl);
	break;

      case OPEN_BLAZE:
	if(strcmp(hl->wdir,tgt_dir)!=0){ // Different dir COPY!
	  cp_dest=g_strdup_printf("%s%s%s",
				  hl->wdir,
				  G_DIR_SEPARATOR_S,
				  g_path_get_basename(*tgt_file));
	  copy_file(*tgt_file,cp_dest);
	  g_free(cp_dest);
	}

	if(hl->ql_blaze) g_free(hl->ql_blaze);
	hl->ql_blaze=get_refname(*tgt_file);
	set_blaze_label(hl);
	break;

      case OPEN_LOG:
	{
	  FILE *fp;

	  if((fp=fopen(*tgt_file, "r"))!=NULL){
	  
	    ReadLog(hl, fp);

	    fclose(fp);
	  }
	}
	break;

      case SAVE_LOG:
	{
	  FILE *fp;

	  if((fp=fopen(*tgt_file, "w"))!=NULL){
	  
	    WriteLog(hl, fp);

	    fclose(fp);
	  }
	  save_cfg_cal(hl);
	}
	break;
      }
    }
    else{
      if(mode==SAVE_LOG){
	  FILE *fp;

	  if((fp=fopen(*tgt_file, "w"))!=NULL){
	  
	    WriteLog(hl, fp);

	    fclose(fp);
	  }
	  save_cfg_cal(hl);
      }
      else{
	popup_message(hl->w_top, 
#ifdef USE_GTK3
		      "dialog-warning",
#else
		      GTK_STOCK_DIALOG_WARNING, 
#endif
		      -1,
		      "<b>Error</b>: File cannot be opened.",
		      " ",
		      fname,
		      NULL);
      }
    }

    if(tgt_dir) g_free(tgt_dir);
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);

  }
}


void edit_ap (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;

  grlog_OpenFile(hl, OPEN_AP);
}

void edit_flat (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;

  grlog_OpenFile(hl, OPEN_FLAT);
}

void edit_thar (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;

  grlog_OpenFile(hl, OPEN_THAR);
}

void edit_mask (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;

  grlog_OpenFile(hl, OPEN_MASK);
}

void edit_blaze (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;

  grlog_OpenFile(hl, OPEN_BLAZE);
}

void edit_cal(GtkWidget *w, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  GtkWidget *dialog, *label, *frame, *combo, *hbox, 
    *vbox, *hbox1, *entry, *button, *table;
  gchar *tmp;
      
  dialog = gtk_dialog_new_with_buttons("GAOES-RV Log Editor : CAL frames for quick look",
				       GTK_WINDOW(hl->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_OK",GTK_RESPONSE_OK,
#else
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
				       NULL);
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hl->w_top));
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 


  frame = gtkut_frame_new ("<b> Aperture reference</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(vbox), hbox,FALSE, FALSE, 5);

  hl->label_edit_ap=gtkut_label_new("Ap");
  set_ap_label(hl);

#ifdef USE_GTK3
  gtk_widget_set_halign (hl->label_edit_ap, GTK_ALIGN_START);
  gtk_widget_set_valign (hl->label_edit_ap, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hl->label_edit_ap), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), hl->label_edit_ap, FALSE, FALSE, 5);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"document-open");
#else
  button=gtkut_button_new_from_stock(NULL, GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  g_signal_connect (button, "clicked",
  		    G_CALLBACK (edit_ap), (gpointer)hl);

  ////  Flat
  frame = gtkut_frame_new ("<b>Flat image</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(vbox), hbox,FALSE, FALSE, 5);

  hl->label_edit_flat=gtkut_label_new("Flat");
  set_flat_label(hl);

#ifdef USE_GTK3
  gtk_widget_set_halign (hl->label_edit_flat, GTK_ALIGN_START);
  gtk_widget_set_valign (hl->label_edit_flat, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hl->label_edit_flat), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), hl->label_edit_flat, FALSE, FALSE, 5);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"document-open");
#else
  button=gtkut_button_new_from_stock(NULL, GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  g_signal_connect (button, "clicked",
  		    G_CALLBACK (edit_flat), (gpointer)hl);

  ////  ThAr
  frame = gtkut_frame_new ("<b>Wavelength reference</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(vbox), hbox,FALSE, FALSE, 5);

  hl->label_edit_thar=gtkut_label_new("ThAr");
  set_thar_label(hl);

#ifdef USE_GTK3
  gtk_widget_set_halign (hl->label_edit_thar, GTK_ALIGN_START);
  gtk_widget_set_valign (hl->label_edit_thar, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hl->label_edit_thar), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), hl->label_edit_thar, FALSE, FALSE, 5);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"document-open");
#else
  button=gtkut_button_new_from_stock(NULL, GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  g_signal_connect (button, "clicked",
  		    G_CALLBACK (edit_thar), (gpointer)hl);

  
  //// Mask
  frame = gtkut_frame_new ("<b> Mask</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(vbox), hbox,FALSE, FALSE, 5);

  hl->label_edit_mask=gtkut_label_new("Mask");
  set_mask_label(hl);

#ifdef USE_GTK3
  gtk_widget_set_halign (hl->label_edit_mask, GTK_ALIGN_START);
  gtk_widget_set_valign (hl->label_edit_mask, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hl->label_edit_mask), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), hl->label_edit_mask, FALSE, FALSE, 5);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"document-open");
#else
  button=gtkut_button_new_from_stock(NULL, GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  g_signal_connect (button, "clicked",
  		    G_CALLBACK (edit_mask), (gpointer)hl);


  //// Blaze
  frame = gtkut_frame_new ("<b> Blaze Function</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(vbox), hbox,FALSE, FALSE, 5);

  hl->label_edit_blaze=gtkut_label_new("Blaze Function");
  set_blaze_label(hl);

#ifdef USE_GTK3
  gtk_widget_set_halign (hl->label_edit_blaze, GTK_ALIGN_START);
  gtk_widget_set_valign (hl->label_edit_blaze, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hl->label_edit_blaze), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), hl->label_edit_blaze, FALSE, FALSE, 5);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"document-open");
#else
  button=gtkut_button_new_from_stock(NULL, GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  g_signal_connect (button, "clicked",
  		    G_CALLBACK (edit_blaze), (gpointer)hl);


  gtk_widget_show_all(dialog);

  gtk_dialog_run(GTK_DIALOG(dialog));
  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
}


