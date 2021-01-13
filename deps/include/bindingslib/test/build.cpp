#include <stdio.h>
#include <process.h>

char *my_env[] =
        {
                "THIS=environment will be",
                "PASSED=to child.exe by the",
                "_SPAWNLE=and",
                "_SPAWNLPE=and",
                "_SPAWNVE=and",
                "_SPAWNVPE=functions",
                NULL
        };

int main( int argc, char *argv[] )
{
    char *args[4];

    // Set up parameters to be sent:
    args[0] = "child";
    args[1] = "spawn??";
    args[2] = "two";
    args[3] = NULL;

    if (argc <= 2)
    {
        printf( "SYNTAX: SPAWN <1-8> <childprogram>\n" );
        exit( 1 );
    }

    switch (argv[1][0])   // Based on first letter of argument
    {
        case '1':
            _spawnl( _P_WAIT, argv[2], argv[2], "_spawnl", "two", NULL );
            break;
        case '2':
            _spawnle( _P_WAIT, argv[2], argv[2], "_spawnle", "two",
                      NULL, my_env );
            break;
        case '3':
            _spawnlp( _P_WAIT, argv[2], argv[2], "_spawnlp", "two", NULL );
            break;
        case '4':
            _spawnlpe( _P_WAIT, argv[2], argv[2], "_spawnlpe", "two",
                       NULL, my_env );
            break;
        case '5':
            _spawnv( _P_OVERLAY, argv[2], args );
            break;
        case '6':
            _spawnve( _P_OVERLAY, argv[2], args, my_env );
            break;
        case '7':
            _spawnvp( _P_OVERLAY, argv[2], args );
            break;
        case '8':
            _spawnvpe( _P_OVERLAY, argv[2], args, my_env );
            break;
        default:
            printf( "SYNTAX: SPAWN <1-8> <childprogram>\n" );
            exit( 1 );
    }
    printf( "from SPAWN!\n" );
}