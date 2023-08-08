#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif  

#include<gtk/gtk.h>
#include <gio/gio.h>

#include <X11/Xlib.h>

#include <unistd.h>

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<time.h>
#include<signal.h>
#include<unistd.h>
#include<pwd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<errno.h>
#include<fcntl.h>
#include <ctype.h>

#include<unistd.h>
#include<dirent.h>
#include<string.h>

#include <fitsio.h>
#include <math.h>

#include <sys/socket.h>
#include <netdb.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#if HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif


#include "gui.h"
#include "gtkut.h"
#include "tree.h"
#include "mltree.h"
#include "mail.h"
#include "grql.h"

#define GRLOG_DIR "Log"

#define SND_CMD "/usr/bin/audioplay /opt/share/hds/au/%s"

#define XGTERM_COM "/usr/local/bin/xgterm -sb -sl 2000 -title %s -e \"cd ~;/usr/local/bin/cl\" &"

#define SHARE_DIR "IRAF/GAOES-RV_ql"
#define GAOES_AP "Ap.GAOES-RV.f8"
#define GAOES_FLAT "Flat.GAOES-RV.f8.sc.nm"
#define GAOES_THAR1D "ThAr.GAOES-RV.f8.center"
#define GAOES_THAR2D "ThAr.GAOES-RV.f8"
#define GAOES_MASK "Mask.GAOES-RV.ocs_ecfw"
#define GAOES_BLAZE "cBlaze.GAOES-RV.f8"
#define GAOES_PY_GRQL "grql.py"
#define GAOES_PY_GRQL_BATCH "grql_batch.py"
#define GAOES_PY_FLAT "gaoes_flat.py"
#define GAOES_PY_COMP "gaoes_comp.py"
#define GAOES_PY_COMP_OBJ "gaoes_comp_obj.py"
#define GAOES_PY_SPLOT "splot.py"
#define GAOES_PY_MASK "gaoes_mkmask.py"
#define GAOES_PY_BLAZE "gaoes_mkblaze.py"
#define GAOES_PY_MKREF "gaoes_mkref.py"

#define GAOES_ST_X (-54)
#define GAOES_ED_X 53

#define MAX_ENTRY 2000

#define QL_PYTHON "python3"
#define QL_TERMINAL "xterm -geometry 85x10 -e"

#define QL_ERROR_THRESHOLD 3

#define AUTO_FLAT0    5
#define AUTO_FLAT_NUM 20

#define SMOKA_LOG_HOST "www.o.kwasan.kyoto-u.ac.jp"
#define SMOKA_LOG_PATH "/seimei_obs_log/add_comment.py"
#define SMOKA_LOG_FILE "/tmp/grlog-www-%d.txt"
#define SEIMEI_PROXY_HOST "192.168.1.52"
#define SEIMEI_PROXY_PORT 3128

#define GRLOG_HTTP_ERROR_GETHOST  -1
#define GRLOG_HTTP_ERROR_SOCKET   -2
#define GRLOG_HTTP_ERROR_CONNECT  -3
#define GRLOG_HTTP_ERROR_TEMPFILE -4
#define GRLOG_HTTP_ERROR_SSL -5
#define GRLOG_HTTP_ERROR_FORK -6

typedef struct _PARAMpost PARAMpost;
struct _PARAMpost{
  gint  flg;
  gchar *key;
  gchar *prm;
};

enum{ POST_NULL, 
      POST_CONST, 
      POST_INPUT
};

static const PARAMpost seimei_log_post[] = {
  {POST_INPUT, "expid",   NULL},
  {POST_CONST, "mode",    "add"},
  {POST_INPUT, "remarks", NULL},
  {POST_CONST, "submit_remarks", "Submit"},
  {POST_NULL,  NULL, NULL}};

enum{QL_THAR, QL_FLAT, QL_OBJECT, QL_OBJECT_BATCH, QL_SPLOT, QL_MASK, QL_BLAZE, NUM_QL};

enum{QLR_NONE, QLR_NOW, QLR_DONE, NUM_QLR};


#define FLAT_IN "flat.in"


#define HTTP_DLSZ_FILE   "hdslog_http_dlsz.txt"

#define REMOTE_HOST "hds.skr.jp"
#define REMOTE_USER "rosegray"
#define REMOTE_DIR "/home/rosegray/www/hds/Data/"


#define FRAME_QL_LABEL  "<span color=\"#FF0000\"><b>Quick Look</b></span>"

