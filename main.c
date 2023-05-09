//    HDS LOG Editor
//   
//                                           2005.12.08  A.Tajitsu

#include"main.h"    // 設定ヘッダ
#include"version.h"
#include"configfile.h"

#undef DEBUG

gboolean flag_make_top=FALSE;
gboolean flagChildDialog=FALSE;
GtkWidget *frame_table;
guint entry_height=24;
gboolean e_init_flag=FALSE;

void write_muttrc();
void write_msmtprc();

void ChildTerm();
static void cc_get_note ();

void refresh_table ();
gboolean create_lock ();
static void remove_lock ();
static void wait_lock ();
static void load_note ();

void update_frame_tree();
int printfits();
void ext_play();
gint scan_command();
gint printdir();
void gui_init();
void splot_help();
void show_version();

gboolean check_scan ();

void do_save();

// Ya is temporary (using Yb setting)
const SetupEntry setups[] = {
  {"Ub",  "BLUE",  17100}, 
  {"Ua",  "BLUE",  17820}, 
  {"Ba",  "BLUE",  19260}, 
  {"Bc",  "BLUE",  19890}, 
  {"Ya",  "BLUE",  21960}, 
  {"I2b", "RED",   14040}, 
  {"Yd",  "RED",   15480}, 
  {"Yb",  "RED",   15730}, 
  {"Yc",  "RED",   16500}, 
  {"I2a", "RED",   18000}, 
  {"Ra",  "RED",   18455}, 
  {"Rb",  "RED",   19080}, 
  {"NIRc","RED",   21360}, 
  {"NIRb","RED",   22860}, 
  {"NIRa","RED",   25200}, 
  {"Ha",  "MIRROR",0}
};


pid_t http_pid;
gboolean flag_make_frame_tree;
gboolean Flag_tree_editing;
gboolean upd0;

void check_reduced_spectra(typHLOG *hl){
  gint i;
  gchar *tmp;

  for(i=0;i<hl->num;i++){
    tmp=g_strdup_printf("%s%sG%08docs_ecfw.fits",
			hl->wdir,
			G_DIR_SEPARATOR_S,
			hl->frame[i].idnum);
    if(access(tmp,F_OK)==0){
      hl->frame[i].qlr=TRUE;
    }
    else{
      hl->frame[i].qlr=FALSE;
    }
    hl->frame[i].note.cnt=get_cnt(hl, i);
  }
}

void copy_file(gchar *src, gchar *dest)
{
  FILE *src_fp, *dest_fp;
  gchar *buf;
  gint n_read;

  if(strcmp(src,dest)==0) return;
  
  buf=g_malloc0(sizeof(gchar)*1024);


  if ((src_fp = fopen(src, "rb")) == NULL) {
    g_print("Cannot open copy source file %s",src);
    exit(1);
  }

  if ((dest_fp = fopen(dest, "wb")) == NULL) {
    g_print("Cannot open copy destination file %s",dest);
    exit(1);
  }

  g_print("Copying \"%s\" --> \"%s\" ...\n", src,dest);
  while (!feof(src_fp)){
    n_read = fread(buf, sizeof(gchar), sizeof(buf), src_fp);
    fwrite(buf, n_read, 1, dest_fp);
  }
  fclose(dest_fp);
  fclose(src_fp);

#ifndef USE_WIN32
  chmod(dest, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
#endif

  g_free(buf);
}


void init_frame(FRAMEpara *frame){
  if(frame->id) g_free(frame->id);
  frame->id=NULL;

  if(frame->prop) g_free(frame->prop);
  frame->prop=NULL;
  
  if(frame->observer) g_free(frame->observer);
  frame->observer=NULL;
  
  if(frame->name) g_free(frame->name);
  frame->name=NULL;
  
  if(frame->type) g_free(frame->type);
  frame->type=NULL;
  
  if(frame->jst) g_free(frame->jst);
  frame->jst=NULL;
  
  if(frame->ut) g_free(frame->ut);
  frame->ut=NULL;
  
  if(frame->date) g_free(frame->date);
  frame->date=NULL;

  if(frame->i2) g_free(frame->i2);
  frame->i2=NULL;
  
  if(frame->note.txt) g_free(frame->note.txt);
  frame->note.txt=NULL;

  frame->note.time=0;
  frame->note.auto_fl=FALSE;
  frame->note.cnt=-1;
}

void get_jst_day(gint *year, gint *mon, gint *mday){
  struct tm t, *lt;
  time_t timer;
   
  t.tm_year=*year-1900;
  t.tm_mon=*mon-1;
  t.tm_mday=*mday;
  t.tm_hour=0;
  t.tm_min=0;
  t.tm_sec=0;
  t.tm_isdst=0;
  
  timer=mktime(&t);
  timer+=9*60*60;
  
  lt=gmtime(&timer);
  
  *year=lt->tm_year+1900;
  *mon =lt->tm_mon+1;
  *mday=lt->tm_mday;
}



gchar* to_utf8(gchar *input){
  gchar *ret;
  ret=g_locale_to_utf8(input,-1,NULL,NULL,NULL);
  if(!ret) ret=g_strdup(input);
  return(ret);
}

gchar* to_locale(gchar *input){
  gchar *ret;
#ifdef USE_WIN32
  ret=g_win32_locale_filename_from_utf8(input);
  //return(x_locale_from_utf8(input,-1,NULL,NULL,NULL,"SJIS"));
#else
  ret=g_locale_from_utf8(input,-1,NULL,NULL,NULL);
#endif
  if(!ret) ret=g_strdup(input);
  return(ret);
}

gboolean my_main_iteration(gboolean may_block){
  return(g_main_context_iteration(NULL, may_block));
}

void write_muttrc(){
  gchar *filename;
  FILE *fp;
  gint i=0;

  filename=g_strconcat(g_get_home_dir(),G_DIR_SEPARATOR_S,
		       MUTT_FILE, NULL);
  if(access(filename, F_OK)==0){
    g_free(filename);
    return;
  }

  fprintf(stderr," Creating MUTTRC file, \"%s\" .\n", filename);

  if((fp=fopen(filename,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", filename);
    exit(1);
  }
  
  while(muttrc_str[i]){
    fprintf(fp, "%s\n", muttrc_str[i]);
    i++;
  }

  fclose(fp);
  g_free(filename);
}

void write_msmtprc(){
  gchar *filename;
  FILE *fp;
  gint i=0;

  filename=g_strconcat(g_get_home_dir(),G_DIR_SEPARATOR_S,
		       MSMTP_FILE, NULL);
  if(access(filename, F_OK)==0){
    g_free(filename);
    return;
  }

  fprintf(stderr," Creating MSMTPRC file, \"%s\" .\n", filename);

  if((fp=fopen(filename,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", filename);
    exit(1);
  }
  
  while(msmtprc_str[i]){
    fprintf(fp, "%s\n", msmtprc_str[i]);
    i++;
  }

  fclose(fp);

  if((chmod(filename, (S_IRUSR | S_IWUSR)))!=0){
    fprintf(stderr," Cannot chmod MSMTPRC file, \"%s\" .\n", filename);
  }
  g_free(filename);
}

gchar *fgets_new(FILE *fp){
  gint c;
  gint i=0, j=0;
  gchar *dbuf=NULL;

  do{
    i=0;
    while(!feof(fp)){
      c=fgetc(fp);
      if((c==0x00)||(c==0x0a)||(c==0x0d)) break;
      i++;
    }
  }while((i==0)&&(!feof(fp)));
  if(feof(fp)){
    if(fseek(fp,(long)(-i+1),SEEK_CUR)!=0) return(NULL);
  }
  else{
    if(fseek(fp,(long)(-i-1),SEEK_CUR)!=0) return(NULL);
  }

  if((dbuf = (gchar *)g_malloc(sizeof(gchar)*(i+2)))==NULL){
    fprintf(stderr, "!!! Memory allocation error in fgets_new().\n");
    fflush(stderr);
    return(NULL);
  }
  if(fread(dbuf,1, i, fp)){
    while( (c=fgetc(fp)) !=EOF){
      if((c==0x00)||(c==0x0a)||(c==0x0d))j++;
      else break;
    }
    if(c!=EOF){
      if(fseek(fp,-1L,SEEK_CUR)!=0) return(NULL);
    }
    dbuf[i]=0x00;
    //printf("%s\n",dbuf);
    return(dbuf);
  }
  else{
    return(NULL);
  }
  
}


void ChildTerm(int dummy){
  int s;

  wait(&s);
  signal(SIGCHLD, ChildTerm);
}

static void close_child_dialog(GtkWidget *w, GtkWidget *dialog)
{
  //gdk_pointer_ungrab(GDK_CURRENT_TIME);

  gtk_main_quit();
}


static void close_dialog(GtkWidget *w, GtkWidget *dialog)
{
  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
}


void cc_get_adj (GtkWidget *widget, gint * gdata)
{
  *gdata=(int)gtk_adjustment_get_value(GTK_ADJUSTMENT(widget));
}

void cc_get_adj_double (GtkWidget *widget, gdouble * gdata)
{
  *gdata=gtk_adjustment_get_value(GTK_ADJUSTMENT(widget));
}

void cc_get_entry (GtkWidget *widget, gchar **gdata)
{
  g_free(*gdata);
  *gdata=g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));
}

void cc_get_dir (GtkWidget *widget, gchar **gdata)
{
  g_free(*gdata);
  *gdata=g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget)));
}

static void cc_get_note (GtkWidget *widget, gpointer gdata)
{
  NOTEpara *nt;

  nt=(NOTEpara *)gdata;

  g_free(nt->txt);
  nt->txt=g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));
  if(nt->auto_fl){
    nt->auto_fl=FALSE;
  }
  else{
    nt->time=time(NULL);
  }
}

void cc_get_toggle (GtkWidget * widget, gboolean * gdata)
{
  *gdata=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

void cc_toggle_update (GtkWidget * widget, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;
  
  hl->upd_flag=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  refresh_table(widget, (gpointer)hl);

}


void cc_toggle_date_flag (GtkWidget * widget, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;

  hl->date_flag=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  refresh_table(widget, (gpointer)hl);
}

void cc_get_disp_flag (GtkWidget * widget, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;
  GtkTreeIter iter;
  
  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hl->disp_flag=n;
  }
  
  refresh_table(widget, (gpointer)hl);
}

