
char * return_invalid(void){
    char buffer[128];
    return buffer;
}


void print_invalid(char * s){
    putc(*(s + 100));
}

