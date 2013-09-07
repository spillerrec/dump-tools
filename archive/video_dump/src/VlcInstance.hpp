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

#ifndef VLC_INSTANCE
#define VLC_INSTANCE

#include <QString>
#include <vlc/vlc.h>

class VlcInstance{
	protected:
		libvlc_instance_t *inst;
		
	public:
		VlcInstance( int argc, const char* const *argv ){ inst = libvlc_new( argc, argv ); }
		VlcInstance( const VlcInstance& org ){
			inst = org;
			libvlc_retain( inst );
		}
		~VlcInstance(){ libvlc_release( inst ); }
		
		void set_user_agent( QString name, QString http ){ libvlc_set_user_agent( inst, name.toLocal8Bit().constData(), http.toLocal8Bit().constData() ); }
		//video_filter_list_get
		//set_exiit_handler
		//TODO: rest
		
		operator libvlc_instance_t*() const{ return inst; }
};

#endif

