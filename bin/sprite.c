#include "war2edit.h"

void *
sprite_load(Eet_File   *src,
            const char *key,
            int        *x_ret,
            int        *y_ret,
            int        *w_ret,
            int        *h_ret)
{
   unsigned char *mem;
   int size, expected_size;
   uint16_t x, y, w, h;

   mem = eet_read(src, key, &size);
   EINA_SAFETY_ON_NULL_RETURN_VAL(mem, NULL);

   memcpy(&x, mem + 0, sizeof(uint16_t));
   memcpy(&y, mem + 2, sizeof(uint16_t));
   memcpy(&w, mem + 4, sizeof(uint16_t));
   memcpy(&h, mem + 6, sizeof(uint16_t));

   expected_size = (w * h * 4) + 8;
   if (expected_size != size)
     {
        CRI("Sprite data was loaded with size [%i], expected [%i]",
            size, expected_size);
        free(mem);
        return NULL;
     }

   DBG("Loaded sprite [%s] of size %ix%i (offsets: %i,%i)\n", key, w, h, x, y);

   if (x_ret) *x_ret = x;
   if (y_ret) *y_ret = y;
   if (w_ret) *w_ret = w;
   if (h_ret) *h_ret = h;
   return mem + 8;
}

Eet_File *
sprite_units_open(void)
{
   Eet_File *ef;
   const char *file;

   file = DATA_DIR"/sprites/units/units.eet";
   ef = eet_open(file, EET_FILE_MODE_READ);
   if (EINA_UNLIKELY(ef == NULL))
     {
        CRI("Failed to open [%s]", file);
        return NULL;
     }
   DBG("Open units file [%s]", file);

   return ef;
}

Eet_File *
sprite_buildings_open(Pud_Era era)
{
   Eet_File *ef;
   const char *file;

   switch (era)
     {
      case PUD_ERA_FOREST:    file = DATA_DIR"/sprites/buildings/forest.eet";    break;
      case PUD_ERA_WINTER:    file = DATA_DIR"/sprites/buildings/winter.eet";    break;
      case PUD_ERA_WASTELAND: file = DATA_DIR"/sprites/buildings/wasteland.eet"; break;
      case PUD_ERA_SWAMP:     file = DATA_DIR"/sprites/buildings/swamp.eet";     break;
     }

   ef = eet_open(file, EET_FILE_MODE_READ);
   if (EINA_UNLIKELY(ef == NULL))
     {
        CRI("Failed to open [%s]", file);
        return NULL;
     }
   DBG("Open buildings file [%s]", file);

   return ef;
}

unsigned char *
sprite_get(Editor        *ed,
           Pud_Unit       unit,
           Sprite_Info    info,
           int           *x,
           int           *y,
           int           *w,
           int           *h,
           Eina_Bool     *flip_me)
{
   unsigned char *data;
   char key[64];
   Eet_File *ef;
   Eina_Bool chk;
   int len;
   int orient;
   Eina_Bool flip;
   uint16_t sx, sy, sw, sh;

   if (pud_unit_building_is(unit))
     {
        ef = ed->buildings;
        len = snprintf(key, sizeof(key), "%s", pud_unit2str(unit));
        flip = EINA_FALSE;
     }
   else
     {
        ef = ed->units;

        if (info != SPRITE_INFO_ICON)
          {
             switch (info)
               {
                case SPRITE_INFO_SOUTH_WEST:
                   orient = SPRITE_INFO_SOUTH_EAST;
                   flip = EINA_TRUE;
                   break;

                case SPRITE_INFO_WEST:
                   orient = SPRITE_INFO_EAST;
                   flip = EINA_TRUE;
                   break;

                case SPRITE_INFO_NORTH_WEST:
                   orient = SPRITE_INFO_NORTH_EAST;
                   flip = EINA_TRUE;
                   break;

                default:
                   orient = info;
                   flip = EINA_FALSE;
                   break;
               }

             if ((unit == PUD_UNIT_GNOMISH_SUBMARINE) ||
                 (unit == PUD_UNIT_GIANT_TURTLE))
               {
                  len = snprintf(key, sizeof(key), "%s/%s/%i",
                                 pud_unit2str(unit), ed->era_str, orient);
               }
             else if ((unit == PUD_UNIT_HUMAN_START) ||
                      (unit == PUD_UNIT_ORC_START))
               {
                  len = snprintf(key, sizeof(key), "%s/0",
                                 pud_unit2str(unit));
               }
             else
               {
                  len = snprintf(key, sizeof(key), "%s/%i",
                                 pud_unit2str(unit), orient);
               }
          }
        else
          {
             CRI("ICONS not implemented!");
             return NULL;
          }
     }
   if (flip_me) *flip_me = flip;

   data = eina_hash_find(ed->sprites, key);
   if (data == NULL)
     {
        data = sprite_load(ef, key, x, y, w, h);
        if (EINA_UNLIKELY(data == NULL))
          {
             ERR("Failed to load sprite for key [%s]", key);
             return NULL;
          }
        chk = eina_hash_add(ed->sprites, key, data);
        if (chk == EINA_FALSE)
          {
             ERR("Failed to add sprite <%p> to hash", data);
             free(data);
             return NULL;
          }
        DBG("Access key [%s] (not yet registered). SRT = <%p>", key, data);
        return data;
     }
   else
     {
        if (x) { memcpy(&sx, data - 8, sizeof(uint16_t)); *x = sx; }
        if (y) { memcpy(&sy, data - 6, sizeof(uint16_t)); *y = sy; }
        if (w) { memcpy(&sw, data - 4, sizeof(uint16_t)); *w = sw; }
        if (h) { memcpy(&sh, data - 2, sizeof(uint16_t)); *h = sh; }
        DBG("Access key [%s] (already registered). SRT = <%p>", key, data);
        return data;
     }
}

static void
_free_cb(void *data)
{
   /* There is a 8 bytes offset */
   data -= 8;
   free(data);
}

Eina_Hash *
sprite_hash_new(void)
{
   return eina_hash_string_superfast_new(_free_cb);
}

Sprite_Info
sprite_info_random_get(void)
{
   /* Does not return 4 */
   return rand() % (SPRITE_INFO_NORTH_WEST - SPRITE_INFO_NORTH) + SPRITE_INFO_NORTH;
}

