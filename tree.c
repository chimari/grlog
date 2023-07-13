#include "main.h"

static void frame_tree_add_columns();
static GtkTreeModel *frame_tree_create_items_model();
static void focus_frame_tree_item ();
void qlr_cell_data_func();
void frame_tree_cell_data_func();
static void cell_editing();
static void cell_canceled();
static void cell_edited();

extern pid_t http_pid;
extern gboolean flag_make_frame_tree;
extern gboolean Flag_tree_editing;

void tree_update_frame (typHLOG *hl)
{
  int i_list;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gint i;

  //if(!Flag_tree_editing){
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
    if(!gtk_tree_model_get_iter_first(model, &iter)){
      return;
    }

    
    for(i_list=0;i_list<hl->num;i_list++){
      //  gtk_tree_model_get (model, &iter, COLUMN_FRAME_NUMBER, &i, -1);
      //i--;
      frame_tree_update_item(hl, GTK_TREE_MODEL(model), iter, i_list);
      if(!gtk_tree_model_iter_next(model, &iter)) break;
    }
    //}
}

void move_focus_item(typHLOG *hl, gint i_set){
  gint i, i_list;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreePath *path;
  GtkTreeIter  iter;
  
  path=gtk_tree_path_new_first();
  
  for(i=0;i<hl->num;i++){
    gtk_tree_model_get_iter (model, &iter, path);
    gtk_tree_model_get (model, &iter, COLUMN_FRAME_NUMBER, &i_list, -1);
    i_list--;
    
    if(i_list==i_set){
      gtk_tree_view_set_cursor(GTK_TREE_VIEW(hl->frame_tree), path, NULL, FALSE);
      break;
    }
    else{
      gtk_tree_path_next(path);
    }
  }
  gtk_tree_path_free(path);
}


void make_frame_tree(typHLOG *hl){
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *sw;
  GtkWidget *button;
  GtkTreeModel *items_model;
  
  if(flag_make_frame_tree){
    gtk_widget_destroy(hl->frame_tree);
  }
  else flag_make_frame_tree=TRUE;

  items_model = frame_tree_create_items_model (hl);

  /* create tree view */
  hl->frame_tree = gtk_tree_view_new_with_model (items_model);
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (hl->frame_tree)),
			       GTK_SELECTION_MULTIPLE);
  frame_tree_add_columns (hl, GTK_TREE_VIEW (hl->frame_tree), 
		       items_model);

  g_object_unref(items_model);
  
  gtk_container_add (GTK_CONTAINER (hl->scrwin), hl->frame_tree);

  g_signal_connect (hl->frame_tree, "cursor-changed",
		    G_CALLBACK (focus_frame_tree_item), (gpointer)hl);

  gtk_widget_show_all(hl->frame_tree);
}


