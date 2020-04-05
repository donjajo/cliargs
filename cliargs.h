#include <stdio.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <unistd.h>
#include <string.h>
#include <stdint-gcc.h>
#include "inc/functions.h"

// For some unknown reasons, after including <linux/limits.h> this is not present. Just redefining
#ifndef ARG_MAX
    #define ARG_MAX 131072
#endif

typedef struct argsFormat {
    char    shortName;
    char    *longName;
    char    *description;
    _Bool   hasValue;
    _Bool   required;
} Arg;

typedef void** arg_v;
typedef int format_t;

/**
 * Checks and get value of an argument, if it was provided in the command line. 
 * 
 * @param   Arg     *formats    All arguments formats provided in LoadArgs()
 * @param   arg_v   *values     Values provided in checkArgs()
 * @param   char    shortName   Short name of the argument to get
 * @param   char    *v          char pointer to copy argument value to. This is only copied when format.hasValue is true
 * 
 * @return  int     Returns true if argument is provided. False if not
 * 
*/
int getArg( Arg *args, size_t len, arg_v values, char shortName, char *buf ) {
    for( int i = 0; i < len; i++ ) {
        Arg arg = args[i];
        
        if( arg.shortName == shortName ) {
            if( arg.hasValue ) {
                if( strcmp( "", (char*)values[i] ) ) {
                    strcpy(buf, (char*)values[i]);
                    return 1;
                }
                return 0;
            }

            uintptr_t v = *(uintptr_t*)&values[i];
            return v == 1;
        }
    }

    return 0;
}

/**
 * Checks if argument is a long argument. i.e. --arg
 * 
 * @param char  *arg an item of argv
 * 
 * @return int 0|1
 * */
int isLong(char const *arg ) {
    return ( arg[0] == '-' && arg[1] == '-' );
}

/**
 * Checks if it is short. i.e. -a
 * 
 * @param char *arg an item of argv
 * 
 * @return int 0|1
 * 
**/
int isShort( char const *arg ) {
    return ( arg[0] == '-' && arg[1] != '-' );
}

int isArg( char const *arg ) {
    return (isShort(arg) || isLong(arg));
}

/**
 * Check if it is the last argument in a short arg. i.e. -aBc, checks if -B or -a or -c is the last
 * 
 * @param char *arg an item of argv
 * @param int   post    current position of the argument being checked
 * 
 * @return  int 0|1
 * 
*/
int isLastShort( char const *arg, int pos ) {
    int len = strlen(arg); // length, starting at 0
    return ( pos+1 > len );
}

int hasValue( const char *argv, Arg *formats ) {
    for(int i=0;&formats[i];i++) {
        Arg format = formats[i];
        
        if( !format.hasValue )
            continue;

        if( isLong(argv) ) {
            char *buf = ltrim(argv, '-');
            if( strcmp(buf, format.longName ) == 0 ) {
                return 1;
            }
            free(buf);
        } else if( isShort( argv ) ) {
            const char *tmp = argv;
            while( *tmp != '\0' ) {
                if( *tmp == format.shortName ) {
                    return 1;
                }
                tmp++;
            }
        }
    }

    return 0;
}

/**
 * Get list of positional arguments. Always free return value if not NULL!
 * 
 * @param   int     argc    Argument count
 * @param   char    **argv  Argument values
 * 
 * @return  char**|NULL  List of positional arguments.NULL on none
*/
char **getPosArgs( int argc, char **argv, Arg *formats ) {
    int found = 0;
    char **buf = malloc(sizeof(char)*131072); //Set ARG_MAX in linux/limits.h

    for(int i=1;i<argc;i++) {
        char *arg = argv[i];

        // If this is a value and the last argv is not an argument. This is positional, because it is not the value of that arg
        if( !isArg(arg) && ( !isArg(argv[i-1]) || !hasValue(argv[i-1], formats ) ) ) {
            buf[found] = arg; // malloc( (strlen(arg)+1)*sizeof(char) );
            
            // strcpy(arg, buf[found]);
            found++;
        }
    }

    if( found ) {
        return buf;
    } 

    // Free this buf is no positional arg is found
    free(buf);
    return NULL;
}

