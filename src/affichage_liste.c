/* ce fichier contient les paramètres de l'affichage de la liste d'opé */

/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org) */
/*			2003 Benjamin Drieu (bdrieu@april.org) */
/* 			http://www.grisbi.org */

/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU General Public License as published by */
/*     the Free Software Foundation; either version 2 of the License, or */
/*     (at your option) any later version. */

/*     This program is distributed in the hope that it will be useful, */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*     GNU General Public License for more details. */

/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#include "include.h"


/*START_INCLUDE*/
#include "affichage_liste.h"
#include "operations_liste.h"
#include "gsb_account.h"
#include "traitement_variables.h"
#include "utils_buttons.h"
#include "utils.h"
#include "affichage.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean allocation_clist_affichage_liste ( GtkWidget *clist,
					    GtkAllocation *allocation );
static gboolean change_choix_ajustement_auto_colonnes ( GtkWidget *bouton );
static gboolean change_largeur_colonne ( GtkWidget *clist,
				  gint colonne,
				  gint largeur );
static GtkWidget *cree_menu_quatres_lignes ( void );
static gboolean lache_bouton_classement_liste ( GtkWidget *clist,
					 GdkEventButton *ev );
static gboolean modification_retient_affichage_par_compte ( void );
static gboolean pression_bouton_classement_liste ( GtkWidget *clist,
					    GdkEventButton *ev );
static void raz_affichage_ope ( void );
static void remplissage_tab_affichage_ope ( GtkWidget *clist );
static void toggled_bouton_affichage_liste ( GtkWidget *bouton,
				      gint *no_bouton );
static gboolean transactions_list_display_modes_menu_changed  ( GtkWidget * menu_shell,
							 gint origine );
/*END_STATIC*/



gchar *labels_boutons [] = {
    N_("Date"),
    N_("Value date"),
    N_("Third party"),
    N_("Budgetary line"),
    N_("Debit"),
    N_("Credit"),
    N_("Balance"),
    N_("Amount"),
    N_("Method of payment"),
    N_("Reconciliation ref."),
    N_("Financial year"),
    N_("Category"),
    N_("C/R"),
    N_("Voucher"),
    N_("Notes"),
    N_("Bank references"),
    N_("Transaction number"),
    NULL };


/* utilisée pour éviter l'emballement de la connection allocation */

gint ancienne_allocation_liste;
gint affichage_realise;


gint col_depart_drag;
gint ligne_depart_drag;
gint tab_affichage_ope[TRANSACTION_LIST_ROWS_NB][TRANSACTION_LIST_COL_NB];
GtkWidget *boutons_affichage_liste[17];
GtkWidget *clist_affichage_liste;
GtkWidget *bouton_choix_perso_colonnes;
GtkWidget *bouton_caracteristiques_lignes_par_compte;
GtkWidget *bouton_affichage_lignes_une_ligne;
GtkWidget *bouton_affichage_lignes_deux_lignes_1;
GtkWidget *bouton_affichage_lignes_deux_lignes_2;
GtkWidget *bouton_affichage_lignes_trois_lignes_1;
GtkWidget *bouton_affichage_lignes_trois_lignes_2;
GtkWidget *bouton_affichage_lignes_trois_lignes_3;
/* contient le % de chaque colonne */
gint rapport_largeur_colonnes[TRANSACTION_LIST_COL_NB];
/* contient la taille de chaque colonne */
gint taille_largeur_colonnes[TRANSACTION_LIST_COL_NB];
/* contient le no de ligne à afficher lorsqu'on n'affiche qu'une ligne */
gint ligne_affichage_une_ligne;
/* contient les no de lignes à afficher lorsqu'on affiche deux lignes */
GSList *lignes_affichage_deux_lignes;
/* contient les no de lignes à afficher lorsqu'on affiche trois lignes */
GSList *lignes_affichage_trois_lignes;


/*START_EXTERN*/
extern gint allocation_precedente;
extern gint col_depart_drag;
extern GtkWidget *formulaire;
extern gint ligne_depart_drag;
extern GSList *liste_labels_titres_colonnes_liste_ope ;
extern GtkWidget *preview;
extern gchar *tips_col_liste_operations[TRANSACTION_LIST_COL_NB];
extern gchar *titres_colonnes_liste_operations[TRANSACTION_LIST_COL_NB];
extern GtkTreeViewColumn *transactions_tree_view_columns[TRANSACTION_LIST_COL_NB];
extern GtkWidget *window;
/*END_EXTERN*/






