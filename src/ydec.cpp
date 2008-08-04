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

#include <sigc++/sigc++.h>
#include "ydecoder.h"

using namespace ydecoder;

void dump( string message )
{
    cout << message << endl;
}

int main( int argc, char *argv[] )
{
    if( argc < 2 )
        return EXIT_FAILURE;

    YDecoder decoder;
    decoder.message.connect( sigc::ptr_fun( dump ) );
    decoder.warning.connect( sigc::ptr_fun( dump ) );
    decoder.error.connect( sigc::ptr_fun( dump ) );
    decoder.debug.connect( sigc::ptr_fun( dump ) );

    for( int i = 1; i < argc; i++ ){
        decoder.decode( argv[i] );
}

    if( !decoder.write( get_current_dir_name() ) ){
        dump( "Writing failed!" );
        return EXIT_FAILURE;
}

    return EXIT_SUCCESS;
}