void cc_get_sort_flag (GtkWidget * widget, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;
  GtkTreeIter iter;
  
  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hl->sort_flag=n;
  }
  
  refresh_table(widget, (gpointer)hl);
}

void cc_get_disp_time (GtkWidget * widget, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;
  GtkTreeIter iter;
  
  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hl->disp_time=n;
  }
  
  refresh_table(widget, (gpointer)hl);
}


void cc_auto_red (GtkWidget * widget, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;

  hl->auto_red=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}


void cc_get_combo_box (GtkWidget *widget,  gint * gdata)
{
  GtkTreeIter iter;
  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    *gdata=n;
  }
}


void refresh_table (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl;
  struct tm tmpt2;
  gint i;
  
  memset(&tmpt2, 0x00, sizeof(struct tm));
  
  hl=(typHLOG *)gdata;

  wait_lock(hl);

  if(hl->upd_flag){
    if(hl->timer!=-1)
      g_source_remove(hl->timer);
    hl->timer=-1;
  }
  

  hl->fr_year=hl->buf_year;
  hl->fr_month=hl->buf_month;
  hl->fr_day=hl->buf_day;

  tmpt2.tm_year=hl->fr_year-1900;
  tmpt2.tm_mon=hl->fr_month-1;
  tmpt2.tm_mday=hl->fr_day;
  tmpt2.tm_hour=9;
  tmpt2.tm_min=0;
  tmpt2.tm_sec=0;

  hl->fr_time=mktime(&tmpt2);
  hl->seek_time=hl->fr_time;
  hl->to_time=hl->fr_time+60*60*24;
    

  for(i=0;i<MAX_FRAME;i++){
    init_frame(&hl->frame[i]);
    //hl->frame[i].note.txt=NULL;
  }
  if(hl->last_frame_id) g_free(hl->last_frame_id);
  hl->last_frame_id=g_strdup("New");
  
  hl->num=0;
  hl->num_old=0;

  hl->i_reduced=0;

  make_frame_tree(hl);
  
  if(hl->upd_flag){
    upd0=FALSE;
    hl->timer=g_timeout_add(CHECK_INTERVAL, 
			    (GSourceFunc)check_scan, 
			    (gpointer)hl);
  }
  else{
    start_scan_command((gpointer)hl);
  }
}

gboolean create_lock (typHLOG *hl){
  gchar lockfile[256];

  if(!hl->upd_flag) return;

  sprintf(lockfile,"%s%sgrlog-%04d%02d%02d-%s.lock",
	  g_get_tmp_dir(),G_DIR_SEPARATOR_S, 
	  hl->fr_year,hl->fr_month,hl->fr_day,hl->uname);
    
  while(1){
    hl->lock_fp=open(lockfile, O_RDWR | O_CREAT | O_EXCL, 0444);
    if (hl->lock_fp == -1){
      printf ("%d - Lock already present   %s\n",getpid(), lockfile);
      gtk_label_set_markup(GTK_LABEL(hl->w_status), 
			   "<span color=\"#FF0000\"><b>File Lock</b></span>");
      while(my_main_iteration(FALSE));
      sleep(1);
    }
    else{
      gtk_label_set_markup(GTK_LABEL(hl->w_status), 
			   "Scanning...");
      while(my_main_iteration(FALSE));
      hl->lock_flag=TRUE;
      break;
    }
  }
  return(TRUE);
}

static void remove_lock (typHLOG *hl){
  gchar lockfile[256];
  sprintf(lockfile,"%s%sgrlog-%04d%02d%02d-%s.lock",
	  g_get_tmp_dir(),G_DIR_SEPARATOR_S, 
	  hl->fr_year,hl->fr_month,hl->fr_day,hl->uname);

  close(hl->lock_fp);
  unlink(lockfile);
  hl->lock_flag=FALSE;
}

static void wait_lock (typHLOG *hl){
  while(hl->lock_flag){
    gtk_label_set_markup(GTK_LABEL(hl->w_status), 
			 "<span color=\"#FF0000\"><b>File Lock</b></span>");
    //while(my_main_iteration(FALSE));
    sleep(1);
  }
}



void save_note (typHLOG *hl)
{
  ConfigFile *cfgfile;
  gchar *filename;
  gint i;

  create_lock(hl);

  filename=g_strdup_printf("%s%s%s%s.grlog-%04d%02d%02d",
			   g_get_home_dir(),G_DIR_SEPARATOR_S,
			   GRLOG_DIR, G_DIR_SEPARATOR_S,
			   hl->fr_year,hl->fr_month,hl->fr_day);
  cfgfile = xmms_cfg_open_file(filename);
  if (!cfgfile)  cfgfile = xmms_cfg_new();

  for(i=0;i<hl->num;i++){
    if(hl->frame[i].note.txt){
      if(hl->frame[i].note.time>=hl->seek_time){
	xmms_cfg_write_string(cfgfile, hl->frame[i].id,
			      "note",hl->frame[i].note.txt);
	xmms_cfg_write_int(cfgfile, hl->frame[i].id,
			   "time",hl->frame[i].note.time);
      }
    }
    if(hl->frame[i].note.cnt>0){
      xmms_cfg_write_int(cfgfile, hl->frame[i].id,
			 "cnt",hl->frame[i].note.cnt);
    }
  }

  xmms_cfg_write_file(cfgfile, filename);
  xmms_cfg_free(cfgfile);
  g_free(filename);

  remove_lock(hl);
}


static void load_note (typHLOG *hl,gboolean force_fl)
{
  ConfigFile *cfgfile;
  gchar filename[256];
  gchar *c_buf;
  gint i, i_buf, i_buf2;
  struct stat statbuf;

  //if(!hl->upd_flag) return;

  sprintf(filename,"%s%s%s%s.grlog-%04d%02d%02d",
	  g_get_home_dir(), G_DIR_SEPARATOR_S,
	  GRLOG_DIR, G_DIR_SEPARATOR_S,
	  hl->fr_year,hl->fr_month,hl->fr_day);

  if (!force_fl){
    stat(filename,&statbuf);
    if((statbuf.st_ctime<hl->seek_time)) return;
  }

  create_lock(hl);

  cfgfile = xmms_cfg_open_file(filename);
  if (cfgfile) {

    for(i=0;i<hl->num;i++){
      if(xmms_cfg_read_int(cfgfile, hl->frame[i].id,
			   "time",&i_buf)){
	if(i_buf>hl->frame[i].note.time){
	  if(xmms_cfg_read_string(cfgfile, hl->frame[i].id,
				  "note",&c_buf)){
	    if(hl->frame[i].note.txt) g_free(hl->frame[i].note.txt);
	    hl->frame[i].note.txt=g_strdup(c_buf);
	    hl->frame[i].note.time=i_buf;
	    if( (hl->frame[i].note.txt) && (!force_fl)){
	      hl->frame[i].note.auto_fl=TRUE;
	      //gtk_entry_set_text(GTK_ENTRY(hl->frame[i].w_note),
	      //		 hl->frame[i].note.txt);
	      //printf("Writing %s  to  %s\n",hl->frame[i].note.txt,
 	      //     hl->frame[i].id);
	    }

	  }
	}
      }
      if(xmms_cfg_read_int(cfgfile, hl->frame[i].id,
			   "cnt",&i_buf2)){
	hl->frame[i].note.cnt=i_buf2;
	hl->frame[i].note.time=i_buf;
	if( (hl->frame[i].note.cnt>0) && (!force_fl)){
	  hl->frame[i].note.auto_fl=TRUE;
	}
      }
    }

    xmms_cfg_free(cfgfile);
  }

  remove_lock(hl);
}


void save_cfg (typHLOG *hl)
{
  ConfigFile *cfgfile;
  gchar *filename;

  filename=g_strdup_printf("%s%s.grlog_conf",
			   g_get_home_dir(),G_DIR_SEPARATOR_S);
  cfgfile = xmms_cfg_open_file(filename);
  if (!cfgfile)  cfgfile = xmms_cfg_new();

  xmms_cfg_write_string(cfgfile, "Directory",
			"Share",hl->sdir);

  xmms_cfg_write_string(cfgfile, "Directory",
			"User",hl->udir);

  xmms_cfg_write_string(cfgfile, "PyRAF",
			"Terminal",hl->ql_terminal);
  
  xmms_cfg_write_string(cfgfile, "PyRAF",
			"Python",hl->ql_python);
  
  xmms_cfg_write_file(cfgfile, filename);
  xmms_cfg_free(cfgfile);
  g_free(filename);
}

static void load_cfg (typHLOG *hl)
{
  ConfigFile *cfgfile;
  gchar *filename;
  gchar *c_buf;

  hl->sdir=NULL;
  hl->udir=NULL;
  hl->ql_terminal=NULL;
  hl->ql_python=NULL;
  
  filename=g_strdup_printf("%s%s.grlog_conf",
			   g_get_home_dir(),G_DIR_SEPARATOR_S);

  cfgfile = xmms_cfg_open_file(filename);
  if (cfgfile) {
    if(xmms_cfg_read_string(cfgfile, "Directory",
			    "Share",&c_buf)){
      hl->sdir=g_strdup(c_buf);
    }
    if(xmms_cfg_read_string(cfgfile, "Directory",
			    "User",&c_buf)){
      hl->udir=g_strdup(c_buf);
    }
      
    if(xmms_cfg_read_string(cfgfile, "PyRAF",
			    "Terminal",&c_buf)){
      hl->ql_terminal=g_strdup(c_buf);
    }
    if(xmms_cfg_read_string(cfgfile, "PyRAF",
			    "Python",&c_buf)){
      hl->ql_python=g_strdup(c_buf);
    }

    xmms_cfg_free(cfgfile);
  }
  g_free(filename);
}


