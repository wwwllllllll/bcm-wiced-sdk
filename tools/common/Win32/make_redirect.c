#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <process.h>
#include <stdlib.h>




int main(int argc, char *argv[])
{
	int status;
	int i;
	char path[] = ".\\tools\\common\\Win32\\make.exe\" HOST_OS=Win32";
	int len = sizeof(path);

    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];

	_splitpath( argv[0], drive, dir, fname, ext );

	len += strlen(drive);
	len += strlen(dir);

//	printf( "path %s\n",argv[0]);
//	printf( "drive %s, dir %s, fname %s, ext %s\n", drive, dir, fname, ext );


//	printf( "startlen: %d\n", len );

	for( i = 1; i < argc; i++ )
	{
		len +=  strlen( argv[i] ) + 4;
//		printf( "new len: %d (arg: \"%s\")\n", len, argv[i] );
	}

//	printf( "len: %d\n", len );

	char * cmd  = (char*) malloc( len+100 );
	if (cmd == 0)
	{
		printf( "out of memory\n");
		return -1;
	}

	strcpy( cmd, " \"\"" );
//	cmd[0] = '\"';
//	cmd[1] = '\x00';

    if ( strlen(drive) != 0 )
    {
        strcat( cmd, drive );
    }
    if ( strlen(dir) != 0 )
    {
        strcat( cmd, dir );
    }

    strcat( cmd, path );

	for( i = 1; i < argc; i++ )
	{
		strcat( cmd, " \"" );
		strcat( cmd, argv[i] );
		strcat( cmd, "\"" );
	}
	strcat( cmd, "\" " );

//	printf( "command: %s\n",cmd );
	status = system( cmd );

//	printf("status=%d, errno=%d\n",status,errno);

	free( cmd );

	return status;

}