static void frame_tree_add_columns (typHLOG *hl,
			     GtkTreeView  *treeview, 
			     GtkTreeModel *items_model)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;  

  /* number column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_FRAME_NUMBER));
  column=gtk_tree_view_column_new_with_attributes ("No.",
						   renderer,
						   "text",
						   COLUMN_FRAME_NUMBER,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_FRAME_COLFG,
						   "background-rgba", COLUMN_FRAME_COLBG,
#else
						   "foreground-gdk", COLUMN_FRAME_COLFG,
						   "background-gdk", COLUMN_FRAME_COLBG,
#endif
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  frame_tree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FRAME_NUMBER),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_FRAME_NUMBER);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Frame ID column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_FRAME_ID));
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text", 
						   COLUMN_FRAME_ID,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_FRAME_COLFG,
						   "background-rgba", COLUMN_FRAME_COLBG,
#else
						   "foreground-gdk", COLUMN_FRAME_COLFG,
						   "background-gdk", COLUMN_FRAME_COLBG,
#endif
						   
						   NULL);
  gtkut_tree_view_column_set_markup(column, "Frame ID");
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  frame_tree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FRAME_ID),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_FRAME_ID);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Object Name column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_FRAME_NAME));
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text", 
						   COLUMN_FRAME_NAME,
						   "weight", COLUMN_FRAME_WEIGHT,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_FRAME_COLFG,
						   "background-rgba", COLUMN_FRAME_COLBG,
#else
						   "foreground-gdk", COLUMN_FRAME_COLFG,
						   "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						   NULL);
  gtkut_tree_view_column_set_markup(column, "Object Name");
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  frame_tree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FRAME_NAME),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Quick Look column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_FRAME_QLR));
  
  column = gtk_tree_view_column_new_with_attributes (NULL,
						     renderer,
						     "markup", 
						     COLUMN_FRAME_QLR,
#ifdef USE_GTK3
						     "foreground-rgba", COLUMN_FRAME_COLFG,
						     "background-rgba", COLUMN_FRAME_COLBG,
#else
						     "foreground-gdk", COLUMN_FRAME_COLFG,
						     "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						     NULL);
  gtkut_tree_view_column_set_markup(column, "QL");
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  qlr_cell_data_func,
					  (gpointer)hl,
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* CAL column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_FRAME_CAL));
  
  column = gtk_tree_view_column_new_with_attributes (NULL,
						     renderer,
						     "markup", 
						     COLUMN_FRAME_CAL,
#ifdef USE_GTK3
						     "foreground-rgba", COLUMN_FRAME_COLFG,
						     "background-rgba", COLUMN_FRAME_COLBG,
#else
						     "foreground-gdk", COLUMN_FRAME_COLFG,
						     "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						     NULL);
  gtkut_tree_view_column_set_markup(column, "CAL");
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  frame_tree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FRAME_CAL),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Prop-ID column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_FRAME_PROP));
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text", 
						   COLUMN_FRAME_PROP,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_FRAME_COLFG,
						   "background-rgba", COLUMN_FRAME_COLBG,
#else
						   "foreground-gdk", COLUMN_FRAME_COLFG,
						   "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						   NULL);
  gtkut_tree_view_column_set_markup(column, "Prop-ID");
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  frame_tree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FRAME_PROP),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Observer column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_FRAME_OBSERVER));
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text", 
						   COLUMN_FRAME_OBSERVER,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_FRAME_COLFG,
						   "background-rgba", COLUMN_FRAME_COLBG,
#else
						   "foreground-gdk", COLUMN_FRAME_COLFG,
						   "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						   NULL);
  gtkut_tree_view_column_set_markup(column, "Observer");
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  frame_tree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FRAME_OBSERVER),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  /* Date column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_FRAME_DATE));
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text", 
						   COLUMN_FRAME_DATE,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_FRAME_COLFG,
						   "background-rgba", COLUMN_FRAME_COLBG,
#else
						   "foreground-gdk", COLUMN_FRAME_COLFG,
						   "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						   NULL);
  gtkut_tree_view_column_set_markup(column, "DATE<sub>JST</sub>");
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  frame_tree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FRAME_DATE),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);

  switch(hl->disp_time){
  case DISP_TIME_JST:
    /* JST column */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FRAME_JST));
    column=gtk_tree_view_column_new_with_attributes (NULL,
						     renderer,
						     "text", 
						     COLUMN_FRAME_JST,
#ifdef USE_GTK3
						     "foreground-rgba", COLUMN_FRAME_COLFG,
						     "background-rgba", COLUMN_FRAME_COLBG,
#else
						     "foreground-gdk", COLUMN_FRAME_COLFG,
						     "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						     NULL);
    gtkut_tree_view_column_set_markup(column, "JST<sub>start</sub>");
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    frame_tree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FRAME_JST),
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    break;

  case DISP_TIME_UT:
    /* UT column */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FRAME_UT));
    column=gtk_tree_view_column_new_with_attributes (NULL,
						     renderer,
						     "text", 
						     COLUMN_FRAME_UT,
#ifdef USE_GTK3
						     "foreground-rgba", COLUMN_FRAME_COLFG,
						     "background-rgba", COLUMN_FRAME_COLBG,
#else
						     "foreground-gdk", COLUMN_FRAME_COLFG,
						     "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						     NULL);
    gtkut_tree_view_column_set_markup(column, "UT<sub>start</sub>");
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    frame_tree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FRAME_UT),
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    break;
    
    /* MJD column */
  case DISP_TIME_MJD:
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FRAME_MJD));
    column=gtk_tree_view_column_new_with_attributes (NULL,
						     renderer,
						     "text", 
						     COLUMN_FRAME_MJD,
#ifdef USE_GTK3
						     "foreground-rgba", COLUMN_FRAME_COLFG,
						     "background-rgba", COLUMN_FRAME_COLBG,
#else
						     "foreground-gdk", COLUMN_FRAME_COLFG,
						     "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						     NULL);
    gtkut_tree_view_column_set_markup(column, "MJD<sub>start</sub>");
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    frame_tree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FRAME_MJD),
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
    break;    
  }


  /* EXPTIME column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_FRAME_EXPTIME));
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text", 
						   COLUMN_FRAME_EXPTIME,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_FRAME_COLFG,
						   "background-rgba", COLUMN_FRAME_COLBG,
#else
						   "foreground-gdk", COLUMN_FRAME_COLFG,
						   "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						   NULL);
  gtkut_tree_view_column_set_markup(column, "Exp.");
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  frame_tree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FRAME_EXPTIME),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* SecZ column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_FRAME_SECZ));
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text", 
						   COLUMN_FRAME_SECZ,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_FRAME_COLFG,
						   "background-rgba", COLUMN_FRAME_COLBG,
#else
						   "foreground-gdk", COLUMN_FRAME_COLFG,
						   "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						   NULL);
  gtkut_tree_view_column_set_markup(column, "<i>sec</i> <i>Z</i>");
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  frame_tree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FRAME_SECZ),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* I2 */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
		     GINT_TO_POINTER (COLUMN_FRAME_I2));
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text", 
						   COLUMN_FRAME_I2,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_FRAME_COLFG,
						   "background-rgba", COLUMN_FRAME_COLBG,
