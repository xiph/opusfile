/*For fileno()*/
#if !defined(_POSIX_SOURCE)
# define _POSIX_SOURCE 1
#endif
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#if defined(_WIN32)
/*We need the following two to set stdin/stdout to binary.*/
# include <io.h>
# include <fcntl.h>
#endif
#include <opus/opusfile.h>

/*Use shorts, they're smaller.*/
#define OP_FIXED_POINT (1)

#if defined(OP_FIXED_POINT)

typedef opus_int16 op_sample;

# define op_read_native op_read

/*TODO: The convergence after 80 ms of preroll is far from exact.
  Our comparison is very rough.
  Need to find some way to do this better.*/
# define MATCH_TOL (16384)

# define ABS(_x) ((_x)<0?-(_x):(_x))

# define MATCH(_a,_b) (ABS((_a)-(_b))<MATCH_TOL)

/*Don't have fixed-point downmixing code.*/
# undef OP_WRITE_SEEK_SAMPLES

#else

typedef float op_sample;

# define op_read_native op_read_float

/*TODO: The convergence after 80 ms of preroll is far from exact.
  Our comparison is very rough.
  Need to find some way to do this better.*/
# define MATCH_TOL (16384.0/32768)

# define FABS(_x) ((_x)<0?-(_x):(_x))

# define MATCH(_a,_b) (FABS((_a)-(_b))<MATCH_TOL)

# if defined(OP_WRITE_SEEK_SAMPLES)
/*Matrices for downmixing from the supported channel counts to stereo.*/
static const float DOWNMIX_MATRIX[8][8][2]={
  /*mono*/
  {
    {1.F,1.F}
  },
  /*stereo*/
  {
    {1.F,0.F},{0.F,1.F}
  },
  /*3.0*/
  {
    {0.5858F,0.F},{0.4142F,0.4142F},{0,0.5858F}
  },
  /*quadrophonic*/
  {
    {0.4226F,0.F},{0,0.4226F},{0.366F,0.2114F},{0.2114F,0.336F}
  },
  /*5.0*/
  {
    {0.651F,0.F},{0.46F,0.46F},{0,0.651F},{0.5636F,0.3254F},{0.3254F,0.5636F}
  },
  /*5.1*/
  {
    {0.529F,0.F},{0.3741F,0.3741F},{0.F,0.529F},{0.4582F,0.2645F},
    {0.2645F,0.4582F},{0.3741F,0.3741F}
  },
  /*6.1*/
  {
    {0.4553F,0.F},{0.322F,0.322F},{0.F,0.4553F},{0.3943F,0.2277F},
    {0.2277F,0.3943F},{0.2788F,0.2788F},{0.322F,0.322F}
  },
  /*7.1*/
  {
    {0.3886F,0.F},{0.2748F,0.2748F},{0.F,0.3886F},{0.3366F,0.1943F},
    {0.1943F,0.3366F},{0.3366F,0.1943F},{0.1943F,0.3366F},{0.2748F,0.2748F}
  }
};

static void write_samples(float *_samples,int _nsamples,int _nchannels){
  float stereo_pcm[120*48*2];
  int   i;
  for(i=0;i<_nsamples;i++){
    float l;
    float r;
    int   ci;
    l=r=0.F;
    for(ci=0;ci<_nchannels;ci++){
      l+=DOWNMIX_MATRIX[_nchannels-1][ci][0]*_samples[i*_nchannels+ci];
      r+=DOWNMIX_MATRIX[_nchannels-1][ci][1]*_samples[i*_nchannels+ci];
    }
    stereo_pcm[2*i+0]=l;
    stereo_pcm[2*i+1]=r;
  }
  fwrite(stereo_pcm,sizeof(*stereo_pcm)*2,_nsamples,stdout);
}
# endif

#endif

