#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define COLUMNS_COUNT     5
#define DATA_COUNT    10000
#define LINE_SIZE       512

#define BIN(ch) (ch - '0')

struct cols_struct {
  char * csv_label;
  char * srt_label;
  int  col;
} cols[COLUMNS_COUNT] = {
  { "Time", "",     0 }, 
  { "GSpd", "km/h", 0 },
  { "Alt",  "m",    0 },
  { "Hdg",  "d",    0 },
  { "RQly", "%",    0 }
};

struct header_struct {
  char * label;
} header[200];
int idx;

struct data_struct {
  int time;
  char * values[COLUMNS_COUNT - 1];
} data[DATA_COUNT];

bool read_csv_header(FILE * f)
{
  if (feof(f)) return false;

  char ch = fgetc(f);
  char buff[21];
  int i;

  idx = 0;

  while ((ch != '\r') && !feof(f)) {

    i = 0;
    while (!feof(f) && (i < 20) && (ch != ',') && (ch != '\r')) {
      buff[i++] = ch;
      ch = fgetc(f);
    }

    buff[i] = 0;
    if (ch == ',') {
      ch = fgetc(f);
    }
    else if (ch != '\r') {
      return false;
    }

    header[idx].label = (char *) malloc(strlen(buff) + 1);
    strcpy(header[idx].label, buff);
    idx++;
  } 

  // for (i = 0; i < idx; i++) {
  //   fprintf(stderr, "%d: %s\n", i, header[i].label);
  // }

  return true;
}

bool get_csv_cols()
{
  bool err = false;

  for (int i = 0; i < COLUMNS_COUNT; i++) {
    for (int j = 0; j < idx; j++) {
      if (strncmp(header[j].label, cols[i].csv_label, strlen(cols[i].csv_label)) == 0) {
        cols[i].col = j;
        // fprintf(stderr, "Colonne %s = %d\n", cols[i].csv_label, j);
        break;
      }
    }
    if (cols[i].col == 0) {
      fprintf(stderr, "Colonne %s pas trouvée.", cols[i].csv_label);
      err = true;
    }
  }

  return !err;
}

bool read_line(FILE * f, char * line, int len)
{
  int i = 0;
  line[0] = 0;
  
  char ch = fgetc(f);
  while (!feof(f) && (ch == '\r')) ch = fgetc(f);
  if (ch == '\n') ch = fgetc(f);
  if (feof(f)) return false;

  while (!feof(f) && (ch != '\r') && (ch != '\n')) {
    line[i++] = ch;
    ch = fgetc(f);
    if (i >= (len - 1)) {
      fprintf(stderr, "Ligne trop longue dans fichier CSV (%d).", i);
      return false;
    }
  }
  line[i] = 0;

  if (!feof(f)) ungetc(ch, f);

  return i > 0;
}

char * get_data(char * line, int col)
{
  while (*line) {
    if (*line == ',') {
      if (--col == 0) break;
    }
    line++;
  }
  if (*line) {
    return line + 1;
  }
  else {
    return NULL;
  }
}

int get_millis(char * time)
{
  int hour, min, sec, mil;
  hour = (BIN(time[0]) * 10) + BIN(time[1]);
  min  = (BIN(time[3]) * 10) + BIN(time[4]);
  sec  = (BIN(time[6]) * 10) + BIN(time[7]);
  mil  = (BIN(time[9]) * 100) + (BIN(time[10]) * 10) + BIN(time[11]);

  return hour * (60*60*1000) + min * (60 * 1000) + sec * 1000 + mil;
}

