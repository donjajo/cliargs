#include <stdio.h>
#include "../cliargs.h"

#define ARG_COUNT 5

int main( int argc, char **argv ) {
    /* 
        typedef struct argsFormat {
            char    shortName;
            char    *longName;
            char    *description;
            _Bool   hasValue;
            _Bool   required;
        } Arg;
    */
   
    Arg args[ARG_COUNT] = {
        { 'd', "dir", "Specify directory", 1, 0 },
        { 'R', "recursive", "list subdirectory recursively", 0, 0 },
        { 'r', "reverse", "Reverse order while listing", 0, 0 },
        { 'c', "count", "Provide number of counts", 1, 0 },
        { 'h', "help", "Show this help message", 0, 0 },
    };

    arg_v values = malloc(sizeof(arg_v)*ARG_COUNT);
    if( values == NULL ) {
        perror( "malloc" );
        exit(0);
    }

    loadArgs( args, ARG_COUNT, argc, argv, values );
    
    // Show help 
    // if( getArg(args, ARG_COUNT, values, 'h', NULL ) ) {
    //     showHelp(args, ARG_COUNT, "This is a test program" );
    // }

    for(int i=0;i<ARG_COUNT;i++) {
        printf( "%c,%-15s==>\t", args[i].shortName, args[i].longName );
        if( args[i].hasValue ) {
            char *buf = malloc(sizeof(char)*ARG_MAX);
            if( getArg(args, ARG_COUNT, values, args[i].shortName, buf ) ) {
                printf( "%s", buf );
                free(buf);
            } else {
                printf( "Not Provided" );
            }
        } 
        else {
            printf( "%s", getArg( args, ARG_COUNT, values, args[i].shortName, NULL ) ? "TRUE" : "FALSE" ); 
        }
        printf( "\n" );
    }

    printf( "\nPOSITIONAL ARGUMENTS\n========\n" );
    char **posArgs = getPosArgs(argc,argv,args);

    if( posArgs != NULL ) {
        for( int i = 0; posArgs[i]; i++ ) {
            printf( "%s\n", posArgs[i] );
        }
        free(posArgs);
    }

    free(values);
}