/***************************************************************************
 *   Copyright (C) 2007 by Lawrence Lee   *
 *   valheru@facticius.net   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
/**
 * \author Lawrence Lee <valheru@facticius.net>
 */

/**
 * \file ydecoder.cpp
 */

#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/cerrno.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "ydecoder.h"

using namespace boost::filesystem;
using namespace ydecoder;

YDecoder::YDecoder()
    : crc( 0 ), line( 0 ), name( NULL ), part( 0 ), part_size( 0 ), pcrc( 0 ),
      size( 0 ), total_parts( 0), escaped( 64 ), magic( 42 )
{
}

YDecoder::~YDecoder()
{
}

void YDecoder::reinitialize()
{
    crc = 0;
    crc_val.reset();
    line = 0;
    delete name;
    name = NULL;
    part = 0;
    pcrc_val.reset();
    part_size = 0;
    pcrc = 0;
    size = 0;
    total_parts = 0;
}

YDecoder::Status YDecoder::decode( const char *input )
{
    filesystem::ifstream in( input );

    if( !in.is_open() ){
        error.emit( str( format( "Failed to open file %1%" ) % input ) );
        return FAILED;
    }

    stringstream write_buffer;
    Status status = SUCCESS;

    while( getline( in, read_buffer ) ){

        if( read_buffer.substr( 0, 8 ) != "=ybegin " )
            continue;

        if( parseHeader( &in ) != SUCCESS ){
            error.emit( "Failed to parse header!" );
            status = FAILED;
            break;
        }

        string::iterator iter;

        //The data read in this loop is the actual encoded file data
        while( getline( in, read_buffer ) ){

            if( read_buffer.substr( 0, 5 ) == "=yend" ){
                break;
            }else{
                for( iter = read_buffer.begin(); iter < read_buffer.end(); iter++ ){

                    if( *iter == '\r' || *iter == '\n' )
                        continue;

                    if( *iter == '=' ){
                        iter++;
                        *iter -= escaped;
                    }

                    *iter -= magic;
                    write_buffer << *iter;
                }
            }

        }

        data << write_buffer.str() ;
        crc_val.process_bytes( write_buffer.str().c_str(), write_buffer.str().length() );
        pcrc_val.process_bytes( write_buffer.str().c_str(), write_buffer.str().length() );
        status = parseTrailer( write_buffer );
        pcrc_val.reset();
    }

    in.close();
    return status;

}

/**
 * Function for obtaining values of parameters from the header and trailer of a yenc encoded file.
 * The return value is only valid immediately after this call returns, so do not under any
 * circumstances assign pointers to this value. Rather, copy the value into a variable of your choice.
 * @param attr The name of the parameter that you wish to obtain a value for.
 * @return The associated value of the parameter \a attr. NULL is returned is the parameter was not found.
 * \warning This function should not be used to retrieve the name parameter. Use the function YDecoder::getName()
 * for that.
 * \sa getName()
 */
const char* YDecoder::getAttribute( const char *attr )
{
    int pos = read_buffer.find( attr );

    if( pos ){
        const char *text = read_buffer.c_str();
        int skip = pos + strlen( attr ) + 1;

        while( skip ){
            *text++;
            skip--;
        }

        return text;
    }

    return NULL;
}

/**
 * Retrieve the filename the decoded data should be written to. If this function returns NULL, then
 * the decoder should abort.
 * @return The filename that the decoded data should be written to. Returns NULL if nothing was found.
 * \sa getAttribute()
 */
char* YDecoder::getName()
{
    int pos = read_buffer.find( "name" );

    if( pos ){
        const char *text = read_buffer.c_str();
        int skip = pos + 5;

        while( skip ){
            *text++;
            skip--;
        }

        stringstream stream;

        while( !( *text == '\r' || *text == '\n' ) ){
            stream << *text;
            *text++;
        }

        string name = stream.str();
        trim( name );
        int len = name.length() + 1;
        char *m_name = new char[len];
        strcpy( m_name, name.c_str() );
        debug.emit( str( format( "Found name : %1%" ) % m_name ) );
        return m_name;
    }

    debug.emit( "Failed to find name!" );
    return NULL;;
}

/**
 * Parses the header data in a yencoded file. Call this function when the line beginning with \c =ybegin
 * followed by a whitespace has been read, as per the yenc specifications. All header variables are set
 * after this function has been called. The filestream passed to the function is garuanteed to point to the
 * beginning of the yencoded data after this function has been called.
 * @param in The filestream that the function should read from.
 * @return \b true if the header variables line, size and name were set, otherwise \b false
 * \sa parseHeader()
 */