enum{CAL_AP, CAL_FLAT, CAL_THAR, NUM_CAL};

enum{FLAT_OW_NONE, FLAT_OW_SKIP, FLAT_OW_GO,  FLAT_OW_ABORT};

enum{OPEN_AP, OPEN_FLAT, OPEN_THAR, OPEN_MASK, OPEN_BLAZE,
     OPEN_LOG, SAVE_LOG, NUM_OPEN};

enum{DISP_ALL, DISP_NORMAL, DISP_I2, DISP_OBJECT, DISP_FLAT, DISP_COMP,
     NUM_DISP};

enum{QLCAL_NONE, QLCAL_FLAT, QLCAL_FLAT0, QLCAL_COMP, QLCAL_COMP0, NUM_QLCAL};

static gchar *disp_name[]={
  "Display All",
  "Normal Only",
  "I2Cell Only",
  "Object Only",
  "Flat Only",
  "Comp Only",
  NULL
};

enum{SORT_ASCENDING, SORT_DESCENDING, NUM_SORT};

static gchar *sort_name[]={
  "Ascending",
  "Descending",
  NULL
};

enum{DISP_TIME_JST, DISP_TIME_UT, DISP_TIME_MJD, NUM_DISP_TIME};

static gchar *disp_time_name[]={
  "JST",
  "UT",
  "MJD",
  NULL
};

#ifndef G_DIR_SEPARATOR_S
#define G_DIR_SEPARATOR_S "/"
#endif


#ifdef SIGRTMIN
#define SIGHTTPDL SIGRTMIN+1
#else
#define SIGHTTPDL SIGUSR2
#endif

#define POPUP_TIMEOUT 2

#define HDSLOG_HTTP_ERROR_GETHOST  -1
#define HDSLOG_HTTP_ERROR_SOCKET   -2
#define HDSLOG_HTTP_ERROR_CONNECT  -3
#define HDSLOG_HTTP_ERROR_TEMPFILE -4
#define HDSLOG_HTTP_ERROR_SSL -5
#define HDSLOG_HTTP_ERROR_FORK -6

#define DEF_D_CROSS_R (+130)
#define DEF_D_CROSS_B (+130)
#define DEF_CAMZ_R (-325)
#define DEF_CAMZ_B (-350)
#define DEF_ECHELLE0 (+880)
#define ALLOWED_DELTA_CROSS 10

#define MAX_MAIL 1000

#define RANDOMIZE() srand(time(NULL)+getpid())
#define RANDOM(x)  (rand()%(x))

#define CHECK_INTERVAL 1000
#define READ_INTERVAL 5*1000

#define MAX_FRAME 1000

#define DEF_MAIL "akito.tajitsu@nao.ac.jp"
#define DEF_FROM "HDS Administrator <tajitsu@naoj.org>"
#define MAIL_COMMAND "mutt"
#define MUTT_FILE ".muttrc"
#define MSMTP_FILE ".msmtprc"

static gchar *muttrc_str[]={
  "set charset=\"utf-8\"",
  "set sendmail=\"/usr/bin/msmtp\"",
  "set realname=\"Seimei / GAOES-RV Observation Log\"",
  "set from=\"" MAIL_ADDRESS "\"",
  NULL
};

static gchar *msmtprc_str[]={
  "# Set default values for all following accounts.",
  "defaults",
  "auth           on",
  "tls            on",
  "tls_trust_file /etc/ssl/certs/ca-bundle.crt",
  "logfile        ~/.msmtp.log",
  " ",
  "# Gmail",
  "account        gmail",
  "host           smtp.gmail.com",
  "port           587",
  "from           " MAIL_ADDRESS,
  "user           " MAIL_ID,
  "password       " MAIL_PASS,
  " ",
  "# Set a default account",
  "account default : gmail",
  NULL
};


// Setup
enum{ StdUb, StdUa, StdBa, StdBc, StdYa, StdI2b, StdYd, StdYb, StdYc, StdI2a, StdRa, StdRb, StdNIRc, StdNIRb, StdNIRa, StdHa};

// FileHead
enum{ FILE_HDSA, FILE_hds};

// Color
enum{ COLOR_R, COLOR_B};

// Scroll
enum{SCR_AUTO, SCR_NONE, NUM_SCR};

static gchar *scr_name[]={
  "Auto Scroll",
  "No Scroll",
  NULL
};