static void verify_seek(OggOpusFile *_of,opus_int64 _byte_offset,
 ogg_int64_t _pcm_offset,ogg_int64_t _pcm_length,op_sample *_bigassbuffer){
  opus_int64  byte_offset;
  ogg_int64_t pcm_offset;
  ogg_int64_t duration;
  op_sample   buffer[120*48*8];
  int         nchannels;
  int         nsamples;
  int         li;
  int         lj;
  int         i;
  byte_offset=op_raw_tell(_of);
  if(_byte_offset!=-1&&byte_offset<_byte_offset){
    fprintf(stderr,"\nRaw position out of tolerance: requested %li, "
     "got %li.\n",(long)_byte_offset,(long)byte_offset);
    exit(EXIT_FAILURE);
  }
  pcm_offset=op_pcm_tell(_of);
  if(_pcm_offset!=-1&&pcm_offset>_pcm_offset){
    fprintf(stderr,"\nPCM position out of tolerance: requested %li, "
     "got %li.\n",(long)_pcm_offset,(long)pcm_offset);
    exit(EXIT_FAILURE);
  }
  if(pcm_offset<0||pcm_offset>_pcm_length){
    fprintf(stderr,"\nPCM position out of bounds: got %li.\n",
     (long)pcm_offset);
    exit(EXIT_FAILURE);
  }
  nsamples=op_read_native(_of,buffer,sizeof(buffer)/sizeof(*buffer),&li);
  if(nsamples<0){
    fprintf(stderr,"\nFailed to read PCM data after seek: %i\n",nsamples);
    exit(EXIT_FAILURE);
  }
  for(lj=0;lj<li;lj++){
    duration=op_pcm_total(_of,lj);
    pcm_offset-=duration;
    if(pcm_offset<0){
      fprintf(stderr,"\nPCM data after seek came from the wrong link: "
       "expected %i, got %i.\n",lj,li);
      exit(EXIT_FAILURE);
    }
    _bigassbuffer+=op_channel_count(_of,lj)*duration;
  }
  duration=op_pcm_total(_of,li);
  if(pcm_offset+nsamples>duration){
    fprintf(stderr,"\nPCM data after seek exceeded link duration: "
     "limit %li, got %li.\n",duration,pcm_offset+nsamples);
    exit(EXIT_FAILURE);
  }
  nchannels=op_channel_count(_of,li);
  for(i=0;i<nsamples*nchannels;i++){
    if(!MATCH(buffer[i],_bigassbuffer[pcm_offset*nchannels+i])){
      fprintf(stderr,"\nData after seek doesn't match declared PCM "
       "position: mismatch %G\n",
       (double)buffer[i]-_bigassbuffer[pcm_offset*nchannels+i]);
      for(i=0;i<duration-nsamples;i++){
        int j;
        for(j=0;j<nsamples*nchannels;j++){
          if(!MATCH(buffer[j],_bigassbuffer[i*nchannels+j]))break;
        }
        if(j==nsamples*nchannels){
          fprintf(stderr,"\nData after seek appears to match position %li.\n",
           (long)i);
        }
      }
      exit(EXIT_FAILURE);
    }
  }
#if defined(OP_WRITE_SEEK_SAMPLES)
  write_samples(buffer,nsamples,nchannels);
#endif
}

#define OP_MIN(_a,_b) ((_a)<(_b)?(_a):(_b))

/*A simple wrapper that lets us count the number of underlying seek calls.*/

static op_seek_func real_seek;

static long nreal_seeks;

static int seek_stat_counter(void *_stream,opus_int64 _offset,int _whence){
  if(_whence==SEEK_SET)nreal_seeks++;
  /*SEEK_CUR with an offset of 0 is free, as is SEEK_END with an offset of 0
     (assuming we know the file size), so don't count them.*/
  else if(_offset!=0)nreal_seeks++;
  return (*real_seek)(_stream,_offset,_whence);
}

#define NSEEK_TESTS (1000)

static void print_duration(FILE *_fp,ogg_int64_t _nsamples){
  ogg_int64_t seconds;
  ogg_int64_t minutes;
  ogg_int64_t hours;
  ogg_int64_t days;
  ogg_int64_t weeks;
  seconds=_nsamples/48000;
  _nsamples-=seconds*48000;
  minutes=seconds/60;
  seconds-=minutes*60;
  hours=minutes/60;
  minutes-=hours*60;
  days=hours/24;
  hours-=days*24;
  weeks=days/7;
  days-=weeks*7;
  if(weeks)fprintf(_fp,"%liw",(long)weeks);
  if(weeks||days)fprintf(_fp,"%id",(int)days);
  if(weeks||days||hours)fprintf(_fp,"%ih",(int)hours);
  if(weeks||days||hours||minutes)fprintf(_fp,"%im",(int)minutes);
  fprintf(_fp,"%i.%03is",(int)seconds,(int)(_nsamples+24)/48);
}