void make_top_table(typHLOG *hl){
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *combo;
  GtkAdjustment *adj;
  GtkWidget *spinner;
  GtkWidget *check;
  GtkWidget *button;
  int col=0;
  

  if(flag_make_top)  gtk_widget_destroy(hl->top_table);
  else flag_make_top=TRUE;


  hl->top_table = gtkut_table_new (1, 2, FALSE, 5, 5, 5);
  
  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtkut_table_attach(hl->top_table, hbox, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("Current/Next : GRA");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  // Next ID
  hl->e_next = gtk_entry_new ();
  gtk_editable_set_editable(GTK_EDITABLE(hl->e_next), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox),hl->e_next,FALSE,FALSE,0);

  Flag_tree_editing=FALSE;
  // Note
  hl->e_note = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),hl->e_note,FALSE,FALSE,0);
  gtk_entry_set_width_chars(GTK_ENTRY(hl->e_note),40);
  g_signal_connect (hl->e_note,
		    "changed",
		    G_CALLBACK(cc_get_entry),
		    &hl->next_note);
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i;
    gchar *tmp;

    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i=0;i<NUM_SCR;i++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 
			 0, scr_name[i],
			 1, i, -1);
      if(hl->scr_flag==i) iter_set=iter;
    }
    
    hl->scr_combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),hl->scr_combo,FALSE,FALSE,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(hl->scr_combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(hl->scr_combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(hl->scr_combo),&iter_set);
    gtk_widget_show(hl->scr_combo);
    g_signal_connect (hl->scr_combo,"changed",G_CALLBACK(cc_get_combo_box),
		       &hl->scr_flag);
  }

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtkut_table_attach(hl->top_table, hbox, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  check = gtk_check_button_new_with_label("Update");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), hl->upd_flag);
  gtk_widget_set_sensitive(check, FALSE);
  //g_signal_connect (check, "toggled",
  //		    G_CALLBACK (cc_toggle_update),
  //		    (gpointer)hl);
  
  check =  gtk_check_button_new_with_label ("filtering by Date (JST)");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), hl->date_flag);
  g_signal_connect (check, "toggled",
		    G_CALLBACK (cc_toggle_date_flag),
		    (gpointer)hl);

  hl->fr_e = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(hbox),hl->fr_e,FALSE,FALSE,0);
  gtk_editable_set_editable(GTK_EDITABLE(hl->fr_e),FALSE);
  gtk_entry_set_width_chars(GTK_ENTRY(hl->fr_e),12);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"go-down");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_DOWN);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
  g_signal_connect(G_OBJECT(button),"pressed",
		   G_CALLBACK(popup_fr_calendar), 
		   (gpointer)hl);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Doublue-Click on calendar to select a new date");
#endif

  set_fr_e_date(hl);

  label = gtk_label_new ("  ");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i;
    gchar *tmp;

    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i=0;i<NUM_DISP_TIME;i++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 
			 0, disp_time_name[i],
			 1, i, -1);
      if(hl->disp_time==i) iter_set=iter;
    }
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    g_signal_connect (combo,"changed",G_CALLBACK(cc_get_disp_time),
		      (gpointer)hl);
  }

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i;
    gchar *tmp;

    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i=0;i<NUM_DISP;i++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 
			 0, disp_name[i],
			 1, i, -1);
      if(hl->disp_flag==i) iter_set=iter;
    }
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    g_signal_connect (combo,"changed",G_CALLBACK(cc_get_disp_flag),
		      (gpointer)hl);
  }

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i;
    gchar *tmp;

    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i=0;i<NUM_SORT;i++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 
			 0, sort_name[i],
			 1, i, -1);
      if(hl->sort_flag==i) iter_set=iter;
    }
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    g_signal_connect (combo,"changed",G_CALLBACK(cc_get_sort_flag),
		      (gpointer)hl);
  }
  

  label = gtk_label_new ("  ");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  
  /*
#ifdef USE_GTK3
  hl->b_refresh=gtkut_button_new_from_icon_name(NULL,"view-refresh");
#else
  hl->b_refresh=gtkut_button_new_from_stock(NULL,GTK_STOCK_REFRESH);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),hl->b_refresh,FALSE,FALSE,0);
  g_signal_connect(hl->b_refresh,"clicked", 
		   G_CALLBACK(refresh_table), 
		   (gpointer)hl);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(hl->b_refresh,
			      "Set Date & flags, then Remake table");
#endif
  */

  hl->w_status = gtkut_label_new ("Starting...");
  while(my_main_iteration(FALSE));
  gtk_box_pack_start(GTK_BOX(hbox),hl->w_status,TRUE,TRUE,0);


  gtk_widget_show_all(hl->top_table);
}


void update_frame_tree(typHLOG *hl, gboolean force_flg){
  int i, col=0;
  gchar *tmp;
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hl->frame_tree));
  gboolean d_flag, i_flag;
  gint t_year, t_mon, t_mday, t_hour, t_min;
  struct tm tmpt2;
  time_t tt;
  gchar odate[32],ojst[32];

  if(Flag_tree_editing) return;

  memset(&tmpt2, 0x00, sizeof(struct tm));
  
#ifdef DEBUG
  fprintf(stderr, "Start Load\n");
#endif
  if((hl->num_old==0)&&(hl->num!=0)){
    // New load
    load_note(hl,TRUE);
  }
  else{
    // No change or Appended New frame
      load_note(hl,FALSE);
  }

#ifdef DEBUG
  fprintf(stderr, "End Load\n");
#endif
  
  // No change
  if(hl->num_old==hl->num){
    if(flag_make_frame_tree){
      if(!gtk_tree_model_get_iter_first(model, &iter)) return;
      
      for(i=0;i<hl->num_old;i++){
	if(hl->frame[i].note.auto_fl){
	  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			     COLUMN_FRAME_NOTE, hl->frame[i].note.txt, 
			     -1);
	  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			     COLUMN_FRAME_COUNT, hl->frame[i].note.cnt, 
			     -1);
	  hl->frame[i].note.auto_fl=FALSE;
	}
	if(!gtk_tree_model_iter_next(model, &iter)) break;
      }
    }
    //return;
  }
  else{
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model),
					 COLUMN_FRAME_ID,
					 hl->sort_flag);
    if(hl->next_note){
      if(hl->frame[hl->num-1].note.txt) g_free(hl->frame[hl->num-1].note.txt);
      hl->frame[hl->num-1].note.txt=g_strdup(hl->next_note);
      gtk_entry_set_text(GTK_ENTRY(hl->e_note),"");
      g_free(hl->next_note);
      hl->next_note=NULL;
    }

    tmp=g_strdup_printf("%08d",
			hl->frame[hl->num-1].idnum+1);
    gtk_entry_set_text(GTK_ENTRY(hl->e_next),tmp);
    g_free(tmp);

    for(i=hl->num_old;i<hl->num;i++){
      //gtk_list_store_insert (GTK_LIST_STORE (model), &iter, i);
      //frame_tree_update_item(hl, GTK_TREE_MODEL(model), iter, i);
      if(hl->date_flag){
	strcpy(odate,hl->frame[i].date);
	t_year=atoi(strtok(odate,"-"));
	t_mon=atoi(strtok(NULL,"-"));
	t_mday=atoi(strtok(NULL,"\0"));
	strcpy(ojst,hl->frame[i].jst);
	t_hour=atoi(strtok(ojst,":"));
	t_min=atoi(strtok(NULL,"\0"));

	tmpt2.tm_year=t_year-1900;
	tmpt2.tm_mon=t_mon-1;
	tmpt2.tm_mday=t_mday;
	tmpt2.tm_hour=t_hour;
	tmpt2.tm_min=t_min;
	tmpt2.tm_sec=0;
	
	tt=mktime(&tmpt2);

	if((tt>hl->date_from)&&(tt<hl->date_to)){
	  d_flag=TRUE;
	}
	else{
	  d_flag=FALSE;
	}
	  
      }
      else{
	d_flag=TRUE;
      }
      
      switch(hl->disp_flag){
      case DISP_NORMAL:
	if(strcmp(hl->frame[i].i2,"Normal")==0){
	  i_flag=TRUE;
	}
	else{
	  i_flag=FALSE;
	}
	break;
	
      case DISP_I2:
	if(strcmp(hl->frame[i].i2,"I2Cell")==0){
	  i_flag=TRUE;
	}
	else{
	  i_flag=FALSE;
	}
	break;
	
      case DISP_OBJECT:
	if(strcmp(hl->frame[i].type,"OBJECT")==0){
	  i_flag=TRUE;
	}
	else{
	  i_flag=FALSE;
	}
	break;
	
      case DISP_FLAT:
	if(strcmp(hl->frame[i].type,"FLAT")==0){
	  i_flag=TRUE;
	}
	else{
	  i_flag=FALSE;
	}
	break;
	
      case DISP_COMP:
	if(strcmp(hl->frame[i].type,"COMPARISON")==0){
	  i_flag=TRUE;
	}
	else{
	  i_flag=FALSE;
	}
	break;
	
      default:
	i_flag=TRUE;
	break;
      }

      if((force_flg)||((d_flag)&&(i_flag))){
	gtk_list_store_insert (GTK_LIST_STORE (model), &iter, i);
	frame_tree_update_item(hl, GTK_TREE_MODEL(model), iter, i);
      }
    }  

    if(hl->num_old+1==hl->num){
      if(strcmp(hl->frame[hl->num-1].type,"OBJECT")==0){
	// auto QL here!
      }
    }
  }

}


int npcmp(FRAMEpara *x, FRAMEpara *y){
  return(x->idnum > y->idnum ? 1 :
	 x->idnum < y->idnum ? -1 : 0);
}

int d_npcmp(FRAMEpara *x, FRAMEpara *y){
  return(x->idnum < y->idnum ? 1 :
	 x->idnum > y->idnum ? -1 : 0);
}