// IRAF
#define NUM_SET 5

static gchar *hdsql_red[]={
  "hdsql",
  "hdsql2",
  "hdsql4",
  "hdsql6",
  "hdsql8",
  NULL
};

#define FLAT_TMP1 "/tmp/hdslog_flat_%s.txt"
#define FLAT_TMP2 "/tmp/hdslog_flat_%s_omlx.txt"

enum{  FLAT_EX_NO,
       FLAT_EX_1,
       FLAT_EX_SC,
       FLAT_EX_SCNM,
	 FLAT_EX_SCFL,
	 NUM_FLAT_EX};

static gchar *flat_ex[]={
  "----",
  "ave",
  "sc",
  "sc.nm",
  "sc.fl",
  NULL
};
       


// Color for GUI
#ifdef USE_GTK3
static GdkRGBA color_comment = {0.87, 0.00, 0.00, 1};
static GdkRGBA color_comp  =   {0.30, 0.30, 1.00, 1};
static GdkRGBA color_flat  =   {0.89, 0.36, 0.00, 1};
static GdkRGBA color_bias  =   {0.25, 0.25, 0.25, 1};
static GdkRGBA color_focus =   {0.53, 0.27, 0.00, 1};
static GdkRGBA color_calib =   {0.00, 0.53, 0.00, 1};
static GdkRGBA color_black =   {0.00, 0.00, 0.00, 1};
static GdkRGBA color_red   =   {1.00, 0.00, 0.00, 1};
static GdkRGBA color_blue =    {0.00, 0.00, 1.00, 1};
static GdkRGBA color_white =   {1.00, 1.00, 1.00, 1};
static GdkRGBA color_gray1 =   {0.40, 0.40, 0.40, 1};
static GdkRGBA color_gray2 =   {0.80, 0.80, 0.80, 1};
static GdkRGBA color_pink =    {1.00, 0.40, 0.40, 1};
static GdkRGBA color_pink2 =   {1.00, 0.80, 0.80, 1};
static GdkRGBA color_pale =    {0.40, 0.40, 1.00, 1};
static GdkRGBA color_pale2 =   {0.80, 0.80, 1.00, 1};
static GdkRGBA color_pale3 =   {0.90, 0.90, 1.00, 1};
static GdkRGBA color_yellow3 = {1.00, 1.00, 0.90, 1};
static GdkRGBA color_orange =  {1.00, 0.80, 0.40, 1};
static GdkRGBA color_orange2 = {1.00, 1.00, 0.80, 1};
static GdkRGBA color_orange3 = {0.95, 0.45, 0.02, 1};
static GdkRGBA color_green  =  {0.40, 0.80, 0.80, 1};
static GdkRGBA color_green2 =  {0.80, 1.00, 0.80, 1};
static GdkRGBA color_purple2 = {1.00, 0.80, 1.00, 1};
static GdkRGBA color_com1 =    {0.00, 0.53, 0.00, 1};
static GdkRGBA color_com2 =    {0.73, 0.53, 0.00, 1};
static GdkRGBA color_com3 =    {0.87, 0.00, 0.00, 1};
static GdkRGBA color_lblue =   {0.80, 0.80, 1.00, 1};
static GdkRGBA color_lgreen =  {0.80, 1.00, 0.80, 1};
static GdkRGBA color_lorange=  {1.00, 0.90, 0.70, 1};
static GdkRGBA color_lred   =  {1.00, 0.80, 0.80, 1};
#else
static GdkColor color_comp =    {0, 0x6600, 0x6600, 0xFFFF};
static GdkColor color_flat =    {0, 0xE300, 0x5C00, 0x0000};
static GdkColor color_bias =    {0, 0x8000, 0x8000, 0x8000};
static GdkColor color_comment = {0, 0xDDDD, 0x0000, 0x0000};
static GdkColor color_focus = {0, 0x8888, 0x4444, 0x0000};
static GdkColor color_calib = {0, 0x0000, 0x8888, 0x0000};
static GdkColor color_black = {0, 0, 0, 0};
static GdkColor color_red   = {0, 0xFFFF, 0, 0};
static GdkColor color_blue = {0, 0, 0, 0xFFFF};
static GdkColor color_white = {0, 0xFFFF, 0xFFFF, 0xFFFF};
static GdkColor color_gray1 = {0, 0x6666, 0x6666, 0x6666};
static GdkColor color_gray2 = {0, 0xBBBB, 0xBBBB, 0xBBBB};
static GdkColor color_pink = {0, 0xFFFF, 0x6666, 0x6666};
static GdkColor color_pink2 = {0, 0xFFFF, 0xCCCC, 0xCCCC};
static GdkColor color_pale = {0, 0x6666, 0x6666, 0xFFFF};
static GdkColor color_pale2 = {0, 0xCCCC, 0xCCCC, 0xFFFF};
static GdkColor color_pale3 = {0, 0xEEEE, 0xEEEE, 0xFFFF};
static GdkColor color_yellow3 = {0, 0xFFFF, 0xFFFF, 0xEEEE};
static GdkColor color_orange = {0, 0xFFFF, 0xCCCC, 0x6666};
static GdkColor color_orange2 = {0, 0xFFFF, 0xFFFF, 0xCCCC};
static GdkColor color_orange3 = {0, 0xFD00, 0x6A00, 0x0200};
static GdkColor color_green = {0, 0x6666, 0xCCCC, 0x6666};
static GdkColor color_green2 = {0, 0xCCCC, 0xFFFF, 0xCCCC};
static GdkColor color_purple2 = {0, 0xFFFF, 0xCCCC, 0xFFFF};
static GdkColor color_com1 = {0, 0x0000, 0x8888, 0x0000};
static GdkColor color_com2 = {0, 0xBBBB, 0x8888, 0x0000};
static GdkColor color_com3 = {0, 0xDDDD, 0x0000, 0x0000};
static GdkColor color_lblue = {0, 0xBBBB, 0xBBBB, 0xFFFF};
static GdkColor color_lgreen= {0, 0xBBBB, 0xFFFF, 0xBBBB};
static GdkColor color_lorange={0, 0xFFFF, 0xCCCC, 0xAAAA};
static GdkColor color_lred=   {0, 0xFFFF, 0xBBBB, 0xBBBB};
#endif


