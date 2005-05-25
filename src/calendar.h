#ifndef _CALENDAR_H
#define _CALENDAR_H (1)

/* fichier d'en tête gsbcalendar.h */
/* Constantes utilisées par la fonction inc_dec_date */
#define ONE_DAY 1
#define ONE_WEEK 7
#define ONE_MONTH 30
#define ONE_YEAR 365
#define SIZEOF_FORMATTED_STRING_DATE 11

/* START_INCLUDE_H */
#include "calendar.h"
/* END_INCLUDE_H */


/*START_DECLARATION*/
gboolean clavier_calendrier ( GtkCalendar *pCalendar,
			      GdkEventKey *ev,
			      GtkWidget *entry );
void date_selection ( GtkCalendar *pCalendar,
		      GtkWidget *entry );
GtkWidget *gsb_calendar_new ( GtkWidget *entry );
void inc_dec_date ( GtkWidget *entree, gint demande );
/*END_DECLARATION*/


#endif