/** FIXME: document this */
GtkWidget *onglet_affichage_liste ( void )
{
    GtkWidget *onglet, *table, *bouton, *paddingbox;
    gchar *titres [] = { _("Col1"), _("Col2"), _("Col3"), _("Col4"), 
	_("Col5"), _("Col6"), _("Col7") };
	gint i, j;
	GtkWidget *scrolled_window;

	ligne_depart_drag = 0;
	col_depart_drag = 0;

	/*   à la base, on met une vbox */

	onglet = new_vbox_with_title_and_icon ( _("Information shown in transaction list"),
						"transaction-list.png" );

	paddingbox = new_paddingbox_with_title (onglet, FALSE,
						_("Transactions list preview"));

	/*     mise en place du scrolled window */
	/* 	il est utilisé pour éviter l'agrandissement de la fenetre de conf */
	/* 	si l'utilisateur change de trop la largeur des colonnes */

	scrolled_window = gtk_scrolled_window_new ( FALSE,
						    FALSE );
	gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					 GTK_POLICY_ALWAYS,
					 GTK_POLICY_NEVER );
	gtk_box_pack_start ( GTK_BOX ( paddingbox),
			     scrolled_window,
			     TRUE,
			     TRUE,
			     0 );
	gtk_widget_show ( scrolled_window );

	/* mise en place de la clist_affichage_liste */
	/*   on lui met des titres redimensionnables */
	/*   elle fait TRANSACTION_LIST_COL_NB colonnes et 4 lignes */
	clist_affichage_liste = gtk_clist_new_with_titles ( TRANSACTION_LIST_COL_NB, titres );
	gtk_clist_column_titles_passive ( GTK_CLIST ( clist_affichage_liste ));

	gtk_signal_connect ( GTK_OBJECT ( clist_affichage_liste ),
			     "button_press_event",
			     GTK_SIGNAL_FUNC ( pression_bouton_classement_liste ),
			     NULL );
	gtk_signal_connect ( GTK_OBJECT ( clist_affichage_liste ),
			     "button_release_event",
			     GTK_SIGNAL_FUNC ( lache_bouton_classement_liste ),
			     NULL );
	gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			    clist_affichage_liste );


	paddingbox = new_paddingbox_with_title (onglet, FALSE,
						_("Transactions list contents"));

	/* on crée maintenant une table de 3x6 boutons */
	table = gtk_table_new ( 3, 6, FALSE );
	gtk_box_pack_start ( GTK_BOX ( paddingbox ), table,
			     FALSE, FALSE, 0 );

	for ( i=0 ; i<3 ; i++ )
	    for ( j=0 ; j<6 ; j++ )
	    {
		if ( labels_boutons[j+ i*6] )
		{
		    boutons_affichage_liste[j + i*6] = 
			gtk_toggle_button_new_with_label ( _(labels_boutons[j + i*6]) );
		    gtk_signal_connect ( GTK_OBJECT ( boutons_affichage_liste[j + i*6] ),
					 "toggled",
					 GTK_SIGNAL_FUNC (toggled_bouton_affichage_liste),
					 GINT_TO_POINTER ( j + i*6 ));
		    gtk_table_attach_defaults ( GTK_TABLE ( table ),
						boutons_affichage_liste[j + i*6],
						j, j+1,
						i, i+1 );
		}
	    }

	/* on permet maintenant de choisir soi même la taille des colonnes */
	bouton_choix_perso_colonnes = new_checkbox_with_title ( _("Adjust column size according to this table"),
								&(etat.largeur_auto_colonnes),
								NULL );
	g_signal_connect ( G_OBJECT ( bouton_choix_perso_colonnes ),
			   "toggled",
			   G_CALLBACK ( change_choix_ajustement_auto_colonnes ),
			   NULL );
	gtk_box_pack_start ( GTK_BOX ( paddingbox ), bouton_choix_perso_colonnes,
			     FALSE, FALSE, 0 );


	/* on permet maintenant de choisir soi-même la taille des colonnes */
	bouton_caracteristiques_lignes_par_compte = 
	    new_checkbox_with_title ( _("Remember display settings for each account separately"),
				      &(etat.retient_affichage_par_compte),
				      G_CALLBACK ( modification_retient_affichage_par_compte )) ;
	gtk_box_pack_start ( GTK_BOX(paddingbox), bouton_caracteristiques_lignes_par_compte,
			     FALSE, FALSE, 0 );

	bouton = gtk_button_new_with_label ( _("Revert to defaults"));
	gtk_signal_connect ( GTK_OBJECT ( bouton ), "clicked",
			     GTK_SIGNAL_FUNC ( raz_affichage_ope ), NULL );
	gtk_box_pack_end ( GTK_BOX(onglet), bouton,
			   TRUE, FALSE, 0 );

	if ( !gsb_account_get_accounts_amount () )
	{
	    gtk_widget_set_sensitive ( onglet, FALSE );
	}

	if ( gsb_account_get_accounts_amount () )
	{
	    /* on remplit le tableau */

	    ancienne_allocation_liste = 0;
	    affichage_realise = 0;
	    remplissage_tab_affichage_ope ( clist_affichage_liste );
	    g_signal_connect ( G_OBJECT ( clist_affichage_liste ),
				 "size-allocate",
				 G_CALLBACK ( allocation_clist_affichage_liste ),
				 NULL );
	}
	else
	    gtk_widget_set_sensitive ( onglet,
				       FALSE ); 

	return ( onglet );
}
/***********************************************************************************************************************/