static const gchar* cal_month[]={"Jan",
				 "Feb",
				 "Mar",
				 "Apr",
				 "May",
				 "Jun",
				 "Jul",
				 "Aug",
				 "Sep",
				 "Oct",
				 "Nov",
				 "Dec"};

static const gchar* day_name[]={"Sun",
				"Mon",
				"Tue",
				"Wed",
				"Thu",
				"Fri",
				"Sat"};


typedef struct _SetupEntry SetupEntry;
struct _SetupEntry{
  gchar *initial;
  gchar *cross;
  gdouble   cross_scan;
};


typedef struct _NOTEpara NOTEpara;
struct _NOTEpara{
  gchar *txt;
  time_t time;
  gboolean auto_fl;
  gint cnt;
};


typedef struct _MAILpara MAILpara;
struct _MAILpara{
  gchar *address;
  gint  year;
  gint  month;
  gint  day;
};

typedef struct _FRAMEpara FRAMEpara;
struct _FRAMEpara{
  gchar *id;
  glong idnum;
  gchar *name;
  gchar *type;
  
  gchar *observer;
  gchar *prop;

  guint exp;
  gint  repeat;

  gchar *jst;
  gchar *date;
  gchar *ut;
  gdouble mjd;

  gdouble secz;


  gchar *i2;

  NOTEpara note;

  gchar *setup;


  GtkWidget *w_note;

  gint qlr;
  gint cal;
};

typedef struct _typHLOG typHLOG;
struct _typHLOG{
  GtkWidget *w_top;
  GtkWidget *w_box;
  GtkWidget *scrwin;
  GtkWidget *top_table;
  GtkWidget *e_next;
  GtkWidget *e_note;
  GtkWidget *b_refresh;
  GtkWidget *w_status;

  gchar *fcdb_host;
  gchar *fcdb_path;
  gchar *fcdb_file;
  gchar *proxy_host;
  gint proxy_port;
  gboolean proxy_flag;
  gboolean push_flag;
  glong psz;

  gchar *seimei_log_id;
  gchar *seimei_log_txt;

  gchar *uname;
  gchar *wdir;
  gchar *sdir;
  gchar *ddir;
  gchar *udir;

  gchar *spass;

  gchar *ref_frame;

  glong idnum0;
  gchar *prop0;

  gchar *data_dir;

  gchar *mail;

  guint timer;

  gint num;
  gint num_old;

  gint lock_fp;
  gboolean lock_flag;

  gint scr_flag;
  GtkWidget *scr_combo;

