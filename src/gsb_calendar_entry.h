#ifndef _GSB_CALENDAR_ENTRY_H
#define _GSB_CALENDAR_ENTRY_H (1)

/* fichier d'en tête gsbcalendar.h */
/* Constantes utilisées par la fonction inc_dec_date */
#define ONE_DAY 1
#define ONE_WEEK 7
#define ONE_MONTH 30
#define ONE_YEAR 365
#define SIZEOF_FORMATTED_STRING_DATE 128

/* START_INCLUDE_H */
#include "gsb_calendar_entry.h"
/* END_INCLUDE_H */


/*START_DECLARATION*/
gboolean gsb_calendar_entry_date_valid ( GtkWidget *entry );
GDate *gsb_calendar_entry_get_date ( GtkWidget *entry );
GtkWidget *gsb_calendar_entry_new ( void );
gboolean gsb_calendar_entry_set_color ( GtkWidget *entry,
					gboolean normal_color );
gboolean gsb_calendar_entry_set_date ( GtkWidget *entry,
				       GDate *date );
gboolean gsb_calendar_init_entry_colors ( void );
/*END_DECLARATION*/


#endif
