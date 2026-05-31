#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/dict.h>
#include <stdatomic.h>
#include <unistd.h>

#define BUFFER_SIZE (8192*1024)

typedef struct sx
{
	pthread_t thr[2];
	void (*title_callback)(char*);
	char *url;
} stream_info;

stream_info *sz;unsigned char *bb,*_Atomic be,*br;

#if __APPLE__
#include <AudioToolbox/AudioQueue.h>

AudioQueueRef qu;

// Audio Callback;
static void queue_audio(void *i,AudioQueueRef a,AudioQueueBufferRef b)
{
	// Calculate Available;
	unsigned char *x=atomic_load(&be);unsigned int n=x-br;

	if(br>x)
	{		
		unsigned int d=(bb+BUFFER_SIZE)-br;

		if(n>d){n=d;};
	};

	if(n<8192)
	{
		b->mAudioDataByteSize=1024;memset(b->mAudioData,0,1024);AudioQueueEnqueueBuffer(a,b,0,0);

		return;
	};
	
	// Copy Interleaved;
	memcpy(b->mAudioData,br,n);br+=n;

	b->mAudioDataByteSize=n;AudioQueueEnqueueBuffer(a,b,0,0);

	// Buffer Loop;
	if(br>=bb+BUFFER_SIZE)
	{
		br=bb;
	};
};

// Playback Setup;
static void *play(void *j)
{
	// Get Device;
	AudioStreamBasicDescription f={(double)(intptr_t)j,kAudioFormatLinearPCM,kAudioFormatFlagIsFloat|kAudioFormatFlagIsPacked};

	f.mChannelsPerFrame=2;f.mBitsPerChannel=32;f.mFramesPerPacket=1;f.mBytesPerPacket=f.mBytesPerFrame=(f.mBitsPerChannel/8)*f.mChannelsPerFrame;

	// Create Queue & Buffers;
	AudioQueueNewOutput(&f,queue_audio,(void*)0,0,0,0,&qu);

	int i=0;AudioQueueBufferRef b[3];

	while(i<3)
	{
		AudioQueueAllocateBuffer(qu,BUFFER_SIZE,&b[i]);queue_audio(0,qu,*(b+i));

		i+=1;
	};

	// Start (Non-Blocking);
	AudioQueueStart(qu,0);

	return 0;
};
#elif __linux__
#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>

struct pw_stream *pw;struct pw_main_loop *pl;

// Pipewire Callback;
static void queue_audio(void *u) 
{
	if(be-bb<4096){return;};

	struct pw_buffer *b=pw_stream_dequeue_buffer(pw);

	if(!b){return;};

	// Check Buffers;
	struct spa_buffer *buf=b->buffer;

	if(buf->n_datas<2) 
	{
		pw_stream_queue_buffer(pw,b);return;
	};

	struct spa_data l=buf->datas[0],r=buf->datas[1];

	if(!l.data||!r.data)
	{
		pw_stream_queue_buffer(pw,b);return;
	};

	// Buffer Loop;
	unsigned int o=BUFFER_SIZE/2,n=b->requested*sizeof(float);

	if(br+n>bb+o)
	{
		br=bb;
	};

	// Write Data;
	memcpy(l.data,br,n);memcpy(r.data,br+o,n);br+=n;

	l.chunk->offset=r.chunk->offset=0;l.chunk->size=r.chunk->size=n;
	l.chunk->stride=r.chunk->stride=sizeof(float);

	// Sumbit;
	pw_stream_queue_buffer(pw,b);
};

// Destroy Playback;
static void free_play(void *j)
{
	if(pw){pw_stream_destroy(pw);};
	if(pl){pw_main_loop_destroy(pl);};

	pw_deinit();
};

// Playback Setup;
static void *play(void *j)
{
	pthread_cleanup_push(free_play,(void*)0);

	// Define Variables;
	const struct spa_pod *o[1];unsigned char b[1024];struct spa_pod_builder p=SPA_POD_BUILDER_INIT(b,sizeof(b));

	pw_init(0,0);pl=pw_main_loop_new(0);

	// Create Playback Stream;
	static const struct pw_stream_events s={PW_VERSION_STREAM_EVENTS,.process=queue_audio};

	pw=pw_stream_new_simple(pw_main_loop_get_loop(pl),"Radio",0,&s,0);

	// Add Format & Connect;
	o[0]=spa_format_audio_raw_build(&p,SPA_PARAM_EnumFormat,&(struct spa_audio_info_raw){SPA_AUDIO_FORMAT_F32P,0,(unsigned int)(long)j,2,{SPA_AUDIO_CHANNEL_FL,SPA_AUDIO_CHANNEL_FR}});

	pw_stream_connect(pw,PW_DIRECTION_OUTPUT,PW_ID_ANY,PW_STREAM_FLAG_AUTOCONNECT|PW_STREAM_FLAG_MAP_BUFFERS|PW_STREAM_FLAG_RT_PROCESS,o,1);

	// Start (Blocking);
	pw_main_loop_run(pl);

	pthread_cleanup_pop(1);
};