int printfits(typHLOG *hl, char *inf){
  fitsfile *fptr; 
  int status=0;
  char obj_name[256];
  char data_typ[256];
  char observer[32];
  char prop[32];
  char caldate[32];
  char frame_id[32];
  char date_str[32];
  char jst_str[32];
  char jst[6];
  char ut_str[32];
  char ut[6];
  char i2[32];
  char date[12];
  char *cp;
  long is, isslic;
  gdouble iswid;
  gdouble exptime;
  int ret=0;
  float f_buf;
  gdouble secz1, secz2;
  glong idnum_tmp;
  gboolean prop_ok;
  gchar *tmp;
  glong ltmp;
  gint t_year, t_mon, t_mday;
  FRAMEpara *tmp_para;

  fits_open_file(&fptr, inf, READONLY, &status);
  fits_read_key_str(fptr, "FRAMEID", frame_id, 0, &status);
  if(!strncmp(frame_id,"GRA9999",7)) return;

  if(strcmp(hl->last_frame_id,frame_id)!=0){
    idnum_tmp=atol(frame_id+5);

    if(1){
      hl->frame[hl->num].id=g_strdup(frame_id);
      cp=frame_id+5;
      hl->frame[hl->num].idnum=atol(cp);
      
      tmp=g_strdup_printf("%s%sG%08docs_ecfw.fits",
			  hl->wdir,
			  G_DIR_SEPARATOR_S,
			  hl->frame[hl->num].idnum);
      if(access(tmp, F_OK)==0){
	hl->frame[hl->num].qlr=TRUE;
      }
      else{
	hl->frame[hl->num].qlr=FALSE;
      }
      g_free(tmp);
      
      if(hl->num==0){
	if((!hl->upd_flag)&&(!e_init_flag)){
	  fits_read_key_str(fptr, "DATE-OBS", caldate, 0, &status);
	  t_year=atoi(strtok(caldate,"-"));
	  t_mon=atoi(strtok(NULL,"-"));
	  t_mday=atoi(strtok(NULL,"\0"));

	  get_jst_day(&t_year, &t_mon, &t_mday);
	  hl->fr_year=t_year;
	  hl->fr_month=t_mon;
	  hl->fr_day=t_mday;
	  hl->buf_year=hl->fr_year;
	  hl->buf_month=hl->fr_month;
	  hl->buf_day=hl->fr_day;

	  set_fr_e_date(hl);

	  e_init_flag=TRUE;
	}
      }

      fits_read_key_str(fptr, "OBSERVER", observer, 0, &status);
      hl->frame[hl->num].observer=g_strdup(observer);

      fits_read_key_str(fptr, "PROP-ID", prop, 0, &status);
      hl->frame[hl->num].prop=g_strdup(prop);
	
      fits_read_key_str(fptr, "OBJECT", obj_name, 0, &status);
      hl->frame[hl->num].name=g_strdup(obj_name);
      
      fits_read_key_str(fptr, "DATA-TYP", data_typ, 0, &status);
      hl->frame[hl->num].type=g_strdup(data_typ);
      
      fits_read_key_flt(fptr, "EXPTIME", &f_buf, 0, &status);
      hl->frame[hl->num].exp=(guint)f_buf;
      
      fits_read_key_str(fptr, "DATE-OBS", date_str, 0, &status);
      hl->frame[hl->num].date=g_strdup(date_str);
      
      fits_read_key_str(fptr, "JST-STR", jst_str, 0, &status);
      strncpy(jst,jst_str,5);
      jst[5]='\0';
      hl->frame[hl->num].jst=g_strdup(jst);

      fits_read_key_str(fptr, "UT-STR", ut_str, 0, &status);
      strncpy(ut,ut_str,5);
      ut[5]='\0';
      hl->frame[hl->num].ut=g_strdup(ut);

      fits_read_key_flt(fptr, "MJD-STR", &f_buf, 0, &status);
      hl->frame[hl->num].mjd=(gdouble)f_buf;
      
      fits_read_key_flt(fptr, "SECZ-STR", &f_buf, 0, &status);
      secz1=(gdouble)f_buf;
      fits_read_key_flt(fptr, "SECZ-END", &f_buf, 0, &status);
      secz2=(gdouble)f_buf;
      hl->frame[hl->num].secz=(secz1+secz2)/2.;
           
      fits_read_key_str(fptr, "Z_OBSMOD", i2, 0, &status);
      hl->frame[hl->num].i2=g_strdup(i2);
                
      //printf("%s %s %4.0fs (%s)\n",frame_id,jst,exptime, obj_name);
      if(hl->last_frame_id) g_free(hl->last_frame_id);
      hl->last_frame_id=g_strdup(frame_id);
      hl->num++;
      ret=1;
    }
  }

  //if(hl->sort_flag==SORT_ASCENDING){
    qsort(hl->frame, hl->num, sizeof(FRAMEpara),
	  (int(*)(const void*, const void*))npcmp);
    //}
    //else{
    //qsort(hl->frame, hl->num, sizeof(FRAMEpara),
    //	  (int(*)(const void*, const void*))d_npcmp);
    //}

  fits_close_file(fptr, &status);

  return(ret);
}

void ScpCAL(GtkWidget *w, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  
  scp_write_cal(hl);
  gtk_main_quit();
}


void WriteLog(typHLOG *hl, FILE *fp){
  gint i;

  //fprintf(fp,"GAOES-RV Observation LOG  %04d-%02d-%02d (JST)\n\n",
  //	  hl->fr_year,hl->fr_month,hl->fr_day);

  fprintf(fp,"No., Frame-ID,  Object Name, Data Type, Proposal ID, Observer, OBS-DATE, JST, UT, MJD,  Exp., secZ, I2-Cell, count (e-), Note\n");
  
  for(i=0;i<hl->num;i++){
    
    fprintf(fp,"%d, %s, \"%s\", %s, %s, \"%s\", %s, %s, %s, %.6lf, %d, %.2lf, %s, ",
	    i+1,
	    hl->frame[i].id,
	    hl->frame[i].name,
	    hl->frame[i].type,
	    hl->frame[i].prop,
	    hl->frame[i].observer,
	    hl->frame[i].date,
	    hl->frame[i].jst,
	    hl->frame[i].ut,
	    hl->frame[i].mjd,
	    hl->frame[i].exp,
	    hl->frame[i].secz,
	    hl->frame[i].i2);
    if(hl->frame[i].note.cnt>0){
      fprintf(fp,"%d, ",hl->frame[i].note.cnt);
    }
    else{
      fprintf(fp,", ");
    }
    if(hl->frame[i].note.txt){
      fprintf(fp,"\"%s\"\n",hl->frame[i].note.txt);
    }
    else{
      fprintf(fp,"\n");
    }
  }
  
  fprintf(fp,"  \n");
}

void ReadLog(typHLOG *hl,  FILE *fp){
  gchar *buf, *c, *n, *fname=NULL;
  gint i_frm=0, min_line;
  
  while(!feof(fp)){
    if((buf=fgets_new(fp))!=NULL){
      if(strlen(buf)>10){
	c=buf+4+2;
	if(strncmp(c,"GRA",3)==0){
	  if(i_frm==0){
	    min_line=strlen(buf);
	  }
	  else if(strlen(buf)<min_line){
	    min_line=strlen(buf);
	  }
	  i_frm++;
	}
      }
    }
  }

  fseek(fp, 0L, SEEK_SET);
  
  while(!feof(fp)){
    if((buf=fgets_new(fp))!=NULL){
      if(strlen(buf)>10){
	c=buf+4+2;
	if(strncmp(c,"GRA",3)==0){
	  if(strlen(buf)>min_line){
	    fname=g_strndup(buf+4+2,12);

	    for(i_frm=0;i_frm<hl->num;i_frm++){
	      if(strcmp(fname,hl->frame[i_frm].id)==0){
		c=buf+min_line;
		if(hl->frame[i_frm].note.txt) g_free(hl->frame[i_frm].note.txt);
		hl->frame[i_frm].note.txt=g_strdup(c);
		hl->frame[i_frm].note.auto_fl=TRUE;
		break;
	      }
	    }
	    g_free(fname);
	  }
	}
      }
    }
  }
}

void SendMail(GtkWidget *w, gpointer gdata){
  typHLOG *hl;
  FILE *fp;
  gchar filename[256];
  gchar sub[256];
  gchar command_line[512];

  hl=(typHLOG *)gdata;

  sprintf(filename,  "%s%s%s%sgrlog-%04d%02d%02d.txt",
	  g_get_home_dir(), G_DIR_SEPARATOR_S,
	  GRLOG_DIR, G_DIR_SEPARATOR_S,
	  hl->fr_year,hl->fr_month,hl->fr_day);

  if((fp=fopen(filename,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", filename);
    exit(1);
  }

  sprintf(sub,"GAOES-RV Observation LOG  %04d-%02d-%02d (JST)",
	  hl->fr_year,hl->fr_month,hl->fr_day);

  WriteLog(hl, fp);

  fclose(fp);

  sprintf(command_line,"cat %s | %s -s \"%s\" %s",
	  filename, MAIL_COMMAND, sub, hl->mail);
  ext_play(command_line);

  parse_address(hl);

  gtk_main_quit();

}


void ext_play(gchar *exe_command)
{
  static pid_t pid;
  gchar *cmdline;
  gint ret;
  
  waitpid(pid,0,WNOHANG);
  if(strcmp(exe_command,"\0")!=0){
    cmdline=g_strdup(exe_command);
    if( (pid = fork()) == 0 ){
      ret=system(cmdline);
      _exit(-1);
      signal(SIGCHLD,ChildTerm);
    }
  }
}



void do_save (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl;

  hl=(typHLOG *)gdata;
  
  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  grlog_OpenFile(hl, SAVE_LOG);
}

void do_mail (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl;
  GtkWidget *button;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *hbox;

  hl=(typHLOG *)gdata;
  
  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  read_ml(hl);

  hl->smdialog = gtk_dialog_new();
  gtk_container_set_border_width(GTK_CONTAINER(hl->smdialog),5);
  gtk_window_set_title(GTK_WINDOW(hl->smdialog),"GAOES-RV Log Editor : Send Mail");
  gtk_window_set_modal(GTK_WINDOW(hl->smdialog),TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(hl->smdialog),GTK_WINDOW(hl->w_top));
  
  hbox=gtkut_hbox_new(FALSE,5);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(hl->smdialog))),
		     hbox,FALSE, FALSE, 0);

  label=gtk_label_new("Mail Addresses");
  gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);

  hl->address_entry = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),hl->address_entry,TRUE,TRUE,0);
  gtk_entry_set_width_chars(GTK_ENTRY(hl->address_entry),60);
  gtk_entry_set_text(GTK_ENTRY(hl->address_entry),hl->mail);
  g_signal_connect (hl->address_entry,
		    "changed",
		    G_CALLBACK(cc_get_entry),
		    (gpointer)&hl->mail);


  hbox=gtkut_hbox_new(FALSE,5);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(hl->smdialog))),
		     hbox,FALSE, FALSE, 0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Address Book","x-office-address-book");
#else
  button=gtkut_button_new_from_stock("Address Book",GTK_STOCK_PASTE);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
  g_signal_connect(button,"pressed",
		   G_CALLBACK(popup_ml), 
		   (gpointer)hl);

  label=gtk_label_new("  ");
  gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE, 0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Send","mail-send");