#else
						   "foreground-gdk", COLUMN_FRAME_COLFG,
						   "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						   NULL);
  gtkut_tree_view_column_set_markup(column, "I<sub>2</sub>-cell");
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					    frame_tree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FRAME_I2),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Count */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_FRAME_COUNT));
  g_object_set (renderer,
                "editable", TRUE,
                NULL);
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "markup", 
						   COLUMN_FRAME_COUNT,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_FRAME_COLFG,
						   "background-rgba", COLUMN_FRAME_COLBG,
#else
						   "foreground-gdk", COLUMN_FRAME_COLFG,
						   "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						   NULL);
  gtkut_tree_view_column_set_markup(column, "<i>e</i><sup>-</sup>");
  g_signal_connect (renderer, "editing_started",
		    G_CALLBACK (cell_editing), NULL);
  g_signal_connect (renderer, "editing_canceled",
		    G_CALLBACK (cell_canceled), NULL);
  g_signal_connect (renderer, "edited", G_CALLBACK (cell_edited), hl);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  frame_tree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FRAME_COUNT),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Note */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_FRAME_NOTE));
  g_object_set (renderer,
                "editable", TRUE,
                NULL);
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text", 
						   COLUMN_FRAME_NOTE,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_FRAME_COLFG,
						   "background-rgba", COLUMN_FRAME_COLBG,
#else
						   "foreground-gdk", COLUMN_FRAME_COLFG,
						   "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						   NULL);
  gtkut_tree_view_column_set_markup(column, "Note <span size=\"smaller\">(<i>editable</i>)</span>");
  g_signal_connect (renderer, "editing_started",
		    G_CALLBACK (cell_editing), NULL);
  g_signal_connect (renderer, "editing_canceled",
		    G_CALLBACK (cell_canceled), NULL);
  g_signal_connect (renderer, "edited", G_CALLBACK (cell_edited), hl);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  frame_tree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FRAME_NOTE),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
}