  gboolean upd_flag;
  gboolean date_flag;
  gint disp_flag;
  gint sort_flag;
  gint disp_time;
  
  gchar *last_frame_id;

  gchar *next_note;

  gint buf_year;
  gint buf_month;
  gint buf_day;
  gint fr_year;
  gint fr_month;
  gint fr_day;

  time_t fr_time;
  time_t seek_time;
  time_t to_time;
  
  time_t date_from;
  time_t date_to;


  FRAMEpara frame[MAX_FRAME];

  MAILpara ml[MAX_MAIL];


  GtkWidget *fr_e;

  GtkWidget *frame_tree;
  gint frame_tree_i;

  gchar *http_host;
  gchar *http_path;
  gchar *http_dlfile;
  glong http_dlsz;
  GtkWidget *pdialog;
  GtkWidget *pbar;
  gboolean http_ok;

  GtkWidget *address_entry;
  GtkWidget *smdialog;
  GtkWidget *mldialog;
  GtkWidget *mltree;
  gint mltree_i;
  gint ml_max;
  GtkWidget *mltree_search_label;
  gchar *mltree_search_text;
  gint mltree_search_imax;
  gint mltree_search_i;
  gint mltree_search_iaddr[MAX_MAIL];

  GCancellable   *sccancel;
  GThread        *scthread;
  GMainLoop      *scloop;
  gint scanning_timer;
  gboolean scanning_flag;

  GThread   *pthread;
  GCancellable   *pcancel;
  GMainLoop *ploop;
  gboolean pabort;

  gint iraf_col;
  gint iraf_hdsql_r;
  gchar *file_write;
  gchar *file_wait;

  gboolean auto_red;
  GtkWidget *check_auto_red;

  gint done_flat;
  gint done_thar;
  glong done_idnum;
  gint auto_flat[AUTO_FLAT_NUM];

  gchar *ql_thar1d;
  gchar *ql_thar2d;
  gchar *ql_flat;
  gchar *ql_ap;
  gchar *ql_mask;
  gchar *ql_blaze;
  gchar *ql_thar_new;
  gchar *ql_flat_new;
  gchar *ql_ap_new;
  gchar *ql_mask_new;
  gchar *ql_blaze_new;
  gint ql_st_x;
  gint ql_ed_x;

  gchar *ql_lock;
  gint ql_timer;
  gint ql_loop;
  gint ql_i;

  gint ql_err;
  
  gint ql_line;
  gint ql_ge_line;
  gint ql_ge_stx;
  gint ql_ge_edx;

  gchar *ql_python;
  gchar *ql_terminal;
  
  GtkWidget *frame_ql;
  gchar *setname_red[NUM_SET];
  gchar *tharname_red[NUM_SET];

  GtkWidget *check_ap_red;
  gchar *ap_red[NUM_SET];
  gboolean flag_ap_red[NUM_SET];

  GtkWidget *combo_flat_red;
  GtkWidget *button_flat_red;
  gchar *flat_red[NUM_SET];
  gint ex_flat_red[NUM_SET];

  GtkWidget *check_thar_red;
  GtkWidget *button_thar_red;
  
  gchar *thar_red[NUM_SET];
  gboolean flag_thar_red[NUM_SET];

  GtkWidget *label_edit_ap;
  GtkWidget *label_edit_flat;
  GtkWidget *label_edit_thar;
  GtkWidget *label_edit_mask;
  GtkWidget *label_edit_blaze;

  GtkWidget *entry_ap_id;
  GtkWidget *entry_thar_reid;
  GtkWidget *entry_thar_id;

  gboolean remote_flag;
  gchar *remote_host;
  gchar *remote_user;
  gchar *remote_pass;
  gchar *remote_dir;

  gint i_reduced;
};


void refresh_table ();
void check_reduced_spectra();
int copy_file();

gchar* to_utf8();
gchar* to_locale();
void ReadLog();
void save_note();

void cc_get_entry ();
void cc_get_adj ();
void cc_get_adj_double ();
void cc_get_toggle();
void cc_get_combo_box ();

gchar *fgets_new();

void WriteLog();

void popup_dl_camz_list();
gchar* get_setname_short();
gchar* get_setname_long();

int scp_write_cal();
void grlog_OpenFile();
int http_c_fcdb_new();

void update_seimei_log();
void save_cfg_cal();
void check_ql_finish();

void ql_ext_play();
gint ql_ext_check();
gint get_cnt();
