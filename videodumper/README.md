## videodumper

Produces dump files from every frame in a video sequence

### Usage
*<X>* required argument *X*
*[Y]* optional argument *Y*

Extract every frame from *m* minutes and *s* seconds

*videodumper <filename> [mm:ss]*

Extract every frame from *X* % based on file size (helps if seeking fails when using time)

*videodumper <filename> XX[.XX]%

Output files are placed in the directory "out/" which must exists. It continues until the end of the video file, so use Ctrl+C to terminate the application in your terminal. Output is uncompressed for higher dumping speed.

### Building

*Dependencies*

- C++11
- Qt5
- qmake
- ffmpeg

*Building*

1. qmake
2. make