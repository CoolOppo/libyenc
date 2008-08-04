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
 * @namespace YEncoder yencoder
 */
#ifndef YENCODER_YENCODER_H
#define YENCODER_YENCODER_H

namespace yencoder {

    /**
     * @class YEncoder yencoder.h
     *
     * @brief Provides an encoder for encoding files according to the yenc 1.2 specifications.
     *
     * This class provides an yenc encoder library that can encode files
     * compliant with the yenc 1.2 specifications. It provides an easy to use
     * interface that makes use of libsigc++ so that you can connect your
     * program to the signals emitted by the library.
     *
     * @author Lawrence Lee <valheru.ashen.shugar@gmail.com>
     */
    class YEncoder{
        public:
            YEncoder();
            ~YEncoder();
    };

}

#endif