/***********************************************************************************************************************/
/* cette fonction est appelée quand on change la conf pour avoir un affichage */
/* propre à chaque compte */
/* si on choisit de séparer l'affichage par compte, on ne fait rien */
/* si on choisit de réunir l'affichage par compte, on met l'affichage de */
/* tous les comptes sur le compte courant */
/***********************************************************************************************************************/
gboolean modification_retient_affichage_par_compte ( void )
{
    gint nb_lignes;
    gint affichage_r;

    if ( etat.retient_affichage_par_compte )
	return FALSE;

    nb_lignes = gsb_account_get_nb_rows ( gsb_account_get_current_account () );
    affichage_r = gsb_account_get_r (gsb_account_get_current_account ());

    /*     on doit réafficher tous les comptes qui ne correspondent pas */

    GSList *list_tmp;

    list_tmp = gsb_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;

	i = gsb_account_get_no_account ( list_tmp -> data );

	if (  gsb_account_get_nb_rows ( i ) != nb_lignes
	      ||
	      gsb_account_get_r (i) != affichage_r )
	{
	    gsb_account_set_nb_rows ( i, 
				      nb_lignes );
	    gsb_account_set_r (i,
			       affichage_r );
	    gtk_list_store_clear ( gsb_transactions_list_get_store()  );
	}

	list_tmp = list_tmp -> next;
    }

    return FALSE;
}
/***********************************************************************************************************************/


/***********************************************************************************************************************/
gboolean change_choix_ajustement_auto_colonnes ( GtkWidget *bouton )
{
    /* etat.largeur_auto_colonnes est réglé automatiquement */
    /*     il ne reste qu'à modifier les colonnes elles même */

    gint i;

    if ( etat.largeur_auto_colonnes )
    {
	allocation_precedente = 0;

	for ( i = 0 ; i < TRANSACTION_LIST_COL_NB ; i++ )
	    gtk_tree_view_column_set_resizable ( transactions_tree_view_columns[i],
						 FALSE );

	changement_taille_liste_ope ( gsb_transactions_list_get_tree_view(),
				      &( GTK_WIDGET (gsb_transactions_list_get_tree_view() ))-> allocation );
    }
    else
    {
	for ( i = 0 ; i < TRANSACTION_LIST_COL_NB ; i++ )
	{
	    gtk_tree_view_column_set_resizable ( transactions_tree_view_columns[i],
						 TRUE );
	    taille_largeur_colonnes[i] = gtk_tree_view_column_get_fixed_width ( transactions_tree_view_columns[i]);
	}
   }
    return ( FALSE );
}
/***********************************************************************************************************************/


/***********************************************************************************************************************/
gboolean change_largeur_colonne ( GtkWidget *clist,
				  gint colonne,
				  gint largeur )
{
    rapport_largeur_colonnes[colonne] = largeur*100/clist->allocation.width;

    if ( etat.largeur_auto_colonnes )
    {
	if ( rapport_largeur_colonnes[colonne] * GTK_WIDGET (gsb_transactions_list_get_tree_view()) -> allocation.width / 100 )
	    gtk_tree_view_column_set_fixed_width ( transactions_tree_view_columns[colonne],
						   rapport_largeur_colonnes[colonne] * GTK_WIDGET (gsb_transactions_list_get_tree_view()) -> allocation.width / 100 );
    }

    return FALSE;
}
/***********************************************************************************************************************/


/** TODO: document this */
gboolean transactions_list_display_modes_menu_changed  ( GtkWidget * menu_shell,
							 gint origine )
{
    ligne_affichage_une_ligne = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU ( bouton_affichage_lignes_une_ligne ) -> menu_item ),
								      "no_ligne" ));

    lignes_affichage_deux_lignes = NULL;
    lignes_affichage_deux_lignes = g_slist_append ( lignes_affichage_deux_lignes,
						    g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU ( bouton_affichage_lignes_deux_lignes_1 ) -> menu_item ),
									"no_ligne" ));
    lignes_affichage_deux_lignes = g_slist_append ( lignes_affichage_deux_lignes,
						    g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU ( bouton_affichage_lignes_deux_lignes_2 ) -> menu_item ),
									"no_ligne" ));

    lignes_affichage_trois_lignes = NULL;
    lignes_affichage_trois_lignes = g_slist_append ( lignes_affichage_trois_lignes,
						     g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU ( bouton_affichage_lignes_trois_lignes_1 ) -> menu_item ),
									 "no_ligne" ));
    lignes_affichage_trois_lignes = g_slist_append ( lignes_affichage_trois_lignes,
						     g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU ( bouton_affichage_lignes_trois_lignes_2 ) -> menu_item ),
									 "no_ligne" ));
    lignes_affichage_trois_lignes = g_slist_append ( lignes_affichage_trois_lignes,
						     g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU ( bouton_affichage_lignes_trois_lignes_3 ) -> menu_item ),
									 "no_ligne" ));


    demande_mise_a_jour_tous_comptes ();
    modification_fichier ( TRUE );

    return ( FALSE );
}


