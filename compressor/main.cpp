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
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QtConcurrent>

#include "DumpPlane.hpp"
using namespace std;

const QString EXT = QStringLiteral("dump");

QString postfix( DumpPlane::Compression comp ){
	switch( comp ){
		case DumpPlane::LZIP: return QStringLiteral(".lzip");
		case DumpPlane::LZMA: return QStringLiteral(".lzma");
		default: return {};
	};
}

QString convert( QString filepath ){
	QFileInfo info( filepath );
	if( info.suffix() != EXT )
		return filepath;
	
	QFile f( filepath );
	QString new_name = info.dir().absolutePath() + QStringLiteral("/") + info.baseName() + postfix( DumpPlane::LZMA ) + QStringLiteral(".") + EXT;
	QString temp_name = new_name + QStringLiteral(".temp");
	if( f.open( QIODevice::ReadOnly ) ){
		QFile copy( temp_name );
		if( copy.open( QIODevice::WriteOnly ) ){
			while( true ){
				DumpPlane p;
				if( !p.read( f ) )
					break;
				p.write( copy );
			}
			
			copy.close();
		}
		f.close();
		
		if( QFileInfo( temp_name ).size() > 0 ){
			QFile::remove( filepath );
			QFile::rename( temp_name, new_name );
		}
	}
	return filepath;
}

int main( int argc, char* argv[] ){
	QCoreApplication app( argc, argv );
	
	QStringList args = app.arguments();
	QStringList files;
	
	auto name_filters = QStringList() << QStringLiteral("*.") + EXT;
	
	for( int i=1; i<args.count(); ++i ){
		QFileInfo info( args[i] );
		if( info.isDir() ){
			//Add entire folder
			QDir dir( args[i]);
			auto list = dir.entryInfoList( name_filters );
			for( auto file : list )
				files << file.filePath();
		}
		else if( info.suffix() == EXT )
			files << args[i];
	}
	
	
	auto future = QtConcurrent::mapped( files, convert );
	for( int i=0; i<files.count(); ++i )
		cout << "[" << i << "/" << files.count()-1 << "] Processed: " << future.resultAt(i).toLocal8Bit().constData() << "\n";
	
	return 0;
}
