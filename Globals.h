/*******************************************************
*   There are the standerd mesagess that get passed 
*   around by ImageShow.
*
*   
*******************************************************/
#ifndef GLOBALS_H
#define GLOBALS_H

#if (B_BEOS_VERSION <= B_BEOS_VERSION_4_5) || (__POWERPC__  && (B_BEOS_VERSION <= B_BEOS_VERSION_4_5))
   // to be able to compile on 4.5 we must do this
   // because R5.0 has support for wheel mouse
   #define B_MOUSE_WHEEL_CHANGED '_MWC'
#endif

#define TRACKER_MIME   "application/x-vnd.Be-TRAK"
#define TRACKER_FIND       'Tfnd'

#define MIME_TYPE_DIR  "application/x-vnd.Be-directory"
#define MIME_TYPE_QUERY "application/x-vnd.Be-query"

#define BE_FILE_TYPE   "BEOS:TYPE"
#define QUERY_STR      "_trk/qrystr"
#define QUERY_VOL      "_trk/qryvol1"
#define QUERY_VOL_ID   "device"
#define QUERY_INF      "_trk/qrymoreoptions_le"
#define QUERY_LIV      "_trk/queryDynamicDate"

#define THUMBNAIL_ATTR "GRAFX:Thumbnail"
#define WIDTH_ATTR     "GRAFX:Width"
#define HEIGHT_ATTR    "GRAFX:Height"

#define BOOK_DIRECTORY "Bookmarks"
#define ABC_DIRECTORY  "aBeSee/Beloved Places" //reconded off of ~/config/settings

#define STATUS 'noop' // for inferno filters
#define PROGRESS 'noop'

#define NO_FUNCTION_YET 'none'
#define I_AM_DIEING_AHH 'idie'
#define EDIT_QUERY      'eqry'
#define SAVE_IMAGE_AS   'simg'
#define SAVE_IMAGE_SPESIFIC 'imgs'
#define CHANGE_IMAGE    'cimg'
#define CHANGE_DIR      'cdir'
#define REFS_CHANGE_DIR 'rcdr'
#define NEXT_IMAGE      'nimg'
#define PREV_IMAGE      'pimg'
#define LAST_IMAGE      'limg'
#define FIRST_IMAGE     'fimg'
#define IMAGE_SIZE      'isiz'
#define FILTER_IMG      'fltr'
#define THUMB_VIEW      'thmb'
#define LIST_SELECT     'list'
#define IMAG_SELECT     'imge'
#define MAKE_LIST_LIST  'mlst'

#define RENAME_ENTRY    'renm'
#define MOVE_ENTRY      'move'
#define DELETE_ENTRY    'delt'

#define RENAMED         'nmed'

#define RESIZE_TO_IMAGE 'rsiz'
#define WINDOW_FIT      'wfit'
#define ASPECT          'spct'
#define OLD_ASPECT      'olda'
#define ACTUAL_SIZE     'actl'
#define STRETCH         'strc'
#define FULLSCREEN      'full'
#define MAX_SCREEN      'maxs'
#define TILE            'tile'

#define GET_IMG_INFO    'iinf'
#define GET_FILE_INFO   'finf'

#define SET_BG_CENTER   'sbgc'
#define SET_BG_SCALED   'sbgs'
#define SET_BG_TILED    'sbgt'
#define FILE_SET_BG_CENTER 'fbgc'
#define FILE_SET_BG_SCALED 'fbgs'
#define FILE_SET_BG_TILED  'fbgt'

#define RUN_FILTER      'rftr'

#define SHOWER_COLOR    'scol'

#define DO_SHOWSLIDE    'slid'
#define KILL_SHOWSLIDE  'noSS'

#define SHOW_LOGO       'logo'

#define LIST_OPTIONS    'ltop'
#define THUMB_SIZE      'tsiz'

#define REMOVE_DIR_THUMB  'rmdt'
#define REMOVE_FILE_THUMB 'rmft'

#define ADD_BOOKMARK    'adbm'
#define SHOW_BOOKMARKS  'swbm'
#define ADD_BOOKMARK_CURRENT 'acbm'
#define GO_TO_BOOKMARK  'gobm'

#define WORKING         'work'
#define FINISHED        'fshd'

#define TRACKER_HERE    'trkh'
#define TRACKER_THERE   'trkt'

#define NULL_IMAGE      'null'

#define SLIDE_OPTIONS   'slop'
#define SLIDE_SPEED     'sdsd'

#define PRINT           'prnt'
#define PRINT_SETUP     'pnst'

#define UNDO            'undo'
#define UNDO_TRUE       'undT'
#define UNDO_FALSE      'undF'

#define SHOW_TOOLBAR    'hdtb'
#define HIDE_TOOLBAR    'swtb'
#define SAVE_FILE_PANEL_FORMAT 'sfpf'
#define SAVE_FILE_PANEL_SETTINGS 'sfps'

#define CHANGE_LANGUAGE 'clng'

#endif