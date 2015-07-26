#ifndef DUMP_CREATOR_HPP
#define DUMP_CREATOR_HPP

#include <kio/thumbcreator.h>

class DumpCreator : public ThumbCreator{
	public:
		DumpCreator();
		virtual ~DumpCreator();
		virtual bool create( const QString& path, int width, int height, QImage& img );
		virtual Flags flags() const;
};

#endif