static GtkTreeModel *
frame_tree_create_items_model (typHLOG *hl)
{
  gint i_frm = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  /* create list store */
  model = gtk_list_store_new (NUM_FRAME_COLUMNS,
			      G_TYPE_INT,      //COLUMN_FRAME_NUMBER,
			      G_TYPE_STRING,   //COLUMN_FRAME_ID,
			      G_TYPE_STRING,   //COLUMN_FRAME_NAME,
			      G_TYPE_STRING,   //COLUMN_FRAME_TYPE,
			      G_TYPE_INT,      //COLUMN_FRAME_QLR,
			      G_TYPE_INT,      //COLUMN_FRAME_CAL,
			      G_TYPE_STRING,   //COLUMN_FRAME_PROP,
			      G_TYPE_STRING,   //COLUMN_FRAME_OBSERVER,
			      G_TYPE_STRING,   //COLUMN_FRAME_DATE,
			      G_TYPE_STRING,   //COLUMN_FRAME_JST,
			      G_TYPE_STRING,   //COLUMN_FRAME_UT,
			      G_TYPE_DOUBLE,   //COLUMN_FRAME_MJD,
			      G_TYPE_INT,      //COLUMN_FRAME_EXPTIME,
			      G_TYPE_DOUBLE,   //COLUMN_FRAME_SECZ,
			      G_TYPE_STRING,   //COLUMN_FRAME_I2,
			      G_TYPE_INT,      //COLUMN_FRAME_COUNT,
			      G_TYPE_STRING,   //COLUMN_FRAME_NOTE,
#ifdef USE_GTK3
			      GDK_TYPE_RGBA,   //fgcolor
			      GDK_TYPE_RGBA,   //bgcolor
#else
			      GDK_TYPE_COLOR,  //fgcolor
			      GDK_TYPE_COLOR,  //bgcolor
#endif
			      G_TYPE_INT      // weight
			      );

  for (i_frm = 0; i_frm < hl->num; i_frm++){
    gtk_list_store_append (model, &iter);
    frame_tree_update_item(hl, GTK_TREE_MODEL(model), iter, i_frm);
  }
 
  return GTK_TREE_MODEL (model);
}


void frame_tree_update_item(typHLOG *hl, 
			    GtkTreeModel *model, 
			    GtkTreeIter iter, 
			    gint i_frm)
{
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_FRAME_NUMBER, i_frm,
		      COLUMN_FRAME_ID,     hl->frame[i_frm].id,
		      COLUMN_FRAME_NAME,   hl->frame[i_frm].name,
		      COLUMN_FRAME_TYPE,   hl->frame[i_frm].type,
		      COLUMN_FRAME_QLR,    hl->frame[i_frm].qlr,
		      COLUMN_FRAME_CAL,    hl->frame[i_frm].cal,
		      COLUMN_FRAME_PROP,   hl->frame[i_frm].prop,
		      COLUMN_FRAME_OBSERVER, hl->frame[i_frm].observer,
		      COLUMN_FRAME_DATE,   hl->frame[i_frm].date,
		      COLUMN_FRAME_JST,    hl->frame[i_frm].jst,
		      COLUMN_FRAME_UT,     hl->frame[i_frm].ut,
		      COLUMN_FRAME_MJD,    hl->frame[i_frm].mjd,
		      COLUMN_FRAME_EXPTIME,hl->frame[i_frm].exp,
		      COLUMN_FRAME_SECZ,   hl->frame[i_frm].secz,
		      COLUMN_FRAME_I2,     hl->frame[i_frm].i2,
		      COLUMN_FRAME_COUNT,  hl->frame[i_frm].note.cnt,
		      COLUMN_FRAME_NOTE,   hl->frame[i_frm].note.txt,
		      COLUMN_FRAME_COLBG, (i_frm%2==0) ? &color_white : &color_lgreen,
		      -1);

  if(strcmp(hl->frame[i_frm].type,"FLAT")==0){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_COLFG, &color_flat, -1);
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_WEIGHT,
			PANGO_WEIGHT_NORMAL,
			-1);
  }
  else if(strcmp(hl->frame[i_frm].type,"COMPARISON")==0){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_COLFG, &color_comp, -1);
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_WEIGHT,
			PANGO_WEIGHT_NORMAL,
			-1);
  }
  else if(strcmp(hl->frame[i_frm].type,"BIAS")==0){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_COLFG, &color_bias, -1);
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_WEIGHT,
			PANGO_WEIGHT_NORMAL,
			-1);
  }
  else if(strcmp(hl->frame[i_frm].type,"OBJECT")==0){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_COLFG, &color_black, -1);
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_WEIGHT,
			PANGO_WEIGHT_BOLD,
			-1);
  }
  else{
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_COLFG, &color_black, -1);
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_WEIGHT,
			PANGO_WEIGHT_NORMAL,
			-1);
  }
}

