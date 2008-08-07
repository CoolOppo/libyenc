/***************************************************************************
 *   Copyright (C) 2007 by Lawrence Lee                                    *
 *   valheru.ashen.shugar@gmail.com                                        *
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
 * @namespace ydecoder
 * The namespace for the ydecoder class.
 */
#ifndef YDECODER_YDECODER_H
#define YDECODER_YDECODER_H

#include <boost/crc.hpp>
#include <boost/filesystem/fstream.hpp>
#include <sigc++/sigc++.h>
#include <string>
#include <sstream>
// #include "bitwise_enums.hpp"

using namespace boost;
using namespace boost::filesystem;
using namespace sigc;
using namespace std;

namespace ydecoder{
    //Enums
    /**
     * @class DecoderStatus ydecoder.h
     *
     * @brief Provides a type-safe enum wrapper for the decoder status.
     *
     * @author Lawrence Lee <valheru.ashen.shugar@gmail.com>
     */
//     class DecoderStatus{
//         public:
    namespace DecoderStatus{
            enum Status{
                SUCCESS = 0, /**< The decoding succeeded */
                CRC_MISMATCH = 1, /**< The crc value of the decoded data doesn't match the crc value in the trailer */
                PART_CRC_MISMATCH = 2, /**< The crc value of the decoded part data doesn't match the part crc value in the trailer */
                PART_MISMATCH = 4, /**< The part number in the trailer doesn't match the part number in the header */
                SIZE_MISMATCH = 8, /**< The size of the data or the size value in the trailer doesn't match the size value in the header */
                NAME_MISMATCH = 16, /**< The name value in the header doesn't match the name of the previous parts */
                FAILED =  32/**< The decoding failed */
            };
    }

//             DecoderStatus() : status( SUCCESS ){}
//             explicit DecoderStatus( const DecoderStatus &other ){ status = other.status; }
//             explicit DecoderStatus( const Status &newstatus ){ status = newstatus; }
//             ~DecoderStatus();

            //Operators
//             inline DecoderStatus& operator=( const DecoderStatus &other ){ if( this != &other ){ status = other.status; } return *this; }
//             inline DecoderStatus& operator=( const Status &newstatus ){ status = newstatus; return *this; }
//             inline bool operator==( const DecoderStatus &other ){ return status == other.status; }
//             inline bool operator==( const Status &other ){ return status == other; }
//             inline bool operator!=( const DecoderStatus &other ){ return !( status == other.status ); }
//             inline bool operator!=( const Status &other ){ return !( status == other ); }
//             inline DecoderStatus& operator|=( const DecoderStatus &other ){ status = static_cast<Status>( status | other.status ); return *this; }
//             inline DecoderStatus& operator|=( const Status &other ){ status = static_cast<Status>( status | other ); return *this; }
//             inline DecoderStatus& operator|( const DecoderStatus &other ){ DecoderStatus ret( *this ); return ( ret |= other ); }
//             inline DecoderStatus& operator|( const Status &other ){ DecoderStatus ret( *this ); ret.status = static_cast<Status>( ret.status | other ); return ret; }
// 
//         private:
//             Status status;
//     };

//     typedef bitwise_enum<Status> DecoderStatus;

    /**
     * @class DecodingOption ydecoder.h
     *
     * @brief Provides a type-safe enum wrapper for the decoding option.
     *
     * @author Lawrence Lee <valheru.ashen.shugar@gmail.com>
     */
//     class DecodingOption{
//         public:
    namespace DecodingOption{
            enum Option{
                STRICT = 0, /**< Strict decoding. This will enforce decoding in a manner compliant with the 1.2 specifications */
                FORCE /**< This will force the decoder to decode the files as best it can, even though it may not comply with the 1.2 specifications */
            };
    }

//             DecodingOption() : option( STRICT ){}
//             explicit DecodingOption( const DecodingOption &other ){ option = other.option; }
//             explicit DecodingOption( const Option &newoption ){ option = newoption; }
//             ~DecodingOption();
// 
//             //Operators
//             DecodingOption& operator=( const DecodingOption &other ){ if( this != &other ){ option = other.option; } return *this; }
//             DecodingOption& operator=( const Option &newoption ){ option = newoption; return *this; }
//             bool operator==( const DecodingOption &other ){ return option == other.option; }
//             bool operator==( const Option &other ){ return option == other; }
//             bool operator!=( const DecodingOption &other ){ return !( option == other.option ); }
//             bool operator!=( const Option &other ){ return !( option == other ); }
//             DecodingOption& operator|=( const DecodingOption &other ){}
//             DecodingOption& operator|=( const Option &other ){}
//             DecodingOption& operator|( const DecodingOption &other ){ DecodingOption ret( *this ); return ( ret |= other ); }
//             DecodingOption& operator|( const Option &other ){ DecodingOption ret( *this ); ret.option |= other; return ret; }

//         private:
//             Option option;
//     };

//     typedef bitwise_enum<Option> DecodingOption;