#else
  button=gtkut_button_new_from_stock("Send",GTK_STOCK_NETWORK);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
  g_signal_connect(button,"pressed",
		   G_CALLBACK(SendMail), 
		   (gpointer)hl);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","process-stop");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
  g_signal_connect(button,"pressed",
		   G_CALLBACK(close_child_dialog), 
		   GTK_WIDGET(hl->smdialog));

  
  gtk_widget_show_all(hl->smdialog);
  gtk_main();

  gtk_widget_destroy(hl->smdialog);
  flagChildDialog=FALSE;
}


void do_read_log(GtkWidget *widget, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreePath *path;
  gint i_frm;

  grlog_OpenFile(hl, OPEN_LOG);

  update_frame_tree(hl, FALSE);

  path=gtk_tree_path_new_first();

  for(i_frm=0;i_frm<hl->num;i_frm++){
    gtk_tree_model_get_iter (model, &iter, path);
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_NOTE, hl->frame[i_frm].note.txt, -1);
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_COUNT, hl->frame[i_frm].note.cnt, -1);
    gtk_tree_path_next(path);
  }
  gtk_tree_path_free(path);
}


void do_cp_cal(GtkWidget *widget, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  gchar *db_dir, *com;
  gint i, ret;

  if(access(hl->sdir, F_OK)==0){
    db_dir=g_strconcat(hl->sdir,
		       G_DIR_SEPARATOR_S,
		       "database",
		       NULL);
    if(access(hl->sdir, F_OK)!=0){
      com=g_strconcat("mkdir ",
		      db_dir,
		      NULL);
      ret=system(com);
      g_free(com);
    }
  }
  else{
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  "<b>Error</b>: cannot access to HDS shared directory.",
		  " ",
		  hl->sdir,
		  NULL);
    return;
  }

  for(i=0;i<NUM_SET;i++){
    // Ap Red
    if(hl->flag_ap_red[i]){
      com=g_strconcat("cp ",
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      hl->ap_red[i],
		      ".fits  ",
		      hl->sdir,
		      G_DIR_SEPARATOR_S,
		      hl->ap_red[i],
		      ".fits  ",
		      NULL);
      ret=system(com);
      g_free(com);

      com=g_strconcat("cp ",
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      "database",
		      G_DIR_SEPARATOR_S,
		      "ap",
		      hl->ap_red[i],
		      " ",
		      db_dir,
		      G_DIR_SEPARATOR_S,
		      "ap",
		      hl->ap_red[i],
		      NULL);
      ret=system(com);
      g_free(com);
    }

    // ThAr Red
    if(hl->flag_thar_red[i]){
      com=g_strconcat("cp ",
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      hl->thar_red[i],
		      ".fits  ",
		      hl->sdir,
		      G_DIR_SEPARATOR_S,
		      hl->thar_red[i],
		      ".fits  ",
		      NULL);
      ret=system(com);
      g_free(com);

      com=g_strconcat("cp ",
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      "database",
		      G_DIR_SEPARATOR_S,
		      "ec",
		      hl->thar_red[i],
		      " ",
		      db_dir,
		      G_DIR_SEPARATOR_S,
		      "ec",
		      hl->thar_red[i],
		      NULL);
      ret=system(com);
      g_free(com);
    }
  }
}


void do_scp (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl;
  GtkWidget *dialog;
  GtkWidget *button;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *hbox;
  GtkWidget *table;

  hl=(typHLOG *)gdata;
  
  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  dialog = gtk_dialog_new();
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"GAOES-RV Log Editor : Upload CAL files to sumda");
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hl->w_top));
  

  table = gtkut_table_new (2, 2, FALSE, 2, 2, 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     table,FALSE, FALSE, 0);
  
  label=gtk_label_new("User ID : ");
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  entry = gtk_entry_new ();
  gtkut_table_attach(table, entry, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_width_chars(GTK_ENTRY(entry),20);
  gtk_entry_set_text(GTK_ENTRY(entry), HDS01_UNAME);
  gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);

  label=gtk_label_new("Password : ");
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  entry = gtk_entry_new ();
  gtkut_table_attach(table, entry, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_width_chars(GTK_ENTRY(entry),20);
  if(hl->spass) gtk_entry_set_text(GTK_ENTRY(entry), hl->spass);
  gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
  g_signal_connect (entry,
		    "changed",
		    G_CALLBACK(cc_get_entry),
		    (gpointer)&hl->spass);

  hbox=gtkut_hbox_new(FALSE,5);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Upload","network-transmit");
#else
  button=gtkut_button_new_from_stock("Upload",GTK_STOCK_SAVE);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
  g_signal_connect(button,"pressed",
		   G_CALLBACK(ScpCAL), 
		   (gpointer)hl);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","process-stop");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
  g_signal_connect(button,"pressed",
		   G_CALLBACK(close_child_dialog), 
		   dialog);

  
  gtk_widget_show_all(dialog);
  gtk_main();

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;
}