/*
void frame_tree_update_note(typHLOG *hl, 
			    GtkTreeModel *model, 
			    GtkTreeIter iter, 
			    gint i_frm)
{
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      -1);
}
*/

void frame_tree_get_selected (GtkTreeModel *model, GtkTreePath *path, 
		    GtkTreeIter *iter, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;
  gint i;
    
  gtk_tree_model_get (model, iter, COLUMN_FRAME_NUMBER, &i, -1);
  hl->frame_tree_i=i;
}

static void
focus_frame_tree_item (GtkWidget *widget, gpointer data)
{
  typHLOG *hl = (typHLOG *)data;
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(hl->frame_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hl->frame_tree));
  gint i_rows;

  i_rows=gtk_tree_selection_count_selected_rows (selection);

  if(i_rows==1){
    gtk_tree_selection_selected_foreach (selection, frame_tree_get_selected, (gpointer)hl);
  }
  else{
    hl->frame_tree_i=0;
  }
}


void qlr_cell_data_func(GtkTreeViewColumn *col , 
			GtkCellRenderer *renderer,
			GtkTreeModel *model, 
			GtkTreeIter *iter,
			gpointer user_data)
{
  gchar *str=NULL;
  gint i_buf;
  gint i;
  typHLOG *hl=(typHLOG *) user_data;
  
  gtk_tree_model_get (model, iter, 
		      COLUMN_FRAME_NUMBER, &i,
		      -1);
  i++;
  
  gtk_tree_model_get (model, iter, 
		      COLUMN_FRAME_QLR, &i_buf,
		      -1);

  switch(i_buf){
  case QLR_DONE:
    str=g_strdup("&#x25CB;"); // o
    break;
    
  case QLR_NOW:
    str=g_strdup("&#x2605;"); // star
    break;

  default:
    str=g_strdup("&#x2015;");  // x
    break;
  }
  
  g_object_set(renderer, "markup", str, NULL);
  if(str) g_free(str);
}



