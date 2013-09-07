#include <QImage>
#include <QByteArray>
#include <QImageIOHandler>
#include <QImageIOPlugin>

#include "Dump.hpp"
#include "vlcdump.hpp"

class ric_handler: public QImageIOHandler{
	private:
		bool image_read;
	public:
		ric_handler( QIODevice *device ) : image_read( false ){
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
	if( image_read )
		return false;
	
	QByteArray data = device()->readAll();
	Dump image( data.data(), data.size() );
	*img_pointer = image.to_qimage();
	
	return image_read=true;
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