bool read_csv(char * filename)
{
  FILE * f = fopen(filename, "r");

  if (f == NULL) {
    fprintf(stderr, "Incapable d'ouvrir le fichier %s.\n", filename);
    return false;
  }

  if (!read_csv_header(f)) {
    fprintf(stderr, "Incapable de lire l'entete du fichier csv.\n");
    return false;
  }

  if (!get_csv_cols()) {
    fprintf(stderr, "Incapable de trouver les colonnes.\n");
    return false;
  }

  int i = 0;
  int offset = 0;
  static char line[LINE_SIZE];

  while (read_line(f, line, LINE_SIZE)) {
    char * time = get_data(line, cols[0].col);
    if (time == NULL) {
      fprintf(stderr, "Incapable de trouver la chaine de temps.\n");
    }
    if (offset == 0) {
      offset = get_millis(time);
    }
    data[i].time = get_millis(time) - offset;

    // fprintf(stderr, "%d: %d", i, data[i].time);

    for (int j = 1; j < COLUMNS_COUNT; j++) {
      char * str = get_data(line, cols[j].col);
      char buff[21];
      int k = 0;
      while (str[k] && (str[k] != ',')) {
        buff[k] = str[k];
        k++;
      }
      buff[k] = 0;
      data[i].values[j - 1] = (char *) malloc(strlen(buff) + 1);
      strcpy(data[i].values[j - 1], buff);
      // fprintf(stderr, " %s", buff);
    }
    // fprintf(stderr, "\n");
    // fprintf(stderr, "%d: %s\n", i, line);

    i++;
  }

  fclose(f);

  return true;
}

bool generate(char * file_in, char * file_out)
{
  FILE * fi = fopen(file_in, "r");
  FILE * fo = fopen(file_out, "w");

  if (fi == NULL) {
    fprintf(stderr, "Incapable d'ouvrir le fichier %s en lecture.", file_in);
    return false;
  }

  if (fo == NULL) {
    fprintf(stderr, "Incapabled'ouvrir le fichier %s en ecriture.", file_out);
    return false;
  }
  static char line[LINE_SIZE];

  while (!feof(fi)) {
    // Ligne 1
    read_line(fi, line, LINE_SIZE);
    if (line[0] == '\0') break;
    fprintf(fo, "%s\r\n", line);

    // Ligne 2
    if (feof(fi)) {
      fprintf(stderr, "Format du fichier srt non compatible.\n");
      return false;
    }
    read_line(fi, line, LINE_SIZE);
    fprintf(fo, "%s\r\n", line);
    int time = get_millis(line);

    int i = 0;
    while ((i < idx) && (data[i].time <= time)) i++;
    if (i > 0) i--;

    // Ligne 3
    if (feof(fi)) {
      fprintf(stderr, "Format du fichier srt non compatible.\n");
      return false;
    }
    read_line(fi, line, LINE_SIZE);
    fprintf(fo, "%s", line);
    for (int j = 1; j < COLUMNS_COUNT; j++) {
      // Format 1
      // fprintf(fo, " %s:%s", cols[j].srt_label, data[i].values[j - 1]);
      
      // Format 2
      fprintf(fo, " %s%s", data[i].values[j - 1], cols[j].srt_label);

      // Format 3
      // fprintf(fo, " %s:%s%s", cols[j].csv_label, data[i].values[j - 1], cols[j].srt_label);
    }
    fprintf(fo, "\r\n");

    // Ligne 4
    if (feof(fi)) {
      fprintf(stderr, "Format du fichier srt non compatible.\n");
      return false;
    }
    read_line(fi, line, LINE_SIZE);
    fprintf(fo, "%s\r\n", line);
  }

  fprintf(fo, "\r\n\r\n");

  fclose(fi);
  fclose(fo);

  return true;
}

void usage(char * name)
{
  fprintf(stderr, "Usage: %s <srt_in> <csv_in> <srt_out>\n", name);  
}

int main(int argc, char **argv)
{
  if (argc != 4) {
    usage(argv[0]);
    return 1;
  }

  if (!read_csv(argv[2])) exit(1);
  if (!generate(argv[1], argv[3])) exit(1);

  fprintf(stderr, "Complété.\n");

  return 0;
}