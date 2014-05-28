/*	This file is part of dump-tools.

	dump-tools is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	dump-tools is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with dump-tools.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QCoreApplication>
#include <QStringList>
#include <QFile>
#include <QFileInfo>

#include "DumpPlane.hpp"
using namespace std;

QString postfix( Plane::Compression comp ){
	switch( comp ){
		case Plane::LZIP: return ".lzip";
		case Plane::LZMA: return ".lzma";
		default: return "";
	};
}

int main( int argc, char* argv[] ){
	QCoreApplication app( argc, argv );
	
	QStringList args = app.arguments();
	for( int i=1; i<args.count(); ++i ){
		QFileInfo info( args[i] );
		if( info.suffix() != "dump" )
			continue;
		
		cout << "[" << i << "/" << args.count()-1 << "] Processing: " << args[i].toLocal8Bit().constData() << "\n";
		QFile f( args[i] );
		QString new_name = info.baseName() + postfix( Plane::LZMA ) + ".dump";
		QString temp_name = new_name + ".temp";
		if( f.open( QIODevice::ReadOnly ) ){
			QFile copy( temp_name );
			if( copy.open( QIODevice::WriteOnly ) ){
				while( true ){
					Plane p;
					if( !p.read( f ) )
						break;
					p.write( copy );
				}
				
				copy.close();
			}
			f.close();
			
			if( QFileInfo( temp_name ).size() > 0 ){
				QFile::remove( args[i] );
				QFile::rename( temp_name, new_name );
			}
		}
		
	}
	
	return 0;
}