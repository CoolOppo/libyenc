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
#ifndef YDECODER_H
#define YDECODER_H

#include <boost/crc.hpp>
#include <boost/filesystem/fstream.hpp>
#include <sigc++/sigc++.h>
#include <string>
#include <sstream>
#include <iostream>

using namespace boost;
using namespace boost::filesystem;
using namespace sigc;
using namespace std;

namespace ydecoder{

    /**
     * @class YDecoder ydecoder.h
     *
     * @brief The YDecoder class provides decoding of yencoded files
     *
     * @author Lawrence Lee <valheru.ashen.shugar@gmail.com>
     *
     * @see YEncoder
     */
    class YDecoder : public trackable
    {
        public:
            //Enums
            /**
             * Enum providing return codes for the decoder.
             */
            enum Status{
                SUCCESS = 0, /**< The decoding succeeded */
                CRC_MISMATCH, /**< The crc value of the decoded data doesn't match the crc value in the trailer */
                PART_CRC_MISMATCH, /**< The crc value of the decoded part data doesn't match the part crc value in the trailer */
                PART_MISMATCH, /**< The part number in the trailer doesn't match the part number in the header */
                SIZE_MISMATCH, /**< The size of the data or the size value in the trailer doesn't match the size value in the header */
                NAME_MISMATCH, /**< The name value in the header doesn't match the name of the previous parts */
                FAILED /**< The decoding failed */
            };

            //Functions

            YDecoder();
            ~YDecoder();

            /**
             * Resets the values of the header and trailer variables. Call this function before decoding a file with a different filename
             * than the previous file. Do not call this function in between decoding parts of a multipart file.
             */
            void reinitialize();

            /**
             * Decode a yencoded file. This function will keep track of the decoded data so that you can decode a multipart file by
             * simply calling this function repeatedly for the different parts.
             * @param input The yencoded file to decode.
             * @return The status of the decoder after the decoding operation is finished. This is a value specified in YDecoder::Status
             */
            Status decode( const char *input, bool forcedecoding = false );

            /**
             * Write the decoded data to a file. This function should only be called once all the neccessary files have been decoded.
             * @param path The path to save the data to. The filename obtained by the decoder from the yencoded file(s) will be appended
             * to this.
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
            Status parseHeader( filesystem::ifstream *in );
            Status parseTrailer( const stringstream &write_stream );
    };
}

#endif