int main(int _argc,const char **_argv){
  OpusFileCallbacks  cb;
  OggOpusFile       *of;
  void              *fp;
#if defined(_WIN32)
# undef fileno
# define fileno _fileno
  /*We need to set stdin/stdout to binary mode. Damn windows.*/
  /*Beware the evil ifdef. We avoid these where we can, but this one we
     cannot.
    Don't add any more.
    You'll probably go to hell if you do.*/
  _setmode(fileno(stdin),_O_BINARY);
  _setmode(fileno(stdout),_O_BINARY);
#endif
  if(_argc!=2){
    fprintf(stderr,"Usage: %s <file.opus>\n",_argv[0]);
    return EXIT_FAILURE;
  }
  memset(&cb,0,sizeof(cb));
  if(strcmp(_argv[1],"-")==0)fp=op_fdopen(&cb,fileno(stdin),"rb");
  else fp=op_fopen(&cb,_argv[1],"rb");
  if(cb.seek!=NULL){
    real_seek=cb.seek;
    cb.seek=seek_stat_counter;
  }
  of=op_open_callbacks(fp,&cb,NULL,0,NULL);
  if(of==NULL){
    fprintf(stderr,"Failed to open file '%s'.\n",_argv[1]);
    return EXIT_FAILURE;
  }
  if(op_seekable(of)){
    op_sample   *bigassbuffer;
    ogg_int64_t  size;
    ogg_int64_t  pcm_print_offset;
    ogg_int64_t  pcm_offset;
    ogg_int64_t  pcm_length;
    ogg_int64_t  nsamples;
    ogg_int64_t  si;
    opus_int32   bitrate;
    int          nlinks;
    int          ret;
    int          li;
    int          i;
    /*Because we want to do sample-level verification that the seek does what
       it claimed, decode the entire file into memory.*/
    nlinks=op_link_count(of);
    fprintf(stderr,"Opened file containing %i links with %li seeks "
     "(%0.3f per link).\n",nlinks,nreal_seeks,nreal_seeks/(double)nlinks);
    /*Reset the seek counter.*/
    nreal_seeks=0;
    nsamples=0;
    for(li=0;li<nlinks;li++){
      nsamples+=op_pcm_total(of,li)*op_channel_count(of,li);
    }
    bigassbuffer=_ogg_malloc(sizeof(*bigassbuffer)*nsamples);
    pcm_offset=op_pcm_tell(of);
    if(pcm_offset!=0){
      fprintf(stderr,"Initial PCM offset was not 0, got %li instead.!\n",
       (long)pcm_offset);
      exit(EXIT_FAILURE);
    }
    pcm_print_offset=pcm_offset-48000;
    bitrate=0;
    for(si=0;si<nsamples;){
      ogg_int64_t next_pcm_offset;
      opus_int32  next_bitrate;
      ret=op_read_native(of,bigassbuffer+si,OP_MIN(120*48*8,nsamples-si),&li);
      if(ret<=0){
        fprintf(stderr,"Failed to read PCM data: %i\n",ret);
        exit(EXIT_FAILURE);
      }
      /*If we have gaps in the PCM positions, seeking is not likely to work
         near them.*/
      next_pcm_offset=op_pcm_tell(of);
      if(pcm_offset+ret!=next_pcm_offset){
        fprintf(stderr,"\nGap in PCM offset: expecting %li, got %li\n",
         (long)(pcm_offset+ret),(long)next_pcm_offset);
        exit(EXIT_FAILURE);
      }
      pcm_offset=next_pcm_offset;
      si+=ret*op_channel_count(of,li);
      if(pcm_offset>=pcm_print_offset+48000){
        next_bitrate=op_bitrate_instant(of);
        if(next_bitrate>=0)bitrate=next_bitrate;
        fprintf(stderr,"\rLoading... [%li left] (%0.3f kbps)               ",
         nsamples-si,bitrate/1000.0);
        pcm_print_offset=pcm_offset;
      }
    }
    {
      op_sample tmp[8];
      ret=op_read_native(of,tmp,sizeof(tmp)/sizeof(*tmp),&li);
      if(ret<0){
        fprintf(stderr,"Failed to read PCM data: %i\n",ret);
        exit(EXIT_FAILURE);
      }
      if(ret>0){
        fprintf(stderr,"Read too much PCM data!\n");
        exit(EXIT_FAILURE);
      }
    }
    pcm_length=op_pcm_total(of,-1);
    size=op_raw_total(of,-1);
    fprintf(stderr,"\rLoaded (%0.3f kbps average).                        \n",
     op_bitrate(of,-1)/1000.0);
    fprintf(stderr,"Testing raw seeking to random places in %li bytes...\n",
     (long)size);
    for(i=0;i<NSEEK_TESTS;i++){
      opus_int64 byte_offset;
      byte_offset=(opus_int64)(rand()/(double)RAND_MAX*size);
      fprintf(stderr,"\r\t%3i [raw position %li]...                ",
       i,(long)byte_offset);
      ret=op_raw_seek(of,byte_offset);
      if(ret<0){
        fprintf(stderr,"\nSeek failed: %i.\n",ret);
        exit(EXIT_FAILURE);
      }
      if(i==28){
        i=28;
      }
      verify_seek(of,byte_offset,-1,pcm_length,bigassbuffer);
    }
    fprintf(stderr,"\rTotal seek operations: %li (%.3f per raw seek).\n",
     nreal_seeks,nreal_seeks/(double)NSEEK_TESTS);
    nreal_seeks=0;
    fprintf(stderr,"Testing PCM page seeking to random places in %li "
     "samples (",(long)pcm_length);
    print_duration(stderr,pcm_length);
    fprintf(stderr,")...\n");
    for(i=0;i<NSEEK_TESTS;i++){
      pcm_offset=(ogg_int64_t)(rand()/(double)RAND_MAX*pcm_length);
      fprintf(stderr,"\r\t%3i [PCM position %li]...                ",
       i,(long)pcm_offset);
      ret=op_pcm_seek_page(of,pcm_offset);
      if(ret<0){
        fprintf(stderr,"\nSeek failed: %i.\n",ret);
        exit(EXIT_FAILURE);
      }
      verify_seek(of,-1,pcm_offset,pcm_length,bigassbuffer);
    }
    fprintf(stderr,"\rTotal seek operations: %li (%.3f per page seek).\n",
     nreal_seeks,nreal_seeks/(double)NSEEK_TESTS);
    nreal_seeks=0;
    fprintf(stderr,"Testing exact PCM seeking to random places in %li "
     "samples (",(long)pcm_length);
    print_duration(stderr,pcm_length);
    fprintf(stderr,")...\n");
    for(i=0;i<NSEEK_TESTS;i++){
      ogg_int64_t pcm_offset2;
      pcm_offset=(ogg_int64_t)(rand()/(double)RAND_MAX*pcm_length);
      fprintf(stderr,"\r\t%3i [PCM position %li]...                ",
       i,(long)pcm_offset);
      ret=op_pcm_seek(of,pcm_offset);
      if(ret<0){
        fprintf(stderr,"\nSeek failed: %i.\n",ret);
        exit(EXIT_FAILURE);
      }
      pcm_offset2=op_pcm_tell(of);
      if(pcm_offset!=pcm_offset2){
        fprintf(stderr,"\nDeclared PCM position did not perfectly match "
         "request: requested %li, got %li.\n",
         (long)pcm_offset,(long)pcm_offset2);
        exit(EXIT_FAILURE);
      }
      verify_seek(of,-1,pcm_offset,pcm_length,bigassbuffer);
    }
    fprintf(stderr,"\rTotal seek operations: %li (%.3f per exact seek).\n",
     nreal_seeks,nreal_seeks/(double)NSEEK_TESTS);
    nreal_seeks=0;
    fprintf(stderr,"OK.\n");
  }
  else{
    fprintf(stderr,"Input was not seekable.\n");
    exit(EXIT_FAILURE);
  }
  op_free(of);
  return EXIT_SUCCESS;
}
