#ifndef _FENETRE_PRINCIPALE_H
#define _FENETRE_PRINCIPALE_H (1)

typedef enum GSB_GENERAL_NOTEBOOK_PAGES {
    GSB_HOME_PAGE,
    GSB_ACCOUNT_PAGE,
    GSB_SCHEDULER_PAGE,
    GSB_PAYEES_PAGE,
    GSB_CATEGORIES_PAGE,
    GSB_BUDGETARY_LINES_PAGE,
    GSB_REPORTS_PAGE,
#ifdef HAVE_G2BANKING
    GSB_AQBANKING_PAGE,
#endif
} GsbGeneralNotebookPages;

/* START_INCLUDE_H */
#include "fenetre_principale.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean change_page_notebook ( GtkNotebook *notebook,
				GtkNotebookPage *page,
				guint numero_page,
				gpointer null );
GtkWidget * create_main_widget ( void );
void gsb_gui_headings_update ( gchar * title, gchar * suffix );
/*END_DECLARATION*/

#endif
