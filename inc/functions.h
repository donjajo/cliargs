/**
 * Strips out left side character if present in string
 * 
 * @param char *str  String to trim
 * @param char c    Character to strip off
 * 
 * @return char *p New trimmed string
 * */
char *ltrim( const char *str, const char c ) {
    if( *(str+0) != c ) 
        return NULL;

    int i = 0;
    int len = strlen(str);

    for( ; *(str+i) == c; i++ );

    char *p = malloc( ( len-i )+1 );

    for(int pi = 0; i<=len; i++,pi++ ) {
        *(p+pi) = *(str+i);
    }

    return p;
}

int endswith(char const *str, char const c) {
    int l = strlen(str);
    return str[l] == c;
} 

int startswith(char const *str, char const c ) {
    return str[0] == c;
}