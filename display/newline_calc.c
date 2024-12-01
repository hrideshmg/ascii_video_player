#include <stdio.h>
#include <string.h>

int main(){
    printf("Enter size: ");
    int n;
    scanf("%d",&n);
    char s[n];
    scanf("%s",&s);
    for (int i=0;i<n;i++){
        printf("%c ",s[i]);
    }
    printf("\n");
    for (int i=0;i<n;i++){
        if (s[i]=='\\' && i!=n-1 && s[i+1]=='n'){
            printf("\n");
            i+=2;
        }
        printf("%c ",s[i]);
    }
}