/** FIXME: document this */
GtkWidget *onglet_affichage_operations ( void )
{
    GtkWidget * vbox_pref, *table, *label, *paddingbox;

    vbox_pref = new_vbox_with_title_and_icon ( _("Transaction list behavior"),
					       "transaction-list.png" );

    /* on permet de choisir quelle ligne seront affichées en fonction des caractéristiques de l'affichage */
    /* pour opé simplifiée */

    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE,
					    _("Display modes"));

    table = gtk_table_new ( 3, 6, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 6 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table,
			 FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON(_("One line mode")));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );

    bouton_affichage_lignes_une_ligne = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU(bouton_affichage_lignes_une_ligne),
			       cree_menu_quatres_lignes ());
    gtk_table_attach_defaults ( GTK_TABLE(table), bouton_affichage_lignes_une_ligne,
				1, 2, 0, 1 );

    /* pour 2 lignes */
    label = gtk_label_new ( COLON(_("Two lines mode")));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );

    bouton_affichage_lignes_deux_lignes_1 = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU(bouton_affichage_lignes_deux_lignes_1),
			       cree_menu_quatres_lignes ());
    gtk_table_attach_defaults ( GTK_TABLE(table), bouton_affichage_lignes_deux_lignes_1,
				1, 2, 1, 2 );

    bouton_affichage_lignes_deux_lignes_2 = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU(bouton_affichage_lignes_deux_lignes_2),
			       cree_menu_quatres_lignes ());
    gtk_table_attach_defaults ( GTK_TABLE(table), bouton_affichage_lignes_deux_lignes_2,
				2, 3, 1, 2 );


    /* pour 3 lignes */
    label = gtk_label_new ( COLON(_("Three lines mode")));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 2, 3,
		       GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );

    bouton_affichage_lignes_trois_lignes_1 = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_affichage_lignes_trois_lignes_1 ),
			       cree_menu_quatres_lignes ());
    gtk_table_attach_defaults ( GTK_TABLE ( table ), bouton_affichage_lignes_trois_lignes_1,
				1, 2, 2, 3 );

    bouton_affichage_lignes_trois_lignes_2 = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_affichage_lignes_trois_lignes_2 ),
			       cree_menu_quatres_lignes ());
    gtk_table_attach_defaults ( GTK_TABLE ( table ), bouton_affichage_lignes_trois_lignes_2,
				2, 3, 2, 3 );


    bouton_affichage_lignes_trois_lignes_3 = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_affichage_lignes_trois_lignes_3 ),
			       cree_menu_quatres_lignes ());
    gtk_table_attach_defaults ( GTK_TABLE ( table ), bouton_affichage_lignes_trois_lignes_3,
				3, 4, 2, 3 );


    if ( gsb_account_get_accounts_amount () )
    {
	/* on place les lignes à afficher */

	gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_affichage_lignes_une_ligne ),
				      ligne_affichage_une_ligne);

	gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_affichage_lignes_deux_lignes_1 ),
				      GPOINTER_TO_INT (lignes_affichage_deux_lignes->data));
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_affichage_lignes_deux_lignes_2 ),
				      GPOINTER_TO_INT (lignes_affichage_deux_lignes->next->data));

	gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_affichage_lignes_trois_lignes_1 ),
				      GPOINTER_TO_INT (lignes_affichage_trois_lignes->data));
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_affichage_lignes_trois_lignes_2 ),
				      GPOINTER_TO_INT (lignes_affichage_trois_lignes->next->data));
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_affichage_lignes_trois_lignes_3 ),
				      GPOINTER_TO_INT (lignes_affichage_trois_lignes->next->next->data));
    }
    else
    {
	gtk_widget_set_sensitive ( table, FALSE );
	gtk_widget_set_sensitive ( bouton_affichage_lignes_une_ligne, FALSE );
	gtk_widget_set_sensitive ( bouton_affichage_lignes_deux_lignes_1, FALSE );
	gtk_widget_set_sensitive ( bouton_affichage_lignes_deux_lignes_2, FALSE );
	gtk_widget_set_sensitive ( bouton_affichage_lignes_trois_lignes_1, FALSE );
	gtk_widget_set_sensitive ( bouton_affichage_lignes_trois_lignes_2, FALSE );
	gtk_widget_set_sensitive ( bouton_affichage_lignes_trois_lignes_3, FALSE );
    }

    /* Connect all menus */
    g_signal_connect ( G_OBJECT ( bouton_affichage_lignes_une_ligne ), "changed",
		       (GCallback) transactions_list_display_modes_menu_changed,
		       NULL );
    g_signal_connect ( G_OBJECT (bouton_affichage_lignes_deux_lignes_1), "changed",
		       (GCallback) transactions_list_display_modes_menu_changed,
		       NULL );
    g_signal_connect ( G_OBJECT(bouton_affichage_lignes_deux_lignes_2), "changed",
		       (GCallback) transactions_list_display_modes_menu_changed,
		       NULL );
    g_signal_connect ( G_OBJECT(bouton_affichage_lignes_trois_lignes_1), "changed",
		       (GCallback) transactions_list_display_modes_menu_changed,
		       NULL );
    g_signal_connect ( G_OBJECT(bouton_affichage_lignes_trois_lignes_2), "changed",
		       (GCallback) transactions_list_display_modes_menu_changed,
		       NULL );
    g_signal_connect ( G_OBJECT(bouton_affichage_lignes_trois_lignes_3), "changed",
		       (GCallback) transactions_list_display_modes_menu_changed,
		       NULL );

    /* Then add the "sort by" buttons */
    paddingbox = new_radiogroup_with_title (vbox_pref,
					    _("Sort transaction list"),
					    _("by value date"),
					    _("by date"),
					    &etat.classement_par_date, NULL);

    if ( !gsb_account_get_accounts_amount () )
    {
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

    return ( vbox_pref );
}



