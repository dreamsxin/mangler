
/*
 * This program was found on the Internet.  It contained no license information
 */

#include <stdio.h>

int main( int argc, char * argv[])
{
    FILE * in, * out;
    unsigned uCount, uLen;
    unsigned char ucData;

    if( argc < 4) {
	printf( "Convert Binary file to C unsigned char array\n");
	printf( "USE: bin2h <infile.bin> <outfile.h> <arrayname> \n");
	return -1;
    }

    in = fopen( argv[ 1], "rb");
    if( in == NULL) { 
	fprintf(stderr,"Cannot find input file '%s'..\n", argv[ 1]);
	return -1;
    }

    out = fopen( argv[ 2], "wb");
    if( out == NULL) { 
	fprintf(stderr,"Cannot create output file '%s'..\n", argv[ 2]);
	return -1;
    }

    fseek( in, 0, SEEK_END);
    uLen = ftell( in);
    fseek( in, 0, SEEK_SET);

    fprintf( out, "// filename: %s\n", argv[ 1]);
    fprintf( out, "\n\n");
    fprintf( out, "uint8_t %s[%u] = {\n   ", argv[3],  uLen);
    uCount = 0;
    while( 1) {
	if( 1 != fread( &ucData, 1, 1, in))
	    break;
	if( uCount)
	    fprintf( out, ", ");
	if( uCount % 16 == 0)
	    fprintf( out, "\n   ");
	uCount++;
	fprintf( out, "0x%02X", ucData);
    }
    fprintf( out, "\n   };\n");

    fclose( in);
    fclose( out);
    return 0;
}

// vim: sts=4 sw=4 cindent
