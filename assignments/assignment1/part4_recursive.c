#include <stdio.h>

void recurse(int a){
  if (a > 0){
    a--;
    recurse(a--);
  }
}

int important_global_variable = 0xFEEDFACE;

int main(void) {
  int depth = 5;

  printf("Value of important variable: %x\n", important_global_variable);
  printf("Attempting to recurse: %d\n", depth);
  recurse(depth);

  printf("Back home :)\n");
  printf("Value of important variable: %x\n", important_global_variable);

  while(1);

}