/* ************************************************************************************************************** */
/* renvoie un menu contenant 1ère ligne, 2ème ligne, 3ème ligne, 4ème ligne */
/* ************************************************************************************************************** */

GtkWidget *cree_menu_quatres_lignes ( void )
{
    GtkWidget *menu;
    GtkWidget *menu_item;

    menu = gtk_menu_new ();

    menu_item = gtk_menu_item_new_with_label ( _("first line"));
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_ligne",
			  GINT_TO_POINTER ( 0 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("second line"));
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_ligne",
			  GINT_TO_POINTER ( 1 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("third line"));
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_ligne",
			  GINT_TO_POINTER ( 2 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("fourth line"));
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_ligne",
			  GINT_TO_POINTER ( 3 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_widget_show ( menu_item );

    gtk_widget_show ( menu );

    return ( menu );
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
gboolean allocation_clist_affichage_liste ( GtkWidget *clist,
					    GtkAllocation *allocation )
{
    gint i;

    if ( ancienne_allocation_liste == allocation -> width )
	return FALSE;

   
    /* règle les largeurs de colonnes */

    ancienne_allocation_liste = allocation->width;

    /* on met la largeur des colonnes en fonction de ce qui avait été enregistré */

    for ( i=0 ; i<TRANSACTION_LIST_COL_NB ; i++ )
	gtk_clist_set_column_width ( GTK_CLIST ( clist ),
				     i,
				     rapport_largeur_colonnes[i] * ancienne_allocation_liste / 100 );

    /*     cette variable sert à éviter que la liste des opés ne soit redimmensionnée un peu */
    /* 	lors de l'affichage des préférences */

    if ( !affichage_realise )
    {
	affichage_realise = 1;
	gtk_signal_connect ( GTK_OBJECT ( clist ),
			     "resize-column",
			     GTK_SIGNAL_FUNC ( change_largeur_colonne ),
			     NULL );
    }
     return FALSE;
}
/* ************************************************************************************************************** */




/* ************************************************************************************************************** */
gboolean pression_bouton_classement_liste ( GtkWidget *clist,
					    GdkEventButton *ev )
{
    GdkCursor *cursor;
    GdkPixmap *source, *mask;
    GdkColor fg = { 0, 65535, 0, 0 }; /* Red. */
    GdkColor bg = { 0, 0, 0, 65535 }; /* Blue. */

    static unsigned char cursor1_bits[] = {
	0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01,
	0x00, 0x00, 0x3f, 0xfc, 0x3f, 0xfc, 0x00, 0x00, 0x80, 0x01, 0x80, 0x01,
	0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01};
    static unsigned char cursor1mask_bits[] = {
	0x80, 0x01, 0x8e, 0x71, 0x86, 0x61, 0x8a, 0x51, 0x90, 0x09, 0xa0, 0x05,
	0x40, 0x02, 0x3f, 0xfc, 0x3f, 0xfc, 0x40, 0x02, 0xa0, 0x05, 0x90, 0x09,
	0x8a, 0x51, 0x86, 0x61, 0x8e, 0x71, 0x80, 0x01};

    /*   si la souris se trouve dans les titres, on se barre simplement */

    if ( ev -> window != GTK_CLIST ( clist ) -> clist_window )
	return ( FALSE );


    /* on crée le nouveau curseur */



    source = gdk_bitmap_create_from_data (NULL,
					  cursor1_bits,
					  16,
					  16);
    mask = gdk_bitmap_create_from_data (NULL,
					cursor1mask_bits,
					16,
					16);

    cursor = gdk_cursor_new_from_pixmap (source,
					 mask,
					 &fg,
					 &bg,
					 8,
					 8);
    gdk_pixmap_unref (source);
    gdk_pixmap_unref (mask);




    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( clist ),
				   "button_press_event");

    /* récupère et sauve les coordonnées de la liste au départ */

    gtk_clist_get_selection_info ( GTK_CLIST ( clist ),
				   ev -> x,
				   ev -> y,
				   &ligne_depart_drag,
				   &col_depart_drag );

    /* on grab la souris */

    gdk_pointer_grab ( GTK_CLIST ( clist) -> clist_window,
		       FALSE,
		       GDK_BUTTON_RELEASE_MASK,
		       GTK_CLIST ( clist ) -> clist_window,
		       cursor,
		       GDK_CURRENT_TIME );

    return ( TRUE );
}
/* ************************************************************************************************************** */




/* ************************************************************************************************************** */
gboolean lache_bouton_classement_liste ( GtkWidget *clist,
					 GdkEventButton *ev )
{
    gint ligne_arrivee_drag;
    gint col_arrivee_drag;
    gchar *texte_depart[1];
    gchar *texte_arrivee[1];
    gchar *buffer;
    gint buffer_int;

    /*   si la souris se trouve dans les titres, on se barre simplement */

    if ( ev -> window != GTK_CLIST ( clist ) -> clist_window )
	return ( FALSE );

    /* récupère et sauve les coordonnées de la liste au départ */

    gtk_clist_get_selection_info ( GTK_CLIST ( clist ),
				   ev -> x,
				   ev -> y - 5,
				   &ligne_arrivee_drag,
				   &col_arrivee_drag );

    /* on dégrab la souris */

    gdk_pointer_ungrab ( GDK_CURRENT_TIME );

    /* si la cellule de départ est la même que celle de l'arrivée, on se barre */

    if ( ligne_depart_drag == ligne_arrivee_drag
	 &&
	 col_depart_drag == col_arrivee_drag )
	return ( TRUE );


    /* on échange les 2 textes */

    gtk_clist_get_text ( GTK_CLIST ( clist ),
			 ligne_depart_drag,
			 col_depart_drag,
			 texte_depart );

    gtk_clist_get_text ( GTK_CLIST ( clist ),
			 ligne_arrivee_drag,
			 col_arrivee_drag,
			 texte_arrivee );

    buffer = g_strdup ( texte_arrivee[0] );

    gtk_clist_set_text ( GTK_CLIST ( clist ),
			 ligne_arrivee_drag,
			 col_arrivee_drag,
			 g_strdup ( texte_depart[0] ));

    gtk_clist_set_text ( GTK_CLIST ( clist ),
			 ligne_depart_drag,
			 col_depart_drag,
			 buffer );

    /* on échange les 2 nombres */

    buffer_int = tab_affichage_ope[ligne_depart_drag][col_depart_drag];
    tab_affichage_ope[ligne_depart_drag][col_depart_drag] = tab_affichage_ope[ligne_arrivee_drag][col_arrivee_drag];
    tab_affichage_ope[ligne_arrivee_drag][col_arrivee_drag] = buffer_int;

    /*     on met à jour les colonnes */

    recuperation_noms_colonnes_et_tips ();
    update_titres_tree_view ();

    /*     on réaffiche les listes d'opé */

    demande_mise_a_jour_tous_comptes ();


    return ( TRUE );
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
void remplissage_tab_affichage_ope ( GtkWidget *clist )
{
    gchar *ligne [TRANSACTION_LIST_COL_NB];
    gint i;

    gtk_clist_freeze ( GTK_CLIST ( clist ));
    gtk_clist_clear ( GTK_CLIST ( clist ));

    for ( i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
    {
	gint j;

	for ( j=0 ; j<TRANSACTION_LIST_COL_NB ; j++ )
	{
	    switch ( tab_affichage_ope[i][j] )
	    {
		case TRANSACTION_LIST_DATE:
		    ligne[j] = _("Date");
		    break;

		case TRANSACTION_LIST_VALUE_DATE:
		    ligne[j] = _("Value date");
		    break;

		case TRANSACTION_LIST_PARTY:
		    ligne[j] = _("Payer/payee");
		    break;

		case TRANSACTION_LIST_BUDGET:
		    ligne[j] = _("Budgetary information");
		    break;

		case TRANSACTION_LIST_DEBIT:
		    ligne[j] = _("Debit");
		    break;

		case TRANSACTION_LIST_CREDIT:
		    ligne[j] = _("Credit");
		    break;

		case TRANSACTION_LIST_BALANCE:
		    ligne[j] = _("Balance");
		    break;

		case TRANSACTION_LIST_AMOUNT:
		    ligne[j] = _("Amount");
		    break;

		case TRANSACTION_LIST_TYPE:
		    ligne[j] = _("Payment method");
		    break;

		case TRANSACTION_LIST_RECONCILE_NB:
		    ligne[j] = _("Reconciliation ref.");
		    break;

		case TRANSACTION_LIST_EXERCICE:
		    ligne[j] = _("Financial year");
		    break;

		case TRANSACTION_LIST_CATEGORY:
		    ligne[j] = _("Categories");
		    break;

		case TRANSACTION_LIST_MARK:
		    ligne[j] = _("C/R");
		    break;

		case TRANSACTION_LIST_VOUCHER:
		    ligne[j] = _("Voucher");
		    break;

		case TRANSACTION_LIST_NOTES:
		    ligne[j] = _("Notes");
		    break;

		case TRANSACTION_LIST_BANK:
		    ligne[j] = _("Bank references");
		    break;

		case TRANSACTION_LIST_NO:
		    ligne[j] = _("Transaction number");
		    break;

		case TRANSACTION_LIST_CHQ:
		    ligne[j] = _("Cheque/Transfer number");
		    break;

		default:	
		    ligne[j] = "";
		    break;

	    }

	    if ( tab_affichage_ope[i][j]
		 &&
		 tab_affichage_ope[i][j] != TRANSACTION_LIST_CHQ )
	    {
		gtk_signal_handler_block_by_func ( GTK_OBJECT ( boutons_affichage_liste[tab_affichage_ope[i][j]-1] ),
						   GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						   GINT_TO_POINTER (tab_affichage_ope[i][j]-1) );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( boutons_affichage_liste[tab_affichage_ope[i][j]-1] ),
					       TRUE );
		gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( boutons_affichage_liste[tab_affichage_ope[i][j]-1] ),
						     GTK_SIGNAL_FUNC ( toggled_bouton_affichage_liste ),
						     GINT_TO_POINTER (tab_affichage_ope[i][j]-1) );
	    }
	}
	gtk_clist_append ( GTK_CLIST ( clist ),
			   ligne );
    }
    gtk_clist_thaw ( GTK_CLIST ( clist ));
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
/* Fonction appelée lorsqu'on click sur un bouton de l'affichage de la liste */
/* retire ou met le texte correspondant dans le 1er emplacement libre de */
/* la liste */
/* ************************************************************************************************************** */

void toggled_bouton_affichage_liste ( GtkWidget *bouton,
				      gint *no_bouton )
{
    /* on travaille en fait sur tab_affichage_ope_tmp et un appel */
    /* à remplissage_tab_affichage_ope va actualiser la liste */

    gint i, j;

    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton )))
    {
	/* on vient d'enfoncer le bouton */

	/* on met ce no+1 dans la 1ère case de libre en commençant en haut à gauche */
	/* si ce no est 9-1(moyen de paiement), on y met aussi 18 (no chèque) */


	for ( i = 0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
	    for ( j = 0 ; j<TRANSACTION_LIST_COL_NB ; j++ )
		if ( !tab_affichage_ope[i][j] )
		{
		    tab_affichage_ope[i][j] = GPOINTER_TO_INT ( no_bouton ) + 1;

		    if ( GPOINTER_TO_INT ( no_bouton ) == 8 )
			no_bouton = GINT_TO_POINTER ( 17 );
		    else
		    {
			i=TRANSACTION_LIST_ROWS_NB;
			j=TRANSACTION_LIST_COL_NB;
		    }
		}
    }
    else
    {
	/*       on vient de désenfoncer le bouton */

	/* recherche le no de bouton dans le tableau et met 0 à la place */
	/*       s'il s'agit du moyen de paiement(9), vire aussi le no de chèque(18) */

	for ( i = 0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
	    for ( j = 0 ; j<TRANSACTION_LIST_COL_NB ; j++ )
		if ( tab_affichage_ope[i][j] == ( GPOINTER_TO_INT ( no_bouton ) + 1 )
		     ||
		     ( GPOINTER_TO_INT ( no_bouton ) == 8
		       &&
		       tab_affichage_ope[i][j] == TRANSACTION_LIST_CHQ ))
		    tab_affichage_ope[i][j] = 0;
    }

    remplissage_tab_affichage_ope ( clist_affichage_liste );

/*     on réaffiche les listes */
    
    demande_mise_a_jour_tous_comptes ();
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
/* récupère les noms de colonnes ( en fait les objets de la 1ère ligne ), */
/* et ce qu'on met dans les tips des titres de colonnes */
/* donc, remplit les variables tips_col_liste_operations et titres_colonnes_liste_operations */
/* ************************************************************************************************************** */
void recuperation_noms_colonnes_et_tips ( void )
{
    gint i, j;
    gchar *ligne[TRANSACTION_LIST_COL_NB];

    /* on met les titres et tips à NULL */

    for ( j=0 ; j<TRANSACTION_LIST_COL_NB ; j++ )
    {
	titres_colonnes_liste_operations[j] = NULL;
	tips_col_liste_operations[j] = NULL;
    }


    for ( i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
	for ( j=0 ; j<TRANSACTION_LIST_COL_NB ; j++ )
	{
	    switch ( tab_affichage_ope[i][j] )
	    {
		case 0:
		    ligne[j] = NULL;
		    break;

		default:
		    ligne[j] = g_slist_nth_data ( liste_labels_titres_colonnes_liste_ope,
						  tab_affichage_ope[i][j] - 1 );
	    }

	    /* 	  si on est sur la 1ère ligne, on met les titres ainsi que la 1ere ligne des tips */
	    /* 	    sinon, on rajoute aux tips existant */

	    if ( i )
	    {
		if ( ligne[j] )
		{
		    if ( !titres_colonnes_liste_operations[j] )
			titres_colonnes_liste_operations[j] = ligne[j];

		    if ( tips_col_liste_operations[j] )
			tips_col_liste_operations[j] = g_strconcat ( tips_col_liste_operations[j],
								     ", ",
								     ligne[j],
								     NULL );
		    else
			tips_col_liste_operations[j] = ligne[j];
		}
	    }
	    else
	    {
		if ( ligne[j] )
		{
		    titres_colonnes_liste_operations[j] = ligne[j];
		    tips_col_liste_operations[j] = ligne[j];
		}
	    }
	}
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
void raz_affichage_ope ( void )
{
    gint i, j;
    gint tab[TRANSACTION_LIST_ROWS_NB][TRANSACTION_LIST_COL_NB] = {
	{ 0, 18, 1, 3, 13, 5, 6, 7 },
	{ 0, 0, 0, 12, 0, 9, 8, 0 },
	{ 0, 0, 11, 15, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 }};


	/* on remet tous les boutons à inactif */

	for ( i = 0 ; i < 17 ; i++ )
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( boutons_affichage_liste[i] ),
					   FALSE );

	for ( i = 0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
	    for ( j = 0 ; j<TRANSACTION_LIST_COL_NB ; j++ )
		tab_affichage_ope[i][j] = tab[i][j];

	/* on met à jour la liste et les boutons */

	remplissage_tab_affichage_ope ( clist_affichage_liste );
/* 	demande_mise_a_jour_tous_comptes (); */
/* 	verification_mise_a_jour_liste (); */

}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
/* renvoie le widget contenu dans l'onglet divers du formulaire/liste des paramètres */
/* ************************************************************************************************************** */
GtkWidget *onglet_diverse_form_and_lists ( void )
{
    GtkWidget *vbox_pref;
    GtkWidget *paddingbox;
    GtkWidget *radiogroup;


    vbox_pref = new_vbox_with_title_and_icon ( _("Form behavior"),
					       "form.png" );

    /* What to do if RETURN is pressed into transaction form */
    radiogroup = new_radiogroup_with_title (vbox_pref,
					    _("Pressing RETURN in transaction form"),
					    _("selects next field"),
					    _("terminates transaction"),
					    &etat.entree, NULL);

    /* Displayed fields */
    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE, 
					    COLON(_("Displayed fields")));

    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 new_checkbox_with_title (_("'Accept' and 'Cancel' buttons"),
						  &etat.affiche_boutons_valider_annuler,
						  ((GCallback) update_transaction_form)),
			 FALSE, FALSE, 0 );

    /* How to display financial year */
    radiogroup = new_radiogroup_with_title (vbox_pref,
					    _("By default, use financial year"),
					    _("last selected financial year"),
					    _("according to transaction date"),
					    &etat.affichage_exercice_automatique, 
					    NULL);

    if ( !gsb_account_get_accounts_amount () )
    {
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

    return vbox_pref;
}
/* ************************************************************************************************************** */


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
