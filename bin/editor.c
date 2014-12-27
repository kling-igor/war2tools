#include "war2edit.h"

static Eina_List *_windows;

/*============================================================================*
 *                                  Callbacks                                 *
 *============================================================================*/

static void
_win_del_cb(void        *data,
            Evas_Object *obj   EINA_UNUSED,
            void        *event EINA_UNUSED)
{
   Editor *ed = data;
   editor_close(ed);
}

static void
_win_focused_cb(void        *data  EINA_UNUSED,
                Evas_Object *obj   EINA_UNUSED,
                void        *event EINA_UNUSED)
{
//   Editor *ed = data;
}

static void
_win_new_cb(void        *data  EINA_UNUSED,
            Evas_Object *obj   EINA_UNUSED,
            void        *event EINA_UNUSED)
{
}

static void
_win_open_cb(void        *data  EINA_UNUSED,
             Evas_Object *obj   EINA_UNUSED,
             void        *event EINA_UNUSED)
{
}

static void
_win_save_cb(void        *data  EINA_UNUSED,
             Evas_Object *obj   EINA_UNUSED,
             void        *event EINA_UNUSED)
{
}

static void
_win_save_as_cb(void        *data  EINA_UNUSED,
                Evas_Object *obj   EINA_UNUSED,
                void        *event EINA_UNUSED)
{
}

/*============================================================================*
 *                                Common Radio                                *
 *============================================================================*/

static Editor *
_editor_for_menu(Evas_Object *menu)
{
   Eina_List *l;
   Editor *e;

   EINA_LIST_FOREACH(_windows, l, e)
      if (e->menu == menu)
        return e;

   return NULL;
}

static void
_radio_cb(void        *data,
          Evas_Object *obj,
          void        *event EINA_UNUSED)
{
   struct _menu_item *mi = data;
   Editor *ed;

   ed = _editor_for_menu(obj);
   EINA_SAFETY_ON_NULL_RETURN(ed);

   /* Already active: do nothing */
   if (ed->tools_item_active != mi)
     {
        /* Disable the already active item */
        if (ed->tools_item_active)
          elm_radio_value_set(ed->tools_item_active->radio, 0);
        /* Enable the new active item */
        elm_radio_value_set(mi->radio, 1);
        ed->tools_item_active = mi;
     }
}

static Elm_Object_Item *
_radio_add(Editor          *editor,
           unsigned int     idx,
           Elm_Object_Item *parent,
           const char      *label,
           unsigned int     flags)
{
   Evas_Object *o;
   Elm_Object_Item *eoi;
   struct _menu_item *mi;

   mi = &(editor->tools_items[idx]);

   o = elm_radio_add(editor->menu);
   elm_radio_state_value_set(o, 1);
   elm_radio_value_set(o, 0);
   elm_object_text_set(o, label);

   eoi = elm_menu_item_add(editor->menu, parent, NULL, NULL, _radio_cb, mi);
   elm_object_item_content_set(eoi, o);

   mi->radio = o;
   mi->item = eoi;

   return eoi;
}





/*============================================================================*
 *                                 Public API                                 *
 *============================================================================*/

Eina_Bool
editor_init(void)
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   return EINA_TRUE;
}

void
editor_shutdown(void)
{
   Editor *ed;

   EINA_LIST_FREE(_windows, ed)
      editor_free(ed);
   _windows = NULL;
}

void
editor_close(Editor *ed)
{
   _windows = eina_list_remove(_windows, ed);

   /* Set the window to NULL as a hint for editor_free() */
   evas_object_del(ed->win);
   ed->win = NULL;
}

void
editor_free(Editor *ed)
{
   if (!ed) return;

   if (ed->win != NULL)
     editor_close(ed);
   pud_close(ed->pud);
   free(ed);
}

static void
_mc_cancel_cb(void        *data,
              Evas_Object *obj  EINA_UNUSED,
              void        *evt  EINA_UNUSED)
{
   Editor *ed = data;
   editor_close(ed);
   editor_free(ed);
}

