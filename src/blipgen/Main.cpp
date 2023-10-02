#include <blip/blip.h>

#include <chrono>
#include <string>
#include <vector>
#include <span>

//----------------------------------------------------------------------------------------------------------------------
bool WriteWAV( std::string fileName, std::span<const float> samples, size_t sampleRate )
{
	FILE* file = fopen( fileName.c_str(), "wb" );
	if ( file == NULL )
	{
		printf( "Can't open file!\n" );
		return false;
	}

	// WAV header
	// https://en.wikipedia.org/wiki/WAV
	struct
	{
		char riff[4] = { 'R', 'I', 'F', 'F' };
		uint32_t size = 0;
		char wave[4] = { 'W', 'A', 'V', 'E' };
		char fmt[4] = { 'f', 'm', 't', ' ' };
		uint32_t fmtSize = 16;
		uint16_t format = 3; // WAVE_FORMAT_IEEE_FLOAT
		uint16_t channels = 1;
		uint32_t sampleRate = 0;
		uint32_t byteRate = 0;
		uint16_t blockAlign = sizeof( float );
		uint16_t bitsPerSample = 8 * sizeof( float );
		char data[4] = { 'd', 'a', 't', 'a' };
		uint32_t dataSize = 0;
	} header;

	header.size = sizeof( header ) - 8;
	header.dataSize = ( uint32_t )( samples.size() * sizeof( float ) );
	header.sampleRate = ( uint32_t )sampleRate;
	header.byteRate = ( uint32_t )sampleRate * sizeof( float );

	fwrite( &header, sizeof( header ), 1, file );
	fwrite( samples.data(), sizeof( float ), samples.size(), file );
	fclose( file );

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
void PrintHelp()
{
	printf( "Usage: blip [options] <MML string>\n\n" );
	printf( "Options:\n" );
	printf( "  -i <input file>     Input file name\n" );
	printf( "  -o <output file>    Output file name\n" );
	printf( "  -s <sample rate>    Sample rate (default: 48000)\n" );

	// TODO:
	// printf( "  -v <volume>         Global volume (default: 127)\n" );
	// printf( "  -ss <milliseconds>  Silence at the beginning (default: 0)\n" );
	// printf( "  -se <milliseconds>  Silence at the end (default: 0)\n" );
	// printf( "  -fi <milliseconds>  Fade in time (default: 0)\n" );
	// printf( "  -fo <milliseconds>  Fade out time (default: 0)\n" );
	// printf( "  -solo <channel>     Solo channel (default: 0 = no solo)\n" );
}

//----------------------------------------------------------------------------------------------------------------------
int main( int argc, char** argv )
{
	uint32_t sampleRate = 48000; // Sample rate
	uint32_t volume = 127;       // Default volume
	uint32_t silenceStartMS = 0; // Extra silence at the beginning of the song (in milliseconds)
	uint32_t silenceEndMS = 0;   // Extra silence at the end of the song (in milliseconds)
	uint32_t fadeInMS = 0;       // Fade in time (in milliseconds)
	uint32_t fadeOutMS = 0;      // Fade out time (in milliseconds)
	uint32_t soloChannel = 0;    // Solo channel (0 = no solo, 1 = first channel, 2 = second, ...)
	std::string outputFileName;  // Output file name
	std::string inputFileName;   // Input file name
	std::string mmlString;       // MML string

	// Parse command line arguments
	int argIndex = 1;
	while ( argIndex < argc )
	{
		std::string arg = argv[argIndex++];
		if ( arg == "-i" )
		{
			if ( argIndex >= argc )
			{
				printf( "Missing input file name!\n" );
				return -1;
			}

			inputFileName = argv[argIndex++];
		}
		else if ( arg == "-o" )
		{
			if ( argIndex >= argc )
			{
				printf( "Missing output file name!\n" );
				return -1;
			}

			outputFileName = argv[argIndex++];
		}
		else if ( arg == "-s" )
		{
			if ( argIndex >= argc )
			{
				printf( "Missing sample rate!\n" );
				return -1;
			}

			sampleRate = atoi( argv[argIndex++] );
		}
		else if ( arg == "-v" )
		{
			if ( argIndex >= argc )
			{
				printf( "Missing volume!\n" );
				return -1;
			}

			volume = atoi( argv[argIndex++] );
		}
		else if ( arg == "-ss" )
		{
			if ( argIndex >= argc )
			{
				printf( "Missing silence at the beginning!\n" );
				return -1;
			}

			silenceStartMS = atoi( argv[argIndex++] );
		}
		else if ( arg == "-se" )
		{
			if ( argIndex >= argc )
			{
				printf( "Missing silence at the end!\n" );
				return -1;
			}

			silenceEndMS = atoi( argv[argIndex++] );
		}
		else if ( arg == "-fi" )
		{
			if ( argIndex >= argc )
			{
				printf( "Missing fade in time!\n" );
				return -1;
			}

			fadeInMS = atoi( argv[argIndex++] );
		}
		else if ( arg == "-fo" )
		{
			if ( argIndex >= argc )
			{
				printf( "Missing fade out time!\n" );
				return -1;
			}

			fadeOutMS = atoi( argv[argIndex++] );
		}
		else if ( arg == "-solo" )
		{
			if ( argIndex >= argc )
			{
				printf( "Missing solo channel!\n" );
				return -1;
			}

			soloChannel = atoi( argv[argIndex++] );
		}
		else if ( arg == "-h" || arg == "--help" )
		{
			PrintHelp();
			return 0;
		}
		else
		{
			mmlString = arg;
		}
	}

	if ( mmlString.empty() && inputFileName.empty() )
	{
		printf( "Missing MML string or input file! Use -h for more info.\n" );
		return -1;
	}

	if ( !inputFileName.empty() )
	{
		FILE* file = fopen( inputFileName.c_str(), "rb" );
		if ( file == NULL )
		{
			printf( "Can't open file!\n" );
			return -1;
		}

		fseek( file, 0, SEEK_END );
		size_t fileSize = ftell( file );
		fseek( file, 0, SEEK_SET );

		std::string fileString;
		fileString.resize( fileSize );
		fread( fileString.data(), 1, fileSize, file );
		fclose( file );

		mmlString += fileString;
	}

	blip_song_t song;
	blip_init_song( &song, mmlString.c_str(), sampleRate );

	std::vector<float> samples;

	size_t numSamples = 0;
	size_t length = 0;
	size_t numChannels = 0;

	std::chrono::microseconds duration( 0 );

	while ( true )
	{
		size_t remainingSamples = samples.size() - numSamples;
		if ( remainingSamples == 0 )
		{
			auto newSize = ( samples.size() * 3 ) / 2 + sampleRate;
			remainingSamples = newSize - samples.size();
			samples.resize( newSize );
		}

		auto start = std::chrono::high_resolution_clock::now();
		auto numGenerated = blip_mix_samples_f32( &song, samples.data() + numSamples, remainingSamples );
		auto end = std::chrono::high_resolution_clock::now();
		duration += std::chrono::duration_cast<std::chrono::microseconds>( end - start );

		if ( numGenerated < 0 )
		{
			printf( "Error after: %d\n", ( int )( song.notes - mmlString.c_str() ) );
			return -1;
		}

		numSamples += numGenerated;

		if ( numGenerated == 0 && remainingSamples > 0 )
		{
			if ( numSamples > length )
				length = numSamples;

			++numChannels;
			if ( song.notes[0] == '\0' )
				break;

			// Skip the track separator and start over
			++song.notes;
			numSamples = 0;
			continue;
		}

		remainingSamples -= ( size_t )numGenerated;
	}

	samples.resize( length );

	auto durationSeconds = ( int )( length / sampleRate );
	printf( "Duration: %d:%d min\n", durationSeconds / 60, durationSeconds % 60 );
	printf( "Channels: %d\n", ( int )numChannels );
	printf( "Rendered: %.3f ms\n", duration.count() / 1000.0f );

	if ( outputFileName.empty() )
	{
		if ( inputFileName.empty() )
			outputFileName = "blip.wav";
		else
		{
			outputFileName = inputFileName;
			size_t dotIndex = outputFileName.find_last_of( '.' );
			if ( dotIndex != std::string::npos )
				outputFileName.resize( dotIndex );

			outputFileName += ".wav";
		}
	}

	WriteWAV( outputFileName.c_str(), samples, sampleRate );

	return 0;
}