void frame_tree_cell_data_func(GtkTreeViewColumn *col , 
			    GtkCellRenderer *renderer,
			    GtkTreeModel *model, 
			    GtkTreeIter *iter,
			    gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
  gint i_buf, i_buf2;
  gdouble d_buf, d_buf2;
  gchar *c_buf, *str, *c_buf2;
  gboolean b_buf;

  switch (index) {
  case COLUMN_FRAME_ID:
  case COLUMN_FRAME_NAME:
  case COLUMN_FRAME_PROP:
  case COLUMN_FRAME_OBSERVER:
  case COLUMN_FRAME_DATE:
  case COLUMN_FRAME_JST:
  case COLUMN_FRAME_UT:
  case COLUMN_FRAME_I2:
  case COLUMN_FRAME_NOTE:
    gtk_tree_model_get (model, iter, 
			index, &c_buf,
			-1);
    break;

  case COLUMN_FRAME_NUMBER:
  case COLUMN_FRAME_EXPTIME:
  case COLUMN_FRAME_COUNT:
  case COLUMN_FRAME_CAL:
    gtk_tree_model_get (model, iter, 
			index, &i_buf,
			-1);
    break;

  case COLUMN_FRAME_SECZ:
  case COLUMN_FRAME_MJD:
    gtk_tree_model_get (model, iter, 
			index, &d_buf,
			-1);
    break;

  case COLUMN_FRAME_QLR:
    gtk_tree_model_get (model, iter, 
			index, &i_buf,
			-1);
    break;
  }

  switch (index) {
  case COLUMN_FRAME_I2:
    if(strcmp(c_buf,"UNKNOWN")==0){
      str=g_strdup("---");
    }
    else{
      str=g_strdup(c_buf);
    }
    break;

  case COLUMN_FRAME_NUMBER:
    str=g_strdup_printf("%d",i_buf+1);
    break;

  case COLUMN_FRAME_EXPTIME:
    str=g_strdup_printf("%ds",i_buf);
    break;

  case COLUMN_FRAME_COUNT:
    if(i_buf>0){
        str=g_strdup_printf("%d",i_buf);
    }
    else{
      str=NULL;
    }
    break;

  case COLUMN_FRAME_SECZ:
    str=g_strdup_printf("%.2lf",d_buf);
    break;
    
  case COLUMN_FRAME_MJD:
    str=g_strdup_printf("%.4lf",d_buf);
    break;

  case COLUMN_FRAME_CAL:
    switch(i_buf){
    case QLCAL_FLAT:
      str=g_strdup("<b>F</b>");
      break;
    case QLCAL_FLAT0:
      str=g_strdup("f");
      break;
    case QLCAL_COMP:
      str=g_strdup("<b>C</b>");
      break;
    case QLCAL_COMP0:
      str=g_strdup("c");
      break;
    default:
      str=NULL;
      break;
    }
    break;

  case COLUMN_FRAME_QLR:
    switch(i_buf){
    case QLR_DONE:
      str=g_strdup("&#x25CB;");
      break;

    case QLR_NOW:
      str=g_strdup("&#x2605;");
      break;

    default:
      str=g_strdup("&#x2015;");
      break;
    }
    break;

  default:
    str=g_strdup(c_buf);
    break;
  }

  switch (index) {
  case COLUMN_FRAME_QLR:
  case COLUMN_FRAME_COUNT:
  case COLUMN_FRAME_CAL:
    g_object_set(renderer, "markup", str, NULL);
    break;

  default:
    g_object_set(renderer, "text", str, NULL);
    break;
  }

  if(str)g_free(str);
}



static void cell_editing (GtkCellRendererText *cell)
{
  Flag_tree_editing=TRUE;
}

static void cell_canceled (GtkCellRendererText *cell)
{
  Flag_tree_editing=FALSE;
}

void update_seimei_log(typHLOG *hl, gint i){
  if(!hl->push_flag) return;
  
  if(hl->seimei_log_id) g_free(hl->seimei_log_id);
  hl->seimei_log_id=g_strdup(hl->frame[i].id);
  
  if(hl->seimei_log_txt) g_free(hl->seimei_log_txt);
  if(hl->frame[i].note.txt){
    if(hl->frame[i].note.cnt>0){
      hl->seimei_log_txt=g_strdup_printf("%s, %de- at order=%d x=%d",
					 hl->frame[i].note.txt,
					 hl->frame[i].note.cnt,
					 hl->ql_ge_line,
					 (hl->ql_ge_stx+hl->ql_ge_edx)/2);
   }
    else{
      hl->seimei_log_txt=g_strdup(hl->frame[i].note.txt);
    }
  }
  else{
    if(hl->frame[i].note.cnt>0){
      hl->seimei_log_txt=g_strdup_printf("%de- at order=%d x=%d",
					 hl->frame[i].note.cnt,
					 hl->ql_ge_line,
					 (hl->ql_ge_stx+hl->ql_ge_edx)/2);
    }
    else{
      hl->seimei_log_txt=g_strdup(" ");
    }
  }
  
  http_c_fcdb_new(hl, FALSE, FALSE);
}