// free play
#endif

// Stream;
AVFormatContext *aa;AVPacket *ad;AVFrame *af;AVCodecContext *ax;

static inline int destroy()
{
	if(ax!=0){avcodec_free_context(&ax);};
	if(aa!=0){avformat_close_input(&aa);};

	be=br=bb;*sz->thr=0;
	
	if(*(sz->thr+1)!=0)
	{
		#if __linux__
		pthread_cancel(*(sz->thr+1));pthread_join(*(sz->thr+1),0);
		#else
		AudioQueueStop(qu,1);AudioQueueDispose(qu,1);
		#endif

		*(sz->thr+1)=0;
	};

	av_frame_free(&af);av_packet_free(&ad);free(bb);

	return 0;
};

static void *stream(void *j)
{
	sz=j;bb=calloc(1,BUFFER_SIZE);

	if(bb==0)
	{
		return 0;
	};

	ad=av_packet_alloc();af=av_frame_alloc();

	if(ad==0||af==0)
	{
		if(af!=0){av_frame_free(&af);};
		if(ad!=0){av_packet_free(&ad);};

		return 0;
	};

	be=br=bb;

	// Open Stream;
	if(avformat_open_input(&aa,sz->url,0,0)<0)
	{
		goto destroy_stream;
	};

	if(avformat_find_stream_info(aa,0)<0)
	{
		goto destroy_stream;
	};

	// Get Stream (0);
	int i=0,t=aa->nb_streams;

	while(i<t&&aa->streams[i]->codecpar->codec_type!=AVMEDIA_TYPE_AUDIO)
	{
		i+=1;
	};

	AVCodecParameters *a=aa->streams[i]->codecpar;

	if(a->codec_type!=AVMEDIA_TYPE_AUDIO) 
	{
		goto destroy_stream;
	};

	// Get Decoder;
	const AVCodec *c=avcodec_find_decoder(a->codec_id);ax=avcodec_alloc_context3(c);

	avcodec_parameters_to_context(ax,a);

	if(avcodec_open2(ax,c,0)<0||a->sample_rate<1000||ax->ch_layout.nb_channels!=2)
	{
		goto destroy_stream;
	};

	int s=av_get_bytes_per_sample(ax->sample_fmt),o=BUFFER_SIZE/2;
	
	// Create Playback Thread (Once);
	if(*(sz->thr+1)==0)
	{
		pthread_create(&*(sz->thr+1),0,play,(void*)(intptr_t)a->sample_rate);
	};

	// Decoding Loop;
	while(av_read_frame(aa,ad)>=0)
	{
		if(ad->stream_index==i)
		{
			// Song Title;
			if(aa->event_flags&AVFMT_EVENT_FLAG_METADATA_UPDATED) 
			{
				AVDictionaryEntry *t=0;t=av_dict_get(aa->metadata,"StreamTitle",0,0);
				
				if(!t) 
				{
					t=av_dict_get(aa->metadata,"title",0,0);
				};

				if(t)
				{
					sz->title_callback(t->value);
				};
				
				// Seen;
				aa->event_flags&=~AVFMT_EVENT_FLAG_METADATA_UPDATED;
			};

			// Packet >> Decoder;
			if(avcodec_send_packet(ax,ad)<0)
			{
				goto destroy_stream;
			};

			int r=1,i,c;

			while(r>=0)
			{
				r=avcodec_receive_frame(ax,af);i=0;

				if(r==AVERROR(EAGAIN)||r==AVERROR_EOF) 
				{
					break;
				} 
				else if(r<0)
				{
					goto destroy_stream;
				};

				int x=af->nb_samples*s;

				#if __linux__
				// Buffer Loop (Prevent Overflow);
				if((be-bb)+x>o)
				{
					atomic_store(&be,bb);
				};

				// Planar Copy;
				memcpy(be,af->data[0],x);memcpy(be+o,af->data[1],x);atomic_fetch_add(&be,x);
				#else
				// Buffer Loop (Prevent Overflow);
				if((be-bb)+(x*2)>BUFFER_SIZE)
				{
					atomic_store(&be,bb);
				};

				// Interleaved Copy;
				float *e=(float*)atomic_load(&be);int n=x/sizeof(float),i=0;float *l=(float*)af->data[0],*r=(float*)af->data[1];

				while(i<n)
				{
					*e=l[i];e+=1;*e=r[i];e+=1;i+=1;
				};

				atomic_store(&be,(unsigned char*)e);
				#endif
			};
		};

		av_packet_unref(ad);
	};

	destroy_stream:
	destroy();
	
	return 0;
};