void showHelp( Arg *args, size_t arg_len, const char *description ) {
    printf( "%s\n", description);
    for(int i=0;i<arg_len;i++) {
        Arg arg = args[i];

        if( arg.shortName )
            printf( "-%c", arg.shortName );

        if(arg.longName[0]!='\0') {
            if( arg.shortName ) 
                printf( "," );
            printf( "--%s", arg.longName );
        }

        if(arg.description) 
            printf( "\t\t%s", arg.description );

        printf( "\n");
    }

    exit(1);
}
/**
 * Checks loaded arguments to flag off missing required ones and allocate values
 * 
 * @param int   argc    count of cli arguments
 * @param char  **argv  Pointer to argument values
 * 
*/ 
void checkArgs( Arg *formats, size_t formatCount, int argc, char **argv, arg_v values ) {
    int missingRequired[formatCount]; // allocate array to put missing required ones
    int missingCount=-1; // count missing ones
    
    // Iterate through formats
    for( int i = 0; i < formatCount; i++ ) {
        Arg format = formats[i]; // the current format
        int is_missing = format.required; // by default, lets assume it is missing :)
        int this = 0; // this value sets to 1 if the current iterating cli argument is equal to the current format being iterated
        if(format.hasValue) {
            *(char**)&values[i] = "";
        }

        // Iterate through argv
        for( int j = 1; j < argc; j++ ) {
            char *arg = argv[j]; // this argument 
            char *argtmp = arg;
            int k = 1;
            

            // Arguements starts with - or --
            if( arg[0] != '-' ) {
                continue;
            } else if( isLong(arg) ) {
                // This is a long format, compare strings
                char *trimmed = ltrim(arg, '-' ); // trim out left --
                this = strcmp( trimmed, format.longName );
                free(trimmed);
            } else {
                char *tmp =  arg;
                // Loop through all characters if it starts with - ... This makes -AbC work as args
                while(*tmp++ != '\0' ) {
                    k++; // Increment argi so we can use it to check if there is a next character and validate if the current short argument has a value
                    // Each char matches a shortname?
                    if( *tmp == format.shortName ) {
                        this = 1;
                        break; 
                    }
                }
            }
            
            // Captured any argument from the argv provided?
            if( this > 0 ) {
                // if the format is required and is still missing by default, mark as provided
                if( format.required && is_missing ) 
                    is_missing = 0;
                
                // Should have a value? Lets look for its value!
                if( format.hasValue ) {
                    if( isShort( argtmp ) && !isLastShort(argtmp, k) ) {
                        // In this scenario, we have -ABc while B matches -B but followed by -c makes it impossible for -B to have a value, any value assigned is going to -c which is the last argument. Lets mark it missing
                        is_missing = 1;
                    } else if( j+1 < argc && !startswith(argv[j+1], '-') ) {
                        // Not match a dead end, make sure the argc is less then or equal to our next index and it is not an argument starting with - as well
                        *(char**)&values[i] = argv[j+1]; // malloc( sizeof(char) * strlen(argv[j+1] )+1 ); // allocate right memory for char*
                        // strcpy(argv[j+1], values[i]); // Copy over!
                    } else {
                        // Oh, no value was provided. Mark back as missing
                        is_missing = 1;
                    }
                } else {
                    // It doesn't require a value. Default is 1 
                    *(uintptr_t*)&values[i] = 1;
                }
                // Break this arguments loop, we are done with "this" argument
                break;
            }
        }
        
        // We have jumped out of the loop just to find out this argument is still missing!
        if( is_missing > 0 ) {
            missingCount++;
            missingRequired[missingCount] = i; // Add to list of missing arguments that are required
        }
    }

    // Loop through all missing and stop the application
    if( missingCount >= 0 ) {
        dprintf(STDERR_FILENO, "Missing options:\n" );
        for( ; missingCount >= 0; missingCount-- ) {
            Arg missing = formats[missingRequired[missingCount]];
            dprintf(STDERR_FILENO, "\t--%s,-%c\t\t%s\n", missing.longName, missing.shortName, missing.description );
        }
        exit(1);
    }
}