void do_dir (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl;
  GtkWidget *dialog;
  GtkWidget *button;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *hbox;
  GtkWidget *table;

  hl=(typHLOG *)gdata;
  
  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  dialog = gtk_dialog_new_with_buttons("GAOES-RV Log Editor : Folder Setup",
				       GTK_WINDOW(hl->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_OK",GTK_RESPONSE_OK,
#else
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 
  gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							   GTK_RESPONSE_OK));

  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  

  table = gtkut_table_new (2, 4, FALSE, 2, 2, 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     table,FALSE, FALSE, 0);
  
  label=gtk_label_new("Raw data directory : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  label=gtk_label_new("Work directory : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  label=gtk_label_new("IRAF login.cl directory : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);

  label=gtk_label_new("Shared data directory : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);

  button = gtk_file_chooser_button_new ("Select Raw data directory",
                                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (button),
                                       hl->ddir);
  gtkut_table_attach(table, button, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  g_signal_connect (button,
		    "file-set",
		    G_CALLBACK(cc_get_dir),
		    (gpointer)&hl->ddir);

  button = gtk_file_chooser_button_new ("Select Work directory",
                                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (button),
                                       hl->wdir);
  gtkut_table_attach(table, button, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  g_signal_connect (button,
		    "file-set",
		    G_CALLBACK(cc_get_dir),
		    (gpointer)&hl->wdir);

  button = gtk_file_chooser_button_new ("Select IRAF login.cl directory",
                                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (button),
                                       hl->udir);
  gtkut_table_attach(table, button, 1, 2, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  g_signal_connect (button,
		    "file-set",
		    G_CALLBACK(cc_get_dir),
		    (gpointer)&hl->udir);

  button = gtk_file_chooser_button_new ("Select Shared data directory",
                                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (button),
                                       hl->sdir);
  gtkut_table_attach(table, button, 1, 2, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  g_signal_connect (button,
		    "file-set",
		    G_CALLBACK(cc_get_dir),
		    (gpointer)&hl->sdir);

  gtk_widget_show_all(dialog);
  gtk_dialog_run(GTK_DIALOG(dialog));

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;
  
  save_cfg(hl);
}


void do_pyraf_conf (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl;
  GtkWidget *dialog;
  GtkWidget *button;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *hbox;
  GtkWidget *table;

  hl=(typHLOG *)gdata;
  
  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  dialog = gtk_dialog_new_with_buttons("GAOES-RV Log Editor : PyRAF Setup",
				       GTK_WINDOW(hl->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_OK",GTK_RESPONSE_OK,
#else
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 
  gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							   GTK_RESPONSE_OK));

  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  

  table = gtkut_table_new (2, 4, FALSE, 2, 2, 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     table,FALSE, FALSE, 0);
  
  label=gtk_label_new("Terminal command for PyRAF : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  entry = gtk_entry_new ();
  gtkut_table_attach(table, entry, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_width_chars(GTK_ENTRY(entry),40);
  gtk_entry_set_text(GTK_ENTRY(entry), hl->ql_terminal);
  gtk_editable_set_editable(GTK_EDITABLE(entry), TRUE);
  g_signal_connect (entry,
		    "changed",
		    G_CALLBACK(cc_get_entry),
		    (gpointer)&hl->ql_terminal);

  
  label=gtk_label_new("Python command : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  entry = gtk_entry_new ();
  gtkut_table_attach(table, entry, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_width_chars(GTK_ENTRY(entry),40);
  gtk_entry_set_text(GTK_ENTRY(entry), hl->ql_python);
  gtk_editable_set_editable(GTK_EDITABLE(entry), TRUE);
  g_signal_connect (entry,
		    "changed",
		    G_CALLBACK(cc_get_entry),
		    (gpointer)&hl->ql_python);
  
  gtk_widget_show_all(dialog);
  gtk_dialog_run(GTK_DIALOG(dialog));

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;

  save_cfg(hl);
}


void do_remote (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl;
  GtkWidget *dialog;
  GtkWidget *check;
  GtkWidget *button;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *hbox;
  GtkWidget *table;

  hl=(typHLOG *)gdata;
  
  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  dialog = gtk_dialog_new_with_buttons("GAOES-RV Log Editor : Remote Upload Setup",
				       GTK_WINDOW(hl->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_OK",GTK_RESPONSE_OK,
#else
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 
  gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							   GTK_RESPONSE_OK));

  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  

  table = gtkut_table_new (2, 5, FALSE, 2, 2, 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     table,FALSE, FALSE, 0);
  

  check = gtk_check_button_new_with_label("Upload reduced spectra after every QL");
  gtkut_table_attach(table, check, 0, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), hl->remote_flag);
  g_signal_connect (check, "toggled",
		    G_CALLBACK (cc_get_toggle),
		    &hl->remote_flag);

  label=gtk_label_new("Host : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  label=gtk_label_new("User : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);

  label=gtk_label_new("Password : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);

  label=gtk_label_new("Upload directory : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 4, 5,
		     GTK_FILL,GTK_SHRINK,0,0);



  entry = gtk_entry_new ();
  gtkut_table_attach(table, entry, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_width_chars(GTK_ENTRY(entry),60);
  if(hl->remote_host) gtk_entry_set_text(GTK_ENTRY(entry), hl->remote_host);
  g_signal_connect (entry,
		    "changed",
		    G_CALLBACK(cc_get_entry),
		    (gpointer)&hl->remote_host);

  entry = gtk_entry_new ();
  gtkut_table_attach(table, entry, 1, 2, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_width_chars(GTK_ENTRY(entry),60);
  if(hl->remote_user) gtk_entry_set_text(GTK_ENTRY(entry), hl->remote_user);
  g_signal_connect (entry,
		    "changed",
		    G_CALLBACK(cc_get_entry),
		    (gpointer)&hl->remote_user);

  entry = gtk_entry_new ();
  gtkut_table_attach(table, entry, 1, 2, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_width_chars(GTK_ENTRY(entry),20);
  if(hl->remote_pass) gtk_entry_set_text(GTK_ENTRY(entry), hl->remote_pass);
  gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
  g_signal_connect (entry,
		    "changed",
		    G_CALLBACK(cc_get_entry),
		    (gpointer)&hl->remote_pass);

  entry = gtk_entry_new ();
  gtkut_table_attach(table, entry, 1, 2, 4, 5,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_width_chars(GTK_ENTRY(entry),60);
  if(hl->remote_dir) gtk_entry_set_text(GTK_ENTRY(entry), hl->remote_dir);
  g_signal_connect (entry,
		    "changed",
		    G_CALLBACK(cc_get_entry),
		    (gpointer)&hl->remote_dir);

  gtk_widget_show_all(dialog);
  gtk_dialog_run(GTK_DIALOG(dialog));

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;
}


gpointer thread_scan_command(gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;

  printdir(hl);

  if(hl->scloop) g_main_loop_quit(hl->scloop);
  while(my_main_iteration(FALSE));

  return(NULL);
}

gboolean start_scan_command(gpointer gdata){
  gboolean update=FALSE;
  typHLOG *hl=(typHLOG *)gdata;
  

  if(hl->scloop) return(FALSE);

  //printf("Parent : Start scanning\n");
  gtk_label_set_markup(GTK_LABEL(hl->w_status), 
			 "Scanning...");
  while(my_main_iteration(FALSE));
  
  hl->scloop=g_main_loop_new(NULL, FALSE);
  hl->scthread=g_thread_new("grlog_scan",
			    thread_scan_command, (gpointer)hl);
  g_main_loop_run(hl->scloop);
  g_thread_join(hl->scthread);
  //printf("Parent : Thread end\n");
  g_main_loop_unref(hl->scloop);
  hl->scloop=NULL;

  update_frame_tree(hl, FALSE);
  if(hl->num_old!=hl->num){
    hl->num_old=hl->num;
    update=TRUE;
  }

  gtk_label_set_markup(GTK_LABEL(hl->w_status), 
			 " ");
  while(my_main_iteration(FALSE));

  if(update){
    if(hl->num >0){
      switch(hl->scr_flag){
      case SCR_AUTO:
	frame_tree_select_last(hl);
	break;
	/*
      case SCR_SMART:
	if(hl->frame_tree_i > hl->num-2){
	  frame_tree_select_last(hl);
	}
	break;
	*/
      }
    }
  }
  
  hl->scanning_timer=-1;
  return(FALSE);
}

gboolean check_scan (gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;

  if(!hl->scanning_flag){
    hl->scanning_timer=g_timeout_add(100, 
				     (GSourceFunc)start_scan_command,
				     (gpointer)hl);
    hl->scanning_flag=TRUE;
  }
  else if (hl->scanning_timer<0){  // 2nd time
    if(hl->upd_flag){
      hl->scanning_timer=g_timeout_add(READ_INTERVAL, 
				       (GSourceFunc)start_scan_command,
				       (gpointer)hl);
    }
  }

  return(TRUE);
}


gint printdir(typHLOG *hl){
  GtkTreeIter iter;
  DIR *dp;
  struct dirent *entry;
  struct stat statbuf;
  int newflag=0;
  int i,n;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));

  if((dp = opendir(hl->data_dir)) == NULL){
    fprintf(stderr, "cannot open directory: %s\n",hl->data_dir);
    return (-1);
  }
#ifdef DEBUG
  else{
    fprintf(stderr, "Reading: %s\n",hl->data_dir);
  }
#endif
  
  chdir(hl->data_dir);
  
  while((entry=readdir(dp))!=NULL){
    stat(entry->d_name,&statbuf);
    if((!strncmp(entry->d_name,"GRA",3))&&(strlen(entry->d_name)==(3+8+5))){
      if(hl->upd_flag){
        if(!upd0){
	  if(labs(statbuf.st_ctime-hl->seek_time)<5) sleep(5);
#ifdef DEBUG
	  printf("1: %s %s",entry->d_name,
		 asctime(localtime(&hl->fr_time)));
#endif
	  newflag+=printfits(hl,entry->d_name);
	}
	else{
	  if( (statbuf.st_ctime>=hl->seek_time)
	      && (statbuf.st_ctime < hl->to_time)){
	    if(statbuf.st_ctime-hl->seek_time<5) sleep(5);
#ifdef DEBUG
	    printf("1: %s %s",entry->d_name,
		   asctime(localtime(&hl->fr_time)));
#endif
	    newflag+=printfits(hl,entry->d_name);
	  }
	}
      }
      else{
	newflag+=printfits(hl,entry->d_name);
      }
      check_reduced_spectra(hl);
    }
  }
    
  if(hl->upd_flag){
    if(!upd0){
      upd0=TRUE;
    }
  }
  
  chdir("..");
  closedir(dp);

#ifdef DEBUG
  fprintf(stderr, "Start Save\n");
#endif
  save_note(hl);
#ifdef DEBUG
  fprintf(stderr, "End Save\n");
#endif
  
  hl->seek_time=time(NULL);
  
#ifdef DEBUG
  fprintf(stderr, "End of Read: %s\n",hl->data_dir);
#endif

  return (0);
}

void do_quit (GtkWidget *widget)
{
  gtk_main_quit();
}

GtkWidget *make_menu(typHLOG *hl){
  GtkWidget *menu_bar;
  GtkWidget *menu_item;
  GtkWidget *menu;
  GtkWidget *popup_button;
  GtkWidget *bar;
  GtkWidget *image;
  GdkPixbuf *pixbuf, *pixbuf2;
  gint w,h;

  menu_bar=gtk_menu_bar_new();
  gtk_widget_show (menu_bar);

  gtk_icon_size_lookup(GTK_ICON_SIZE_MENU,&w,&h);

  //// File
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("system-file-manager", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "File");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("File");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

  //File/Send Mail
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Save Log");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Save Log");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  g_signal_connect (popup_button, "activate",G_CALLBACK(do_save),(gpointer)hl);
  
  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);
    
  if(hl->upd_flag){
    //File/Send Mail
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("mail-send", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Send Mail");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_NETWORK, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Send Mail");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    g_signal_connect (popup_button, "activate",G_CALLBACK(do_mail),(gpointer)hl);
    
    bar =gtk_separator_menu_item_new();
    gtk_widget_show (bar);
    gtk_container_add (GTK_CONTAINER (menu), bar);   
  }
  else{
  //File/Open Obs Log
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("document-open", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Open Obs Log");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Open Obs Log");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    g_signal_connect (popup_button, "activate",G_CALLBACK(do_read_log),(gpointer)hl);
  }

  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  //File/Quit
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("application-exit", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Quit");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Quit");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  g_signal_connect (popup_button, "activate",G_CALLBACK(do_quit),NULL);

  //// Config
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("utilites-terminal", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "Config");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_ABOUT, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("IRAF");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

  //IRAF/xgterm
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("utilities-terminal", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "PyRAF setups");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_ABOUT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("PyRAF setups");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  g_signal_connect (popup_button, "activate", 
		    G_CALLBACK(do_pyraf_conf), (gpointer)hl);

  //IRAF/folder
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("folder", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Folder Setups");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_DIRECTORY, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Folder Setups");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  g_signal_connect (popup_button, "activate", 
		    G_CALLBACK(do_dir), (gpointer)hl);

  if(hl->upd_flag){
  //// IRAF/Upload
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("network-transmit", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Upload reduced spectra");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Upload reduced spectra");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    g_signal_connect (popup_button, "activate",G_CALLBACK(do_remote),(gpointer)hl);
  }
  else{
  //File/Upload CAL
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-copy", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Copy CAL files to Share Dir.");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_COPY, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Copy CAL files to Share Dir.");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    g_signal_connect (popup_button, "activate",G_CALLBACK(do_cp_cal),(gpointer)hl);
  }

#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("help-browser", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Splot Help");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_HELP, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Splot Help");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  g_signal_connect (popup_button, "activate", 
		    G_CALLBACK(splot_help), (gpointer)hl);

  //// Info
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("user-info", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "Info");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_INFO, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("Info");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  //Info/About
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("help-about", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "About");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_ABOUT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("About");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  g_signal_connect (popup_button, "activate", 
		    G_CALLBACK(show_version), (gpointer)hl);

  gtk_widget_show_all(menu_bar);
  return(menu_bar);
}



void gui_init(typHLOG *hl){
  gchar *tmp;
  GtkWidget *menubar, *label, *table, *check;
  GtkWidget *hbox, *hbox1, *hbox2, *button, *frame, *frame1, *combo, *spinner;
  GtkAdjustment *adj;

  // Main Window 
  hl->w_top = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect(hl->w_top, "destroy",
		   G_CALLBACK(gtk_main_quit),NULL);
  gtk_container_set_border_width(GTK_CONTAINER(hl->w_top),0);
  tmp=g_strdup_printf("GAOES-RV Log Editor (%s)",hl->uname);
  gtk_window_set_title(GTK_WINDOW(hl->w_top),tmp);
  g_free(tmp);

  hl->w_box = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hl->w_top), hl->w_box);

  menubar=make_menu(hl);
  gtk_box_pack_start(GTK_BOX(hl->w_box), menubar,FALSE, FALSE, 0);

  make_top_table(hl);
  gtk_box_pack_start(GTK_BOX(hl->w_box), hl->top_table,FALSE, FALSE, 5);

  hl->scrwin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(hl->scrwin),
				  GTK_POLICY_NEVER,
				  GTK_POLICY_ALWAYS);
  gtk_box_pack_start(GTK_BOX(hl->w_box), hl->scrwin,TRUE, TRUE, 5);

  gtk_widget_set_size_request(hl->scrwin,-1,400);


  make_frame_tree(hl);

  hbox = gtkut_hbox_new(FALSE,5);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(hl->w_box), hbox,FALSE, FALSE, 5);

  hl->frame_ql = gtkut_frame_new (FRAME_QL_LABEL);
  gtk_box_pack_start (GTK_BOX (hbox),hl->frame_ql, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hl->frame_ql), 5);

  table = gtkut_table_new (5, 2, FALSE, 2, 2, 2);
  gtk_container_add (GTK_CONTAINER (hl->frame_ql), table);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("<b>Obj</b>","media-playback-start");
#else
  button=gtkut_button_new_from_stock("<b>Obj</b>", GTK_STOCK_MEDIA_PLAY);
#endif
  gtkut_table_attach(table, button, 0, 1, 0, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (ql_obj_red), (gpointer)hl);


  label = gtk_label_new ("splot def.");
  gtkut_table_attach(table, label, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_ord;
    gchar *tmp;

    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_ord=1;i_ord<=15;i_ord++){
      tmp=g_strdup_printf("order %d", i_ord);
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 
			 0, tmp,
			 1, i_ord, -1);
      g_free(tmp);
      if(hl->ql_line==i_ord) iter_set=iter;
    }
    
    tmp=g_strdup_printf("Make 1D");
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 
		       0, tmp,
		       1, 16, -1);
    g_free(tmp);
    if(hl->ql_line == 16) iter_set=iter;

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 1, 2, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    g_signal_connect (combo,"changed",G_CALLBACK(cc_get_combo_box),
		      &hl->ql_line);
  }
 
  frame1 = gtkut_frame_new ("Calibration");
  gtkut_table_attach(table, frame1, 4, 5, 0, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (frame1), 5);

  hbox2 = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox2), 2);
  gtk_container_add (GTK_CONTAINER (frame1), hbox2);

  hl->button_flat_red=gtk_button_new_with_label("Flat");
  gtk_box_pack_start(GTK_BOX(hbox2),hl->button_flat_red,FALSE, FALSE, 0);
  g_signal_connect (hl->button_flat_red, "clicked",
		    G_CALLBACK (ql_flat_red), (gpointer)hl);

  hl->button_thar_red=gtk_button_new_with_label("ThAr");
  gtk_box_pack_start(GTK_BOX(hbox2),hl->button_thar_red,FALSE, FALSE, 0);
  g_signal_connect (hl->button_thar_red, "clicked",
		    G_CALLBACK (ql_thar_red), (gpointer)hl);

  button=gtk_button_new_with_label("Mask");
  gtk_box_pack_start(GTK_BOX(hbox2),button,FALSE, FALSE, 0);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (ql_mask), (gpointer)hl);

  button=gtk_button_new_with_label("Blaze");
  gtk_box_pack_start(GTK_BOX(hbox2),button,FALSE, FALSE, 0);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (ql_blaze), (gpointer)hl);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"preferences-system");