YDecoder::Status YDecoder::parseHeader( filesystem::ifstream *in )
{
    //Read the yEnc header
    part = atoi( getAttribute( "part" ) );
    debug.emit( str( format( "part : %1%" ) % part ) );
    line = atoi( getAttribute( "line" ) );
    debug.emit( str( format( "line : %1%" ) % line ) );
    size = atoi( getAttribute( "size" ) );
    debug.emit( str( format( "size : %1%" ) % size ) );

    if( !name ){
        name = getName();
    }else{

        if( strcmp( name, getName() ) != 0 ){
            warning.emit( "Name mismatch!" );
            return NAME_MISMATCH;
        }
    }

    //If the part variable was set we are dealing with a multipart file
    if( part ){
        getline( *in, read_buffer );
        int begin = atoi( getAttribute( "begin" ) );
        int end = atoi( getAttribute( "end" ) );
        part_size = ( end - begin ) + 1;
        debug.emit( str( format( "part size : %1%" ) % part_size ) );
        total_parts = atoi( getAttribute( "total" ) );
        debug.emit( str( format( "total parts : %1%" ) % total_parts ) );
    }

    if( !( line && size && name ) ){
        error.emit( "Unable to find all required header variables!" );
        return FAILED;
    }

    return SUCCESS;

}


/**
 * Parse the trailer of the yencoded file. Call this function when the line beginning with \c =yend has been read.
 * This function should only be called \em after all the data has been read and the final CRC value has been
 * calculated for the decoded data.
 * @return The status of the decoder.
 */
YDecoder::Status YDecoder::parseTrailer( const stringstream &write_stream )
{
    if( part ){

        if( part != atoi( getAttribute( "part" ) ) )
            return PART_MISMATCH;

        if( part_size != atoi( getAttribute( "size" ) ) )
            return SIZE_MISMATCH;

        if( part_size != write_stream.str().length() )
            return SIZE_MISMATCH;

        pcrc = strtoul( getAttribute( " pcrc32" ), NULL, 16 );
        debug.emit( str( format( "pcrc : %1$x" ) % pcrc ) );

        if( pcrc != pcrc_val.checksum() ){
            debug.emit( str( format( "pcrc_val : %1$x" ) % pcrc_val.checksum() ) );
            warning.emit( "pcrc mismatch!" );
            return PART_CRC_MISMATCH;
        }

    }else{

        if( size != atoi( getAttribute( "size" ) ) )
            return SIZE_MISMATCH;

    }

    crc = strtoul( getAttribute( " crc32" ), NULL, 16 );
    debug.emit( str( format( "crc : %1$x" ) % crc ) );

    if( crc && crc != crc_val.checksum() ){
        debug.emit( str( format( "crc_val : %1$x" ) % crc_val.checksum() ) );
        warning.emit( "crc mismatch!" );
        return CRC_MISMATCH;
    }

    return SUCCESS;
}

bool YDecoder::write( const char *path )
{
    if( !name ){
        error.emit( "Unable to write to file : filename not set" );
        return false;
    }

    filesystem::path p( path );

    try{

        if( !exists( p ) ){
            warning.emit( str( format( "Directory %1% doesn't exist, creating..." ) % p.native_file_string() ) );

            if( !create_directory( p ) ){
                error.emit( str( format( "Failed to create directory %1%, aborting!" ) % p.native_file_string() ) );
                return false;
            }

        }else{

            if( !is_directory( p ) ){
                error.emit( str( format( "%1% is not a directory, aborting!" ) % p.native_file_string() ) );
                return false;
            }

        }

        p /= name;
        filesystem::ofstream out( p );

        if( !out.is_open() ){
            error.emit( str( format( "Failed to open %1% for writing, aborting!" ) % p.native_file_string() ) );
            return false;
        }

        debug.emit( str( format( "Writing data to %1%"  ) % p.native_file_string() ) );
        out << data.str();
        out.close();
        return true;

    }catch( filesystem_error &err ){
        string sys_err;
        system_message( err.system_error(), sys_err );
        error.emit( str( format( "Error in writing to file  %1% : %2%") % p.native_file_string() % sys_err ) );
        return false;
    }
}

void dump( string message )
{
    cout << message <<endl;
}

int main( int argc, char *argv[] )
{
    if( argc < 2 )
        return EXIT_FAILURE;

    YDecoder decoder;
    decoder.message.connect( sigc::ptr_fun( dump ) );
    decoder.warning.connect( sigc::ptr_fun( dump ) );
    decoder.error.connect( sigc::ptr_fun( dump ) );
//     decoder.debug.connect( sigc::ptr_fun( dump ) );

    for( int i = 1; i < argc; i++ ){
        decoder.decode( argv[i] );
    }

    if( !decoder.write( get_current_dir_name() ) ){
        dump( "Writing failed!" );
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
