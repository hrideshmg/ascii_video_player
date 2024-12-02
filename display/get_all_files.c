#include <stdio.h>
#include <dirent.h>
int main(void) {
   DIR *dr;
   struct dirent *en;
   dr = opendir("."); //open all or present directory
   if (dr) {
      while ((en = readdir(dr)) != NULL) {
         printf("%s\n", en->d_name); //print all directory name
      }
      closedir(dr); //close all directory
   }
   return(0);
}