#else
  button=gtkut_button_new_from_stock(NULL, GTK_STOCK_PROPERTIES);
#endif
  gtk_box_pack_start(GTK_BOX(hbox2),button,FALSE, FALSE, 0);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (edit_cal), (gpointer)hl);


  frame = gtkut_frame_new ("<b>Get Spectrum Count</b>");
  gtk_box_pack_start (GTK_BOX (hbox),frame, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  table = gtkut_table_new (5, 2, FALSE, 2, 2, 2);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label = gtk_label_new ("measuring at");
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_ord;
    gchar *tmp;

    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_ord=1;i_ord<=15;i_ord++){
      tmp=g_strdup_printf("order %d", i_ord);
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 
			 0, tmp,
			 1, i_ord, -1);
      g_free(tmp);
      if(hl->ql_ge_line ==i_ord) iter_set=iter;
    }
       
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 0, 1, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    g_signal_connect (combo,"changed",G_CALLBACK(cc_get_combo_box),
		      &hl->ql_ge_line);
  }

  label = gtkut_label_new ("<span size=\"smaller\">Start X</span>");
  gtkut_table_attach(table, label, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hl->ql_ge_stx,
					    1,3000,
					    1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table, spinner, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  g_signal_connect (adj, "value_changed",
		    G_CALLBACK (cc_get_adj),
		    &hl->ql_ge_stx);

  label = gtkut_label_new ("<span size=\"smaller\">End X</span>");
  gtkut_table_attach(table, label, 2, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hl->ql_ge_edx,
					    1000,4000,
					    1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtkut_table_attach(table, spinner, 2, 3, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  g_signal_connect (adj, "value_changed",
		    G_CALLBACK (cc_get_adj),
		    &hl->ql_ge_edx);
  

  gtk_widget_show_all(hl->w_top);
}



void splot_help(GtkWidget *w, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  GtkWidget *dialog, *frame, *table, *label, *vbox, *button, *hbox;

  dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(dialog), "GAOES-RV Log Editor : Help for splot");

  vbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (dialog), vbox);

  frame = gtkut_frame_new ("<b>Change Order</b>");
  gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  
  table = gtkut_table_new (2, 2, FALSE, 2, 2, 2);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label=gtkut_label_new("    <b>(</b>    ");
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Move to previous");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label=gtkut_label_new("    <b>)</b>    ");
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Move to next");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  frame = gtkut_frame_new ("<b>X-axis Scale</b>");
  gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  
  table = gtkut_table_new (2, 2, FALSE, 2, 2, 2);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label=gtkut_label_new("    <b>v</b>    ");
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Velocity scale [km/s] (toggle)");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label=gtkut_label_new("    <b>$</b>    ");
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Pixel (toggle)");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  frame = gtkut_frame_new ("<b>Y-axis Scale</b>");
  gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  
  table = gtkut_table_new (2, 1, FALSE, 2, 2, 2);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label=gtkut_label_new("    <b>b</b>    ");
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Set base line to zero (toggle)");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
   
  frame = gtkut_frame_new ("<b>Magnify</b>");
  gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  
  table = gtkut_table_new (2, 2, FALSE, 2, 2, 2);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label=gtkut_label_new("    <b>a</b> &amp; <b>a</b>    ");
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Auto expand between cursors");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label=gtkut_label_new("    <b>c</b>    ");
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Clear and redraw");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  frame = gtkut_frame_new ("<b>Measurement</b>");
  gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  
  table = gtkut_table_new (2, 4, FALSE, 2, 2, 2);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label=gtkut_label_new("    <b>m</b> &amp; <b>m</b>   ");
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Avg, RMS, and SNR between cursors");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
 
  label=gtkut_label_new("    <b>k</b> &amp; <b>k</b>   ");
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Gaussian fit");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label=gtkut_label_new("    <b>e</b> &amp; <b>e</b>   ");
  gtkut_table_attach(table, label, 0, 1, 2, 3,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Get equivalent width");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label=gtkut_label_new("    <b>C</b>   ");
  gtkut_table_attach(table, label, 0, 1, 3, 4,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Get values at cursor position");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  frame = gtkut_frame_new ("<b>Other Display Commands</b>");
  gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  
  table = gtkut_table_new (2, 2, FALSE, 2, 2, 2);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label=gtkut_label_new("    <b>s</b>    ");
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Smooth the spectrum by boxcar");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  label=gtkut_label_new("    <b>:hist y</b>    ");
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Set line type to Histgram");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
   
  
  hbox = gtkut_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Close","window-close");
#else
  button=gtkut_button_new_from_stock("Close",GTK_STOCK_CLOSE);
#endif
  g_signal_connect (button, "clicked",
		    G_CALLBACK (close_dialog), dialog);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

  gtk_widget_show_all(dialog);
}


void show_version (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox;
  GdkPixbuf *pixbuf, *pixbuf2;
#if HAVE_SYS_UTSNAME_H
  struct utsname utsbuf;
#endif
  gchar buf[1024];
  GtkWidget *scrolledwin;
  GtkWidget *text;
  GtkTextBuffer *buffer;
  GtkTextIter iter;
  typHLOG *hl=(typHLOG *) gdata;
  gint result;

  dialog = gtk_dialog_new_with_buttons("GAOES-RV Log Editor : About This Program",
				       GTK_WINDOW(hl->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_OK",GTK_RESPONSE_OK,
#else
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 
  gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							   GTK_RESPONSE_OK));
  

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

  pixbuf = gdk_pixbuf_new_from_resource ("/icons/gaoes-rv_logo_s.png", NULL);
  pixbuf2=gdk_pixbuf_scale_simple(pixbuf,
				  500,86,GDK_INTERP_BILINEAR);
  pixmap = gtk_image_new_from_pixbuf(pixbuf2);
  g_object_unref(pixbuf);
  g_object_unref(pixbuf2);

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);


  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  gtk_label_set_markup(GTK_LABEL(label), "<span size=\"larger\"><b>Seimei/GAOES-RV Log Editor</b></span>   version "VERSION);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  g_snprintf(buf, sizeof(buf),
	     "GTK+ %d.%d.%d / GLib %d.%d.%d",
	     gtk_major_version, gtk_minor_version, gtk_micro_version,
	     glib_major_version, glib_minor_version, glib_micro_version);
  label = gtk_label_new (buf);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

#if HAVE_SYS_UTSNAME_H
  uname(&utsbuf);
  g_snprintf(buf, sizeof(buf),
	     "Operating System: %s %s (%s)",
	     utsbuf.sysname, utsbuf.release, utsbuf.machine);
#else
  g_snprintf(buf, sizeof(buf),
	     "Operating System: unknown UNIX");
#endif
  label = gtk_label_new (buf);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);


  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

 
  
  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL(label), "&#xA9; 2023  Akito Tajitsu");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new ("Okayama Branch, Subaru Telescope");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  label = gtk_label_new ("National Astronomical Observatory of Japan, NINS");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  label=gtk_label_new(NULL);
  gtk_label_set_markup (GTK_LABEL(label), "&lt;<i>akito.tajitsu@nao.ac.jp</i>&gt;");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  scrolledwin = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwin),
				 GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwin),
				      GTK_SHADOW_IN);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     scrolledwin, TRUE, TRUE, 0);
  gtk_widget_set_size_request (scrolledwin, 400, 250);
  
  text = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(text), FALSE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text), GTK_WRAP_WORD);
  gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text), 6);
  gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text), 6);
  gtk_container_add(GTK_CONTAINER(scrolledwin), text);
  
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
  gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);
  
  gtk_text_buffer_insert(buffer, &iter,
			 "This program is free software; you can redistribute it and/or modify "
			 "it under the terms of the GNU General Public License as published by "
			 "the Free Software Foundation; either version 3, or (at your option) "
			 "any later version.\n\n", -1);

  gtk_text_buffer_insert(buffer, &iter,
			 "This program is distributed in the hope that it will be useful, "
			 "but WITHOUT ANY WARRANTY; without even the implied warranty of "
			 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. "
			 "See the GNU General Public License for more details.\n\n", -1);

  gtk_text_buffer_insert(buffer, &iter,
			 "You should have received a copy of the GNU General Public License "
			 "along with this program.  If not, see <http://www.gnu.org/licenses/>.", -1);

  gtk_text_buffer_get_start_iter(buffer, &iter);
  gtk_text_buffer_place_cursor(buffer, &iter);

  gtk_widget_show_all(dialog);

  result= gtk_dialog_run(GTK_DIALOG(dialog));

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
}


