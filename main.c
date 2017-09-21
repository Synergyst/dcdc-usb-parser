#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  FILE *fp;
  char path[1035];

  char *strData = "";
  char searchArray[4][100] = {
    "input voltage: ",
    "output voltage: ",
    "ignition voltage: ",
    "mode: "
  };
  #define SRCHARRLEN sizeof(searchArray) / 100

  /* Open the command for reading. */
  fp = popen("/usr/local/bin/dcdc-usb -a 2>&1", "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit(1);
  }

  char fullStrOutput[8192];

  /* Read the output a line at a time - output it. */
  while (fgets(path, sizeof(path)-1, fp) != NULL) {
    for (int c = 0; c < SRCHARRLEN; c++) {
      strData = strstr(path, searchArray[c]);
      if (strData != NULL) {
        strcat(fullStrOutput, path);
      } else {
        // Do nothing..
      }
    }
  }
  printf("%s", fullStrOutput);

  /* close */
  pclose(fp);

  return 0;
}
