enum
{ COLUMN_FRAME_NUMBER,
  COLUMN_FRAME_ID,
  COLUMN_FRAME_NAME,
  COLUMN_FRAME_TYPE,
  COLUMN_FRAME_QLR,
  COLUMN_FRAME_PROP,
  COLUMN_FRAME_OBSERVER,
  COLUMN_FRAME_DATE,
  COLUMN_FRAME_JST,
  COLUMN_FRAME_UT,
  COLUMN_FRAME_MJD,
  COLUMN_FRAME_EXPTIME,
  COLUMN_FRAME_SECZ,
  COLUMN_FRAME_I2,
  COLUMN_FRAME_COUNT,
  COLUMN_FRAME_NOTE,
  COLUMN_FRAME_COLFG,
  COLUMN_FRAME_COLBG,
  COLUMN_FRAME_WEIGHT,
  NUM_FRAME_COLUMNS
};


void tree_update_frame();
void move_focus_item();
void make_frame_tree();
void frame_tree_update_item();
void frame_tree_update_note();
void frame_tree_select_last();
void frame_tree_update_ql();
