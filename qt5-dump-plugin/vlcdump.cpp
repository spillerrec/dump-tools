#include <QImage>
#include <QByteArray>
#include <QImageIOHandler>
#include <QImageIOPlugin>

#include "Dump.hpp"
#include "vlcdump.hpp"

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(pnp_dumpplugin, vlcdump)
#endif

const QByteArray EXT = "dump";

class ric_handler: public QImageIOHandler{
	private:
		bool image_read;
	public:
		ric_handler( QIODevice *device ) : image_read( false ){
			setDevice( device );
			setFormat( EXT );
		}
		
		bool canRead() const;
		bool read( QImage *image );
};

bool ric_handler::canRead() const{
	return true;
	if( format() == EXT )
		return true;
	else
		return false;
}


bool ric_handler::read( QImage *img_pointer ){
	if( image_read )
		return false;
	
	*img_pointer = Dump( device() ).to_qimage();
	
	return image_read=true;
}


QStringList vlcdump::keys() const{
	return QStringList() << QString::fromLatin1(EXT);
}

QImageIOPlugin::Capabilities vlcdump::capabilities( QIODevice *device, const QByteArray &format ) const{
	if( format == EXT )
		return Capabilities( CanRead );
	else
		return 0;
}

QImageIOHandler* vlcdump::create( QIODevice *device, const QByteArray &format ) const{
	return (QImageIOHandler*) new ric_handler( device );
}


