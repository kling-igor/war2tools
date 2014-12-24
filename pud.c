#include "pudviewer.h"

#define PUD_VERBOSE(pud, msg, ...) \
   if (pud->verbose) fprintf(stdout, "-- " msg "\n", ## __VA_ARGS__)

#define PUD_SANITY_CHECK(pud, ...) \
   do { \
      if (!(pud) || !((pud)->file)) { \
         DIE_RETURN(__VA_ARGS__, "Invalid PUD input [%p]", pud); \
      } \
   } while (0)

#define PUD_CHECK_FERROR(f, ret) \
   do { \
      if (ferror(f)) DIE_RETURN(ret, "Error while reading file"); \
   } while (0)




struct _Pud
{
   FILE         *file;

   uint32_t      tag;
   uint16_t      version;
   char          description[32];
   uint16_t      era;
   Pud_Dimensions dims;

   Pud_Section   current_section;

   unsigned int  verbose    : 1;
};

static const char * const _sections[] =
{
   "TYPE", "VER ", "DESC", "OWNR", "ERA ",
   "ERAX", "DIM ", "UDTA", "ALOW", "UGRD",
   "SIDE", "SGLD", "SLBR", "SOIL", "AIPL",
   "MTXM", "SQM ", "OILM", "REGM", "UNIT"
};

bool
pud_section_is_optional(Pud_Section sec)
{
   return ((sec == PUD_SECTION_ERAX) ||
           (sec == PUD_SECTION_ALOW));
}

bool
pud_go_to_section(Pud         *pud,
                  Pud_Section  sec)
{
   FILE *f;
   uint8_t b;
   uint32_t l;
   char buf[4];
   const char *sec_str;

   PUD_SANITY_CHECK(pud, 1);
   if (sec > 19) DIE_RETURN(1, "Invalid section ID [%i]", sec);

   f = pud->file;
   sec_str = _sections[sec];

   /* If the section to search for is before the current section,
    * rewind the file to catch it. If it is after, do nothing */
   if (sec <= pud->current_section)
     rewind(f);

   l = file_read_long(f);
   if (ferror(f)) DIE_RETURN(false, "Error while reading file");
   memcpy(buf, &l, sizeof(uint32_t));

   while (!feof(f))
     {
        if (!strncmp(buf, sec_str, 4))
          {
             /* Update current section */
             pud->current_section = sec;
             return true;
          }

        memmove(buf, &(buf[1]), 3 * sizeof(char));
        b = file_read_byte(f);
        if (ferror(f)) DIE_RETURN(false, "Error while reading file");
        buf[3] = b;
     }
   pud->current_section = PUD_SECTION_UNIT; // Go to last section

   return false;
}



Pud *
pud_new(const char *file)
{
   Pud *pud;

   pud = calloc(1, sizeof(Pud));
   if (!pud) DIE_GOTO(err, "Failed to alloc Pud: %s", strerror(errno));

   pud->file = fopen(file, "r");
   if (!pud->file) DIE_GOTO(err_free, "Failed to open file [%s]", file);

   return pud;

err_free:
   free(pud);
err:
   return NULL;
}

void
pud_free(Pud *pud)
{
   fclose(pud->file);
   free(pud);
}

void
pud_verbose_set(Pud *pud,
                int  on)
{
   if (pud)
     pud->verbose = !!on;
}

void
pud_print(Pud  *pud,
          FILE *stream)
{
   if (!stream) stream = stdout;

   fprintf(stream,
           "Type...: %x\n"
           "Ver....: %x\n"
           "Desc...: %s\n"
           "Era....: %x\n"
           "Dims...: %x\n",
           pud->tag,
           pud->version,
           pud->description,
           pud->era,
           pud->dims);
}

int
pud_parse(Pud *pud)
{
 //  char buf[32];

 //  fread(buf, sizeof(char), 32, pud->file);
 //  for (int i = 0; i < 32; i++)
 //    printf("0x%hhx ", buf[i]);
 //  printf("\n");
 //  rewind(pud->file);

   pud_parse_type(pud);
   pud_parse_ver(pud);
   pud_parse_desc(pud);
   pud_parse_era(pud);
   pud_parse_dim(pud);


   pud_print(pud, stdout);


   return 0;
}

/*============================================================================*
 *                              Parsing Routines                              *
 *============================================================================*/

bool
pud_parse_type(Pud *pud)
{
   PUD_SANITY_CHECK(pud, 0);

   char buf[16];
   FILE *f = pud->file;
   uint32_t l;
   bool chk;

   chk = pud_go_to_section(pud, PUD_SECTION_TYPE);
   if (!chk) DIE_RETURN(false, "Failed to reach section TYPE");
   PUD_VERBOSE(pud, "At section TYPE");

   /* Read ID TAG */
   l = file_read_long(f);
   PUD_CHECK_FERROR(f, false);

   /* Read 10bytes + 2 unused */
   fread(buf, sizeof(uint8_t), 12, f);
   PUD_CHECK_FERROR(f, false);
   if (strncmp(buf, "WAR2 MAP\0\0", 10))
     DIE_RETURN(false, "TYPE section has a wrong header");

   pud->tag = l;

   return true;
}

bool
pud_parse_ver(Pud *pud)
{
   PUD_SANITY_CHECK(pud, false);

   FILE *f = pud->file;
   uint16_t w;
   bool chk;

   chk = pud_go_to_section(pud, PUD_SECTION_VER);
   if (!chk) DIE_RETURN(false, "Failed to reach section VER");
   PUD_VERBOSE(pud, "At section VER");

   fread(&w, sizeof(uint16_t), 1, f);
   PUD_CHECK_FERROR(f, false);

   pud->version = w;
   return true;
}

bool
pud_parse_desc(Pud *pud)
{
   PUD_SANITY_CHECK(pud, false);

   bool chk;
   FILE *f = pud->file;
   char buf[32];

   chk = pud_go_to_section(pud, PUD_SECTION_DESC);
   if (!chk) DIE_RETURN(false, "Failed to reach section DESC");
   PUD_VERBOSE(pud, "At section DESC");

   fread(buf, sizeof(char), 32, f);
   PUD_CHECK_FERROR(f, false);

   memcpy(pud->description, buf, 32 * sizeof(char));
   return true;
}

/*
 * TODO pud_parse_ownr()
 */

bool
pud_parse_era(Pud *pud)
{
   PUD_SANITY_CHECK(pud, false);

   bool chk;
   FILE *f = pud->file;
   uint16_t w;

   chk = pud_go_to_section(pud, PUD_SECTION_ERAX);
   if (!chk) // Optional section, use ERA by default
     {
        PUD_VERBOSE(pud, "Failed to find ERAX. Trying with ERA...");
        chk = pud_go_to_section(pud, PUD_SECTION_ERA);
        if (!chk) DIE_RETURN(false, "Failed to reach section ERA");
        PUD_VERBOSE(pud, "At section ERA");
     }

   fread(&w, sizeof(uint16_t), 1, f);
   PUD_CHECK_FERROR(f, false);

   pud->era = w;
   return true;
}

bool
pud_parse_dim(Pud *pud)
{
   PUD_SANITY_CHECK(pud, false);

   bool chk;
   FILE *f = pud->file;
   uint16_t x, y;

   chk = pud_go_to_section(pud, PUD_SECTION_DIM);
   if (!chk) DIE_RETURN(false, "Failed to reach section DIM");
   PUD_VERBOSE(pud, "At section DIM");

   fread(&x, sizeof(uint16_t), 1, f);
   fread(&y, sizeof(uint16_t), 1, f);
   PUD_CHECK_FERROR(f, false);
   printf("%x %x\n", x, y);

   if ((x == 32) && (y == 32))
     pud->dims = PUD_DIMENSIONS_32_32;
   else if ((x == 64) && (y == 64))
     pud->dims = PUD_DIMENSIONS_64_64;
   else if ((x == 96) && (y == 96))
     pud->dims = PUD_DIMENSIONS_96_96;
   else if ((x == 128) && (y == 128))
     pud->dims = PUD_DIMENSIONS_128_128;
   else
     return false;

   return true;
}

bool
pud_parse_udta(Pud *pud)
{
   PUD_SANITY_CHECK(pud, false);
   return false;
}

