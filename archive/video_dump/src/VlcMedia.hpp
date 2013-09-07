/*
	This file is part of vlc_dump.

	vlc_dump is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	vlc_dump is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with vlc_dump.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef VLC_MEDIA
#define VLC_MEDIA

#include <QString>
#include <QUrl>
#include <vlc/vlc.h>
#include "VlcInstance.hpp"

class VlcMedia{
	protected:
		libvlc_media_t *media;
		
	public:
		VlcMedia( VlcInstance &inst, QUrl uri ){
			media = libvlc_media_new_path( inst, uri.toEncoded().constData() );
		}
		VlcMedia( VlcInstance &inst, QString path ){
			media = libvlc_media_new_path( inst, QUrl::fromLocalFile( path ).toEncoded().constData() );
		}
	//	VlcMedia( VlcInstance &inst, QUrl url );
		VlcMedia( VlcInstance &inst, int file_descriptor );
		VlcMedia( const VlcMedia &org ){
			media = org;
			libvlc_media_retain( media );
		}
		//VlcMedia( VlcInstance &inst, QString name ); //Can't do this :
		
		~VlcMedia(){ libvlc_media_release( media ); }
		
		operator libvlc_media_t*() const{ return media; }
};

#endif

