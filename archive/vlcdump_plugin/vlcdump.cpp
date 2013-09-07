#include <QImage>
#include <QByteArray>
#include <QImageIOHandler>
#include <QImageIOPlugin>

#include "ImageEx.hpp"
#include "vlcdump.h"

class ric_handler: public QImageIOHandler{
	public:
		ric_handler( QIODevice *device ){
			setDevice( device );
			setFormat( "dump" );
		}
		
		bool canRead() const;
		bool read( QImage *image );
};

bool ric_handler::canRead() const{
	return true;
	if( format() == "dump" )
		return true;
	else
		return false;
}


bool ric_handler::read( QImage *img_pointer ){
	//QByteArray data = device()->readAll();
	ImageEx img_raw;// data.data(), data.size() );
	img_raw.from_dump( *device() );
	*img_pointer = img_raw.to_qimage();
	
	return true;
}


QStringList vlcdump::keys() const{
	return QStringList() << "dump";
}

QImageIOPlugin::Capabilities vlcdump::capabilities( QIODevice *device, const QByteArray &format ) const{
	if( format == "dump" )
		return Capabilities( CanRead );
	else
		return 0;
}

QImageIOHandler* vlcdump::create( QIODevice *device, const QByteArray &format ) const{
	return (QImageIOHandler*) new ric_handler( device );
}


