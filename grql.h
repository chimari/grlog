void popup_message(GtkWidget *parent, gchar* stock_id,gint delay, ...);

void edit_ap();
void edit_flat();
void edit_thar();
void edit_cal();

void iraf_apall();
void iraf_ecreidentify();
void iraf_ecidentify();
void get_flat_scnm();

void set_ql_frame_label();
void set_cal_frame_red();
gchar *get_work_dir();
gchar *get_share_dir();
gchar *get_logincl_dir();
void edit_uparm();
gchar *get_indir();
void ql_obj_red();
void ql_ap_red();
void ql_flat_red();
void ql_thar_red();
void ql_mask();
void ql_blaze();
void ql_find_red();
void ql_param_red();

void check_reference_data();
void prepare_pyraf();