static void
_mc_create_cb(void        *data,
              Evas_Object *obj  EINA_UNUSED,
              void        *evt  EINA_UNUSED)
{
   Editor *ed = data;
   editor_mainconfig_hide(ed);
}


static Eina_Bool
_mainconfig_create(Editor *ed)
{
   Evas_Object *o, *box, *b2, *b3, *img, *t, *f;
   Elm_Object_Item *itm;

   /* Create main box (mainconfig) */
   box = elm_box_add(ed->win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_horizontal_set(box, EINA_FALSE);

   /* Box to hold buttons */
   b2 = elm_box_add(box);
   elm_box_horizontal_set(b2, EINA_TRUE);
   evas_object_size_hint_weight_set(b2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(box, b2);
   evas_object_show(b2);

   /* Cancel button */
   o = elm_button_add(b2);
   elm_object_text_set(o, "Cancel");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(o, "clicked", _mc_cancel_cb, ed);
   elm_box_pack_start(b2, o);
   evas_object_show(o);

   /* Create button */
   o = elm_button_add(b2);
   elm_object_text_set(o, "Create");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(o, "clicked", _mc_create_cb, ed);
   elm_box_pack_end(b2, o);
   evas_object_show(o);

   /* Box to hold map and menus */
   b2 = elm_box_add(box);
   elm_box_horizontal_set(b2, EINA_TRUE);
   evas_object_size_hint_weight_set(b2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_start(box, b2);

   /* Image to hold the minimap overview */
   img = elm_image_add(b2);
   elm_box_pack_start(b2, img);
//   elm_image_file_set(img,

   /* Box to put commands */
   b3 = elm_box_add(b2);
   elm_box_horizontal_set(b3, EINA_FALSE);
   evas_object_size_hint_weight_set(b3, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(b2, b3);
   evas_object_show(b3);

  // f = elm_frame_add(b3);
  // elm_object_text_set(f, "Map Size");
  // evas_object_size_hint_weight_set(f, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  // evas_object_size_hint_align_set(f, EVAS_HINT_FILL, EVAS_HINT_FILL);
  // elm_box_pack_start(b3, f);

   o = elm_radio_add(b3);
   evas_object_show(f);


//   /* Table to hold commands */
//   t = elm_table_add(b2);
//   evas_object_size_hint_weight_set(t, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
//   evas_object_size_hint_align_set(t, EVAS_HINT_FILL, EVAS_HINT_FILL);
//   elm_box_pack_end(b2, t);
//   evas_object_show(b2);

//   /* Label for map size  */
//   o = elm_label_add(t);
//   elm_object_text_set(o, "Map Size:");
//   elm_table_pack(t, o, 0, 0, 1, 1);
//   evas_object_show(o);
//
//   /* Label for map era */
//   o = elm_label_add(t);
//   elm_object_text_set(o, "Map Era:");
//   elm_table_pack(t, o, 0, 1, 1, 1);
//   evas_object_show(o);

 //  /* Menu for map size */
 //  o = elm_menu_add(t);
 //  ed->mainconfig.menu_size = o;
 //  itm = elm_menu_item_add(o, NULL, NULL, "32 x 32", NULL, NULL);
 //  elm_menu_item_add(o, itm, NULL, "64 x 64", NULL, NULL);
 //  elm_menu_item_add(o, itm, NULL, "96 x 96", NULL, NULL);
 //  elm_menu_item_add(o, itm, NULL, "128 x 128", NULL, NULL);
 //  elm_table_pack(t, o, 1, 0, 1, 1);
 //  evas_object_show(o);

 //  /* Menu for map era */
 //  o = elm_menu_add(t);
 //  ed->mainconfig.menu_era = o;
 //  itm = elm_menu_item_add(o, NULL, NULL, "Summer", NULL, NULL);
 //  elm_menu_item_add(o, itm, NULL, "Winter", NULL, NULL);
 //  elm_menu_item_add(o, itm, NULL, "Wasteland", NULL, NULL);
 //  elm_menu_item_add(o, itm, NULL, "Swamp", NULL, NULL);
 //  elm_table_pack(t, o, 1, 1, 1, 1);
 //  evas_object_show(o);


   evas_object_show(t);
   ed->mainconfig.container = box;
   ed->mainconfig.img = img;

   return EINA_TRUE;
}

void
editor_mainconfig_show(Editor *ed)
{
   int i;

   /* Disable main menu */
   for (i = 0; i < 4; i++)
     elm_object_item_disabled_set(ed->main_sel[i], EINA_TRUE);

   /* Show inwin */
   elm_win_inwin_activate(ed->inwin);
   evas_object_show(ed->inwin);
}

void
editor_mainconfig_hide(Editor *ed)
{
   int i;

   evas_object_hide(ed->inwin);
   for (i = 0; i < 4; i++)
     elm_object_item_disabled_set(ed->main_sel[i], EINA_FALSE);
}

Editor *
editor_new(void)
{
   Editor *ed;
   char title[128], wins[32];
   Evas_Object *o;
   Elm_Object_Item *itm, *i;
   int k = 0;

   ed = calloc(1, sizeof(Editor));
   EINA_SAFETY_ON_NULL_GOTO(ed, err_ret);

   /* Create window's title */
   snprintf(wins, sizeof(wins), " - %i", eina_list_count(_windows));
   snprintf(title, sizeof(title), "Untitled%s",
            (eina_list_count(_windows) == 0) ? "" : wins);

   /* Create window and set callbacks */
   ed->win = elm_win_util_standard_add("win-editor", title);
   EINA_SAFETY_ON_NULL_GOTO(ed->win, err_free);
   elm_win_focus_highlight_enabled_set(ed->win, EINA_TRUE);
   evas_object_smart_callback_add(ed->win, "focus,in", _win_focused_cb, ed);
   evas_object_smart_callback_add(ed->win, "delete,request", _win_del_cb, ed);
   evas_object_resize(ed->win, 640, 480);

   /* Add a box to put widgets in it */
   o = elm_box_add(ed->win);
   EINA_SAFETY_ON_NULL_GOTO(o, err_win_del);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_horizontal_set(o, EINA_FALSE);
   elm_win_resize_object_add(ed->win, o);
   evas_object_show(o);

   /* Get the main menu */
   ed->menu = elm_win_main_menu_get(ed->win);
   EINA_SAFETY_ON_NULL_GOTO(ed->menu, err_win_del);

   itm = ed->main_sel[0] = elm_menu_item_add(ed->menu, NULL, NULL,  "File", NULL, NULL);
   elm_menu_item_add(ed->menu, itm, NULL, "New...", _win_new_cb, NULL);
   elm_menu_item_add(ed->menu, itm, NULL, "Open...", _win_open_cb, NULL);
   elm_menu_item_add(ed->menu, itm, NULL, "Save", _win_save_cb, ed);
   elm_menu_item_add(ed->menu, itm, NULL, "Save As...", _win_save_as_cb, ed);
   elm_menu_item_separator_add(ed->menu, itm);
   elm_menu_item_add(ed->menu, itm, NULL, "Close", _win_del_cb, ed);

   itm = ed->main_sel[1] = elm_menu_item_add(ed->menu, NULL, NULL, "Tools", NULL, NULL);

#define RADIO_ADD(flags, label) _radio_add(ed, k++, i, label, flags)

   i = itm;
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Human Start Location");

   i = ed->hmn_sel[0] = elm_menu_item_add(ed->menu, itm, NULL, "Human Air", NULL, NULL);
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Gnomish Flying Machine");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Gryphon Rider");

   i = ed->hmn_sel[1] = elm_menu_item_add(ed->menu, itm, NULL, "Human Land", NULL, NULL);
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Peasant");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Footman");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Elven Archer");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Knight");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Balista");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Dwarven Demolition Squad");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Mage");

   i = ed->hmn_sel[2] = elm_menu_item_add(ed->menu, itm, NULL, "Human Water", NULL, NULL);
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Human Tanker");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Elven Destroyer");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Battleship");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Human Transport");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Gnomish Submarine");

   i = ed->hmn_sel[3] = elm_menu_item_add(ed->menu, itm, NULL, "Human Buildings", NULL, NULL);
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Farm");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Town Hall");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Keep");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Castle");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Human Barracks");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Human Shipyard");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Elven Lumber Mill");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Human Foundry");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Human Refinery");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Human Oil Platform");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Human Blacksmith");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Stables");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Church");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Gnomish Inventor");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Gryphon Aviary");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Human Scout Tower");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Human Guard Tower");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Human Cannon Tower");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Mage Tower");

   elm_menu_item_separator_add(ed->menu, itm);

   i = itm;
   RADIO_ADD(EDITOR_ITEM_ORC, "Orc Start Location");

   i = ed->orc_sel[0] = elm_menu_item_add(ed->menu, itm, NULL, "Orc Air", NULL, NULL);
   RADIO_ADD(EDITOR_ITEM_ORC, "Goblin Zepplin");
   RADIO_ADD(EDITOR_ITEM_ORC, "Dragon");

   i = ed->orc_sel[1] = elm_menu_item_add(ed->menu, itm, NULL, "Orc Land", NULL, NULL);
   RADIO_ADD(EDITOR_ITEM_ORC, "Peon");
   RADIO_ADD(EDITOR_ITEM_ORC, "Grunt");
   RADIO_ADD(EDITOR_ITEM_ORC, "Troll Axethrower");
   RADIO_ADD(EDITOR_ITEM_ORC, "Ogre");
   RADIO_ADD(EDITOR_ITEM_ORC, "Catapult");
   RADIO_ADD(EDITOR_ITEM_ORC, "Goblin Sapper");
   RADIO_ADD(EDITOR_ITEM_ORC, "Death Knight");

   i = ed->orc_sel[2] = elm_menu_item_add(ed->menu, itm, NULL, "Orc Water", NULL, NULL);
   RADIO_ADD(EDITOR_ITEM_ORC, "Orc Tanker");
   RADIO_ADD(EDITOR_ITEM_ORC, "Troll Destroyer");
   RADIO_ADD(EDITOR_ITEM_ORC, "Juggernaught");
   RADIO_ADD(EDITOR_ITEM_ORC, "Orc Transport");
   RADIO_ADD(EDITOR_ITEM_ORC, "Giant Turtle");

   i = ed->orc_sel[3] = elm_menu_item_add(ed->menu, itm, NULL, "Orc Buildings", NULL, NULL);
   RADIO_ADD(EDITOR_ITEM_ORC, "Pig Farm");
   RADIO_ADD(EDITOR_ITEM_ORC, "Great Hall");
   RADIO_ADD(EDITOR_ITEM_ORC, "Stronghold");
   RADIO_ADD(EDITOR_ITEM_ORC, "Fortress");
   RADIO_ADD(EDITOR_ITEM_ORC, "Orc Barracks");
   RADIO_ADD(EDITOR_ITEM_ORC, "Orc Shipyard");
   RADIO_ADD(EDITOR_ITEM_ORC, "Troll Lumber Mill");
   RADIO_ADD(EDITOR_ITEM_ORC, "Orc Foundry");
   RADIO_ADD(EDITOR_ITEM_ORC, "Orc Refinery");
   RADIO_ADD(EDITOR_ITEM_ORC, "Orc Oil Platform");
   RADIO_ADD(EDITOR_ITEM_ORC, "Orc Blacksmith");
   RADIO_ADD(EDITOR_ITEM_ORC, "Ogre Mound");
   RADIO_ADD(EDITOR_ITEM_ORC, "Altar of Storms");
   RADIO_ADD(EDITOR_ITEM_ORC, "Goblin Alchemist");
   RADIO_ADD(EDITOR_ITEM_ORC, "Dragon Roost");
   RADIO_ADD(EDITOR_ITEM_ORC, "Orc Scout Tower");
   RADIO_ADD(EDITOR_ITEM_ORC, "Orc Guard Tower");
   RADIO_ADD(EDITOR_ITEM_ORC, "Orc Cannon Tower");
   RADIO_ADD(EDITOR_ITEM_ORC, "Temple of the Damned");

   elm_menu_item_separator_add(ed->menu, itm);

   i = itm;
   RADIO_ADD(EDITOR_ITEM_HUMAN | EDITOR_ITEM_ORC, "Gold Mine");
   RADIO_ADD(EDITOR_ITEM_HUMAN | EDITOR_ITEM_ORC, "Oil Patch");
   RADIO_ADD(EDITOR_ITEM_HUMAN | EDITOR_ITEM_ORC, "Critter");
   RADIO_ADD(EDITOR_ITEM_HUMAN | EDITOR_ITEM_ORC, "Circle of Power");
   RADIO_ADD(EDITOR_ITEM_HUMAN | EDITOR_ITEM_ORC, "Dark Portal");
   RADIO_ADD(EDITOR_ITEM_HUMAN | EDITOR_ITEM_ORC, "Runestone");

   elm_menu_item_separator_add(ed->menu, itm);

   i = elm_menu_item_add(ed->menu, itm, NULL, "NPC's", NULL, NULL);

   RADIO_ADD(EDITOR_ITEM_ORC, "Cho'Gall");
   RADIO_ADD(EDITOR_ITEM_ORC, "Zuljin");
   RADIO_ADD(EDITOR_ITEM_ORC, "Gul'Dan");
   RADIO_ADD(EDITOR_ITEM_ORC, "Grom Hellscream");
   RADIO_ADD(EDITOR_ITEM_ORC, "Kargath Bladefist");
   RADIO_ADD(EDITOR_ITEM_ORC, "Dentarg");
   RADIO_ADD(EDITOR_ITEM_ORC, "Teron Gorefiend");
   RADIO_ADD(EDITOR_ITEM_ORC, "Deathwing");

   elm_menu_item_separator_add(ed->menu, i);

   RADIO_ADD(EDITOR_ITEM_HUMAN, "Lothar");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Uther Lightbringer");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Turalyon");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Alleria");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Danath");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Khadgar");
   RADIO_ADD(EDITOR_ITEM_HUMAN, "Kurdan and Sky'Ree");

   elm_menu_item_separator_add(ed->menu, i);

   RADIO_ADD(EDITOR_ITEM_HUMAN | EDITOR_ITEM_ORC, "Skeleton");
   RADIO_ADD(EDITOR_ITEM_HUMAN | EDITOR_ITEM_ORC, "Daemon");

