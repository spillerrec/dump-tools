#include "VlcInstance.hpp"
#include "GuiPlayer.hpp"
#include <QApplication>

int main( int argc, char* argv[] ){
	QApplication app( argc, argv );
	
	VlcInstance inst( 0, NULL );
	GuiPlayer p( inst );
	p.show();
	
	return app.exec();
}

