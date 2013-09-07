#include <QWidget>
#include <QMutex>

struct libvlc_media_player_t;

struct Plane;

class ARender : public QWidget{
	Q_OBJECT
	
	public:
		virtual void lock( void **p_pixels ) = 0;
		virtual void unlock( void *id, void *const *p_pixels ) = 0;
		virtual void display( void *id ) = 0;
		virtual void format( char *chroma, unsigned &width, unsigned &height, unsigned *pitches, unsigned *lines ) = 0;
		virtual void cleanup() = 0;
		virtual void set_recording( bool state ) = 0;
		
	public:
		ARender( QWidget *parent = NULL ) : QWidget( parent ){ }
		void set_calbacks( libvlc_media_player_t *mp );
};

class Render : public ARender{
	Q_OBJECT
	
	private:
		QMutex surface_lock;
		Plane *data_planes;
		
		bool save;
		
		static const unsigned PLANES_AMOUNT = 3;
		
	protected:
		void paintEvent( QPaintEvent *event );
		
	public:
		void lock( void **p_pixels );
		void unlock( void *id, void *const *p_pixels );
		void display( void *id );
		void format( char *chroma, unsigned &width, unsigned &height, unsigned *pitches, unsigned *lines );
		void cleanup();
		void set_recording( bool state ){ save = state; }
		
	public:
		Render( QWidget *parent = NULL );
		~Render();
		
};