void usage(void)
{
  g_print(" grlog : Seimei / GAOES-RV Obs-Log Editor   Ver"VERSION"\n");
  g_print("  [usage] %% grlog [-s shared_dir] [-w work_dir] [-h] [-u] data_dir\n");

  exit(0);
}


void get_option(int argc, char **argv, typHLOG *hl)
{
  int i_opt;
  int valid=1;
  gchar *cwdname=NULL, *dbase;

  hl->sdir=NULL;
  hl->wdir=NULL;
  
  i_opt = 1;
  while((i_opt < argc-1)&&(valid==1)) {
    if(strcmp(argv[i_opt],"-s") == 0){ 
      if(i_opt+1 < argc ) {
	i_opt++;
	if(!g_path_is_absolute(g_path_get_dirname(argv[i_opt]))){
	  cwdname=g_malloc0(sizeof(gchar)*1024);
	  if(!getcwd(cwdname,1024)){
	    fprintf(stderr, "Warning: Could not find the shared data directory.");
	  }
	  hl->sdir=g_strconcat(cwdname,"/",argv[i_opt],NULL);
	}
	else{
	  hl->sdir=g_strdup(argv[i_opt]);
	}
	i_opt++;
      }
      else{
	valid = 0;
      }
    }
    else if(strcmp(argv[i_opt],"-w") == 0){ 
      if(i_opt+1 < argc ) {
	i_opt++;
	if(!g_path_is_absolute(g_path_get_dirname(argv[i_opt]))){
	  cwdname=g_malloc0(sizeof(gchar)*1024);
	  if(!getcwd(cwdname,1024)){
	    fprintf(stderr, "Warning: Could not find the working directory.");
	  }
	  hl->wdir=g_strconcat(cwdname,"/",argv[i_opt],NULL);
	}
	else{
	  hl->wdir=g_strdup(argv[i_opt]);
	}
	if(access(hl->wdir, F_OK)!=0){
	  fprintf(stderr, "Making the database directory \"%s\" ...\n", hl->wdir);
	  mkdir(hl->wdir, (S_IRWXU|S_IRWXG|S_IRWXO));
	}
	dbase=g_strconcat(hl->wdir,
			  G_DIR_SEPARATOR_S,
			  "database",
			  NULL);
	if(access(dbase, F_OK)!=0){
	  fprintf(stderr, "Making the database directory \"%s\" ...\n", dbase);
	  mkdir(dbase, (S_IRWXU|S_IRWXG|S_IRWXO));
	}
	g_free(dbase);
	
	i_opt++;
      }
      else{
	valid = 0;
      }
    }
    else if ((strcmp(argv[i_opt], "-h") == 0) ||
	     (strcmp(argv[i_opt], "--help") == 0)) {
      i_opt++;
      usage();
    }
    else if ((strcmp(argv[i_opt], "-u") == 0) ||
	     (strcmp(argv[i_opt], "--debug") == 0)) {
      hl->upd_flag=TRUE;
      i_opt++;
    }
    else{
      fprintf(stderr, "Warning: detected invalid command line option.\n");
      usage();
    }

  }
  
}


int main(int argc, char* argv[]){
  typHLOG *hl;
  time_t t;
  struct tm *tmpt,tmpt2;
  gint i;
  gchar *filename;
  GdkPixbuf *icon;
  gchar hostname[128], *argdir=NULL;
  

  hl=g_malloc0(sizeof(typHLOG));
  
  memset(&tmpt2, 0x00, sizeof(struct tm));


  get_option(argc, argv, hl);
  argdir=g_strdup(argv[argc-1]);

  load_cfg(hl);

  upd0=FALSE;
  
  filename=g_strconcat(g_get_home_dir(),G_DIR_SEPARATOR_S,
		       GRLOG_DIR, NULL);
  if(access(filename, F_OK)!=0){
    fprintf(stderr, "Creating Directory \"%s\".\n",filename);
    mkdir(filename,(S_IRWXU|S_IRGRP|S_IROTH));
  }
  g_free(filename);

  write_muttrc();
  write_msmtprc();


  if(argdir[strlen(argdir)-1]=='/'){
    hl->data_dir=g_strndup(argdir,strlen(argdir)-1);
  }
  else{
    hl->data_dir=g_strdup(argdir);
  }
  g_free(argdir);
  hl->num=0;
  hl->idnum0=0;
  hl->mail=g_strdup(DEF_MAIL);

  hl->http_host=NULL;
  hl->http_path=NULL;
  hl->http_dlfile=NULL;

  t = time(NULL);
  tmpt = localtime(&t);

  hl->fr_year=tmpt->tm_year+1900;
  hl->fr_month=tmpt->tm_mon+1;
  hl->fr_day=tmpt->tm_mday;
  if(tmpt->tm_hour<9){
    get_jst_day(&hl->fr_year, &hl->fr_month, &hl->fr_day);
  }

  tmpt2.tm_year=hl->fr_year-1900;
  tmpt2.tm_mon=hl->fr_month-1;
  tmpt2.tm_mday=hl->fr_day;
  tmpt2.tm_hour=9;
  tmpt2.tm_min=0;
  tmpt2.tm_sec=0;
  
  hl->fr_time=mktime(&tmpt2);
  hl->seek_time=hl->fr_time;

  hl->to_time=hl->fr_time+60*60*24;
  
  hl->buf_year=hl->fr_year;
  hl->buf_month=hl->fr_month;
  hl->buf_day=hl->fr_day;

  hl->scr_flag=SCR_AUTO;

  for(i=0;i<MAX_FRAME;i++){
    init_frame(&hl->frame[i]);
  }
  hl->last_frame_id=g_strdup("New");
  

  for(i=0;i<MAX_MAIL;i++){
    hl->ml[i].address=NULL;
    hl->ml[i].year=0;
    hl->ml[i].month=0;
    hl->ml[i].day=0;
  }
  hl->ml_max=0;
  
  flag_make_frame_tree=FALSE;

  // IRAF
  hl->ql_thar1d=g_strdup(GAOES_THAR1D);
  hl->ql_thar2d=g_strdup(GAOES_THAR2D);
  hl->ql_flat=g_strdup(GAOES_FLAT);
  hl->ql_ap=g_strdup(GAOES_AP);
  hl->ql_mask=g_strdup(GAOES_MASK);
  hl->ql_blaze=g_strdup(GAOES_BLAZE);
  hl->ql_thar_new=NULL;
  hl->ql_flat_new=NULL;
  hl->ql_ap_new=NULL;

  hl->ql_st_x=GAOES_ST_X;
  hl->ql_ed_x=GAOES_ED_X;

  hl->ql_line=2;
  hl->ql_ge_line=2;
  hl->ql_ge_stx=2150;
  hl->ql_ge_edx=2400;
  
  if(!hl->ql_python) hl->ql_python=g_strdup(QL_PYTHON);
  if(!hl->ql_terminal) hl->ql_terminal=g_strdup(QL_TERMINAL);
  
  hl->uname=getenv("USER");
  hl->ql_lock=g_strdup_printf("%s%s.lock-grql.%s",
			      g_get_tmp_dir(),
			      G_DIR_SEPARATOR_S,
			      hl->uname);
  if(access(hl->ql_lock,F_OK)==0){
    printf(stderr,"!!! Lock file for quick look \"%s\" exists. Removed. !!!\n",
	   hl->ql_lock);
    unlink(hl->ql_lock);
  }
  hl->ql_timer=-1;
  hl->ddir=g_strdup(hl->data_dir);
  if(!hl->wdir) hl->wdir=get_work_dir(hl);
  if(!hl->sdir) hl->sdir=get_share_dir(hl);
  
  if(!hl->udir) hl->udir=get_logincl_dir(hl);
  hl->spass=NULL;
  hl->iraf_hdsql_r=0;
  hl->file_write=NULL;
  hl->file_wait=NULL;
  hl->ref_frame=NULL;

  hl->date_flag=FALSE;
  hl->disp_flag=DISP_ALL;
  hl->sort_flag=SORT_ASCENDING;

  hl->auto_red=FALSE;

  for(i=0;i<NUM_SET;i++){
    hl->setname_red[i]=NULL;
    hl->ap_red[i]=NULL;
    hl->flat_red[i]=NULL;
    hl->thar_red[i]=NULL;

    hl->flag_ap_red[i]=FALSE;
    hl->ex_flat_red[i]=FLAT_EX_NO;
    hl->flag_thar_red[i]=FALSE;
  }
  
  hl->i_reduced=0;

  hl->remote_flag=FALSE;
  hl->remote_host=g_strdup(REMOTE_HOST);
  hl->remote_user=g_strdup(REMOTE_USER);
  hl->remote_pass=NULL;
  hl->remote_dir=g_strconcat(REMOTE_DIR,
			     hl->uname,
			     G_DIR_SEPARATOR_S,
			     NULL);
  
  gtk_init(&argc, &argv);

#ifndef USE_GTK3
  gdk_color_alloc(gdk_colormap_get_system(),&color_red);
  gdk_color_alloc(gdk_colormap_get_system(),&color_black);
#endif

  //popup_dl_camz_list(NULL, (gpointer)hl);
  icon = gdk_pixbuf_new_from_resource ("/icons/gaoes_icon.png", NULL);
  gtk_window_set_default_icon(icon);

  gui_init(hl);
  
  hl->scanning_flag=FALSE;
  if(hl->upd_flag){
    check_reference_data(hl);
    prepare_pyraf(hl);
    
    hl->timer=g_timeout_add(CHECK_INTERVAL, 
			    (GSourceFunc)check_scan, 
			    (gpointer)hl);
  }
  else{
    do_dir(NULL,(gpointer)hl);
    check_reference_data(hl);
    prepare_pyraf(hl);
    
    usleep(1e6);
    start_scan_command((gpointer)hl);
  }
  gtk_main();
}