#undef RADIO_ADD

   itm = ed->main_sel[2] = elm_menu_item_add(ed->menu, NULL, NULL, "Players", NULL, NULL);
   itm = ed->main_sel[3] = elm_menu_item_add(ed->menu, NULL, NULL, "Help", NULL, NULL);

   Evas_Object *b;

   /* Scroller */
   ed->scroller = elm_scroller_add(ed->win);
   EINA_SAFETY_ON_NULL_GOTO(ed->scroller, err_win_del);
   evas_object_size_hint_weight_set(ed->scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ed->scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_scroller_policy_set(ed->scroller, ELM_SCROLLER_POLICY_ON, ELM_SCROLLER_POLICY_ON);
   elm_scroller_propagate_events_set(ed->scroller, EINA_TRUE);
   elm_box_pack_end(o, ed->scroller);
   evas_object_show(ed->scroller);
   elm_scroller_page_relative_set(ed->scroller, 0, 1);

   /* Mainconfig: get user input for various mainstream parameters */
   _mainconfig_create(ed);

   /* Add inwin */
   ed->inwin = elm_win_inwin_add(ed->win);
   EINA_SAFETY_ON_NULL_GOTO(ed->inwin, err_win_del);
   elm_win_inwin_content_set(ed->inwin, ed->mainconfig.container);

   /* Show window */
   evas_object_show(ed->win);

   editor_mainconfig_show(ed);

   /* Add to list of editor windows */
   _windows = eina_list_append(_windows, ed);

   return ed;

err_win_del:
   evas_object_del(ed->win);
err_free:
   free(ed);
err_ret:
   return NULL;
}