static void cell_edited (GtkCellRendererText *cell,
			 const gchar         *path_string,
			 const gchar         *new_text,
			 gpointer             data)
{
  typHLOG *hl = (typHLOG *)data;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  gint column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (cell), "column"));
  gchar tmp[128];

  gtk_tree_model_get_iter (model, &iter, path);

  switch (column) {
  case COLUMN_FRAME_NOTE:
    {
      gint i;
      gchar *old_text;
      
      gtk_tree_model_get (model, &iter, column, &old_text, -1);
      g_free (old_text);
      
      gtk_tree_model_get (model, &iter, COLUMN_FRAME_NUMBER, &i, -1);
      
      if(hl->frame[i].note.txt) g_free(hl->frame[i].note.txt);
      hl->frame[i].note.txt=g_strdup(new_text);
      gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
			  hl->frame[i].note.txt, -1);
      
      if(hl->frame[i].note.auto_fl){
	hl->frame[i].note.auto_fl=FALSE;
      }
      else{
	hl->frame[i].note.time=time(NULL);
      }
      save_note(hl);

      update_seimei_log(hl, i);
    }
    break;

  case COLUMN_FRAME_COUNT:
    {
      gint i;
      gchar *old_text;
      
      gtk_tree_model_get (model, &iter, COLUMN_FRAME_NUMBER, &i, -1);
      
      hl->frame[i].note.cnt=atoi(new_text);
      gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
			  hl->frame[i].note.cnt, -1);
      
      if(hl->frame[i].note.auto_fl){
	hl->frame[i].note.auto_fl=FALSE;
      }
      else{
	hl->frame[i].note.time=time(NULL);
      }
      save_note(hl);

      update_seimei_log(hl, i);
    }
    break;
  }

  Flag_tree_editing=FALSE;
  gtk_tree_path_free (path);
}


void frame_tree_update_ql (typHLOG *hl, gint i_sel){
  GtkTreeModel *model 
    = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreeIter iter;
  GtkTreePath *path;
  gint i_frm;

  path=gtk_tree_path_new_first();

  for(i_frm=0;i_frm<i_sel;i_frm++){
      gtk_tree_path_next(path);
  }
  gtk_tree_model_get_iter (model, &iter, path);

  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_FRAME_QLR, hl->frame[i_sel].qlr, -1);
  if(hl->frame[i_sel].note.cnt>0){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FRAME_COUNT, hl->frame[i_sel].note.cnt, -1);
  }
}

void frame_tree_update_note (typHLOG *hl, gint i_sel){
  GtkTreeModel *model 
    = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreeIter iter;
  GtkTreePath *path;
  gint i_frm;

  path=gtk_tree_path_new_first();

  for(i_frm=0;i_frm<i_sel;i_frm++){
      gtk_tree_path_next(path);
  }
  gtk_tree_model_get_iter (model, &iter, path);

  if(hl->frame[i_sel].note.txt){
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		       COLUMN_FRAME_NOTE, hl->frame[i_sel].note.txt, -1);
    tree_update_frame(hl);
  }
  gtk_tree_path_free (path);
}

void frame_tree_update_cal (typHLOG *hl, gint i_sel){
  GtkTreeModel *model 
    = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreeIter iter;
  GtkTreePath *path;
  gint i_frm;

  path=gtk_tree_path_new_first();

  for(i_frm=0;i_frm<i_sel;i_frm++){
      gtk_tree_path_next(path);
  }
  gtk_tree_model_get_iter (model, &iter, path);

  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
		     COLUMN_FRAME_COUNT, hl->frame[i_sel].cal, -1);
}


void frame_tree_select_last(typHLOG *hl){
  gint i, i_frame;
  GtkTreeModel *model 
    = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreePath *path;
  GtkTreeIter  iter;
  GtkAdjustment *adj;

  if(Flag_tree_editing){
    return;
  }
  
  path=gtk_tree_path_new_first();

  if(hl->sort_flag==SORT_ASCENDING){
    for(i=0;i<hl->num-1;i++){
      gtk_tree_path_next(path);
    }
  }
  gtk_tree_view_set_cursor(GTK_TREE_VIEW(hl->frame_tree), 
			   path, NULL, FALSE);
  gtk_tree_path_free(path);

  adj=gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(hl->scrwin));
  if(hl->sort_flag==SORT_ASCENDING){
    gtk_adjustment_set_value(adj,
			     gtk_adjustment_get_upper(adj)
			     -gtk_adjustment_get_page_size(adj));
  }
  else{
    gtk_adjustment_set_value(adj,0);
  }
}

