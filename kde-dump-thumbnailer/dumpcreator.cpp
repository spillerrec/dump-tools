#include "dumpcreator.hpp"

#include <QImage>
#include <KDebug>

extern "C"{
	KDE_EXPORT ThumbCreator *new_creator(){ return new DumpCreator; }
}

DumpCreator::DumpCreator(){}
DumpCreator::~DumpCreator(){}

bool DumpCreator::create( const QString& path, int, int, QImage& img ){
	img = QImage( path );
	return !img.isNull();
}