    /**
     * @class YDecoder ydecoder.h
     *
     * @brief The YDecoder class provides decoding of yencoded files
     *
     * This class provides a yenc 1.2 compliant decoder. It uses libsigc++
     * in order to provide an easy to use interface to which you can connect
     * in order to recieve various message from the library.
     *
     * In the simplest case, you can initialize an instance of the class
     * and then sequentially call the decode() function on the multipart files
     * you wish to decode, and then call write() once the files have been processed
     * to write the decoded files to disk. If you want to decode different files
     * that each consist of one or more multipart files, then you must call the
     * initialize() function inbetween files. A standard use of the decoder would
     * look like this, assuming your wanted to connect to all signals:
     *
     * @code
     * #include <iostream>
     * #include "ydecoder.h"
     *
     * using namespace ydecoder;
     *
     * void print( string message )
     * {
     *     cout << message << endl;
     * }
     *
     * int main( int argc, char *argv[] )
     * {
     *     if( argc < 2 )
     *         return EXIT_FAILURE;
     *
     *     YDecoder decoder;
     *     decoder.message.connect( sigc::ptr_fun( print ) );
     *     decoder.warning.connect( sigc::ptr_fun( print ) );
     *     decoder.error.connect( sigc::ptr_fun( print ) );
     *     decoder.debug.connect( sigc::ptr_fun( print ) );
     *
     *     for( int i = 1; i < argc; i++ ){
     *         decoder.decode( argv[i] );
     *     }
     *
     *     if( !decoder.write( get_current_dir_name() ) ){
     *         print( "Writing failed!" );
     *         return EXIT_FAILURE;
     *     }
     *
     *     return EXIT_SUCCESS;
     * }
     * @endcode
     *
     * Since the class inherits from sigc++::trackable, any signals that you connect
     * to this class will automaticall be disconnected when the class is deleted.
     *
     * @author Lawrence Lee <valheru.ashen.shugar@gmail.com>
     *
     * @see YEncoder
     */
    class YDecoder : public trackable
    {
        public:
            //Functions
            YDecoder();
            ~YDecoder();

            /**
             * Initializes the values of the header and trailer variables. Call this function before decoding a file with a different filename
             * than the previous file. Do not call this function in between decoding parts of a multipart file.
             */
            void initialize();

            /**
             * Decode a yencoded file. This function will keep track of the decoded data so that you can decode a multipart file by
             * simply calling this function repeatedly for the different parts.
             *
             * @param input
             *      The yencoded file to decode.
             *
             * @param decoding
             *      If set to STRICT then the decoder will decode multiparts compliant with the 1.2 version of the yenc specifications;
             *      ie. if it encounters any CRC mismatches or missing parts it will abort.
             *      If this is set to FORCE then the decoder will ignore any CRC mismatch errors it encounters, as well as writing 0's
             *      for the duration of any missing files.
             *
             * @return
             *      The status of the decoder after the decoding operation is finished. This is a value specified in ydecoder::Status
             */
            DecoderStatus::Status decode( const string &input, const DecodingOption::Option &decoding = DecodingOption::STRICT );

            /**
             * Helper function the does the same as the above funtion, except it accepts a list of files. This allows you to decode
             * a list of multipart files in one call, for example. You can pass more than one file to the function, it will sort the
             * list into distinct files and arrange them in memory.
             *
             * @param input
             *
             * @param decoding
             *
             * @return
             */
            DecoderStatus::Status decode( const vector<string>& input, const DecodingOption::Option &decoding = DecodingOption::STRICT );

            /**
             * Write the decoded data to a file. This function should only be called once all the neccessary files have been decoded.
             *
             * @param path The path to save the data to. The filename obtained by the decoder from the yencoded file(s) will be appended
             * to this.
             *
             * @return \b true if the write succeeded, \b false if it failed.
             */
            bool write( const char *path );

            //Signals
            /**
             * Signal you can connect to to track the progress of the decoder
             */
            signal<void, string> progress;

            /**
             * Signal you can connect to to recieve messages from the decoder
             */
            signal<void, string> message;

            /**
             * Signal you can connect to to recieve warnings from the decoder
             */
            signal<void, string> warning;

            /**
             * Signal you can connect to to recieve errors from the decoder
             */
            signal<void, string> error;

            /**
             * Signal you can connect to to recieve debug information from the decoder
             */
            signal<void, string> debug;

        private:
            //Variables
            const unsigned char escaped, magic;
            string read_buffer;
            stringstream data;
            int crc, pcrc;
            crc_32_type crc_val, pcrc_val;
            int line;
            char* name;
            int part;
            int part_size, size;
            int total_parts;

            //Functions
            const char* getAttribute( const char *attr );
            char* getName();
            DecoderStatus::Status parseHeader( filesystem::ifstream *in, const DecodingOption::Option &decoding = DecodingOption::STRICT );
            DecoderStatus::Status parseTrailer( const stringstream &write_stream, const DecodingOption::Option &decoding = DecodingOption::STRICT );
    };
}

#endif
