
/*
    a simple JPEG APP/EXIF stripper

    by robin leffmann (aka djinn/wd) on 17 dec 2007 <djinn at stolendata.net>
    EXIF copyright insertion feature added 9 may 2011

    this is a blunt attempt to strip a JPEG file of all its APP segments,
    meaning all EXIF tags, thumbnails, lossless- and JPS data etc.
    it assumes that all JPEG segments are entropy-coded and that ICC color
    profiles always identify as "0xFFE2" segments, which seems to be true
    from the various files and picture sources i've tested, but, as i
    actually haven't read the JPEG/EXIF specifications i don't know if
    this is always the case, so your mileage may vary... :)

    this source code is released under the cc by-nc-sa license, and comes
    without any warranties of any kind

    http://creativecommons.org/licenses/by-nc-sa/3.0/
*/

#include <stdio.h>

int main( int argc, char **argv )
{
    unsigned char app_stub[] = { 0xff, 0xe1, 0x00, 0x00, 0x45, 0x78, 0x69, 0x66,
                                 0x00, 0x00, 0x49, 0x49, 0x2a, 0x00, 0x08, 0x00,
                                 0x00, 0x00, 0x01, 0x00, 0x98, 0x82, 0x02, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00 };
    int c1, c2, marker_size, stripped = 0, color = 1, notice = 0, i = 0, l = 0;
    FILE *in, *out;

    in = fopen( argv[1], "rb" );
    out = fopen( argv[2], "r" );

    /* check for usage error */
    if( argc < 3 || !in || out )
    {
        printf( "JPEG APP/EXIF stripper / 17 dec '07 <djinn at stolendata.net>\n"
                "usage: appexifstrip <input> <output> [\"copyright notice\"] [-]\n"
                "passing the '-' as argument will also strip ICC color profiles\n\n" );
        if( in )
            fclose( in );
        else if( argc > 2 && !in )
            printf( "input file not found\n" );
        if( in && out )
        {
            printf( "output file already exists\n" );
            fclose( out );
        }
        return 0;
    }

    /* remove color profiles? add (c) notice? what a dirty hack... */
    if( argc > 3 )
    {
        if( argv[3][0] == '-' && argv[3][1] == 0 )
            color = 0;
        else
            notice = 3;
        if( argc > 4 )
        {
            if( argv[4][0] == '-' && argv[4][1] == 0 )
                color = 0;
            else
                notice = 4;
        }
    }

    /* pass initial SOI marker to prepare for eventual EXIF copyright notice */
    out = fopen( argv[2], "wb" );
    fputc( 0xff, out );
    fputc( 0xd8, out );
    fseek( in, 2, SEEK_SET );

    /* add EXIF copyright notice? */
    if( notice )
    {
        /* determine lengths of copyright notice and APP marker */
        while( argv[notice][l] )
            l++;
        marker_size = sizeof( app_stub ) + l - ( l & 1 );

        /* insert lengths into stub */
        app_stub[2] = marker_size >> 8;
        app_stub[3] = marker_size & 0xff;
        app_stub[24] = l + ( 1 & 0xff );
 
        /* write stub and notice to output file */
        while( i < sizeof(app_stub) )
            fputc( app_stub[i++], out );
        i = 0;
        while( i < l + 1 )
            fputc( argv[notice][i++], out );

        /* pad with null if required */
        if( !(l & 1) )
            fputc( 0, out );
    }

    /* stream and filter (TO-DO: read entire file into a buffer instead...) */
    while( (c1 = fgetc(in)) != EOF )
    {
        /* marker? */
        if( c1 == 0xff )
        {
            c2 = fgetc( in );

            /* is it an APP marker or a JPEG comment? */
            if( (c2&0xf0) == 0xe0 || c2 == 0xfe )
            {
                /* is it a color profile we wish to keep? */
                if( c2 != 0xe2 || !color )
                {
                    /* if not, strip the marker/profile away */
                    marker_size = fgetc( in ) << 8;
                    marker_size |= fgetc( in );
                    if( c2 == 0xe2 )
                        printf( "stripping color profile " );
                    else
                        printf( "stripping APP/EXIF marker " );
                    printf( "(0x%x%x), %i bytes\n", c1, c2, marker_size + 2 );
                    fseek( in, marker_size - 2, SEEK_CUR );
                    stripped += marker_size + 2;
                    continue;
                }
            }

            /* add copyright notice? (this time as a JPEG comment) */
            if( c2 == 0xd9 && notice )
            {
                /* write comment marker */
                fputc( 0xff, out );
                fputc( 0xfe, out );

                /* write length of marker */
                i = l + 2;
                fputc( i >> 8, out );
                fputc( i & 0xff, out );

                /* write notice */
                i = 0;
                while( argv[notice][i] )
                    fputc( argv[notice][i++], out );

                printf( "added copyright notice: \"%s\"\n", argv[notice] );
            }

            /* non-APP marker or color profile - let it through */
            fputc( c1, out );
            fputc( c2, out );
            continue;
        }

        /* other arbitrary data - let it through */
        fputc( c1, out );
    }

    fclose( in );
    fclose( out );
    printf( "removed %i bytes of data\n\n", stripped );

    return 0;
}
