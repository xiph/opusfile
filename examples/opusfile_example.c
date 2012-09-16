/*For fileno()*/
#if !defined(_POSIX_SOURCE)
# define _POSIX_SOURCE 1
#endif
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#if defined(_WIN32)
/*We need the following two to set stdin/stdout to binary.*/
# include <io.h>
# include <fcntl.h>
#endif
#include <opus/opusfile.h>

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

int main(int _argc,const char **_argv){
  OggOpusFile *of;
  ogg_int64_t  pcm_offset;
  ogg_int64_t  nsamples;
  int          ret;
  int          prev_li;
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
  if(strcmp(_argv[1],"-")==0){
    OpusFileCallbacks cb={NULL,NULL,NULL,NULL};
    of=op_open_callbacks(op_fdopen(&cb,fileno(stdin),"rb"),&cb,NULL,0,NULL);
  }
#if 0
  /*For debugging: force a file to not be seekable.*/
  else{
    OpusFileCallbacks  cb={NULL,NULL,NULL,NULL};
    void              *fp;
    fp=op_fopen(&cb,_argv[1],"rb");
    cb.seek=NULL;
    cb.tell=NULL;
    of=op_open_callbacks(fp,&cb,NULL,0,NULL);
  }
#else
  else of=op_open_file(_argv[1],NULL);
#endif
  if(of==NULL){
    fprintf(stderr,"Failed to open file '%s'.\n",_argv[1]);
    return EXIT_FAILURE;
  }
  if(op_seekable(of)){
    ogg_int64_t duration;
    fprintf(stderr,"Total number of links: %i\n",op_link_count(of));
    duration=op_pcm_total(of,-1);
    fprintf(stderr,"Total duration: %f seconds (%li samples @ 48 kHz).\n",
     duration/48000.0,(long)duration);
  }
  prev_li=-1;
  nsamples=0;
  pcm_offset=op_pcm_tell(of);
  if(pcm_offset!=0){
    fprintf(stderr,"Non-zero starting PCM offset: %li\n",(long)pcm_offset);
  }
  for(;;){
    ogg_int64_t next_pcm_offset;
    float       pcm[120*48*8];
    float       stereo_pcm[120*48*2];
    int         nchannels;
    int         li;
    int         i;
    ret=op_read_float(of,pcm,sizeof(pcm)/sizeof(*pcm),&li);
    if(ret<0){
      fprintf(stderr,"Error decoding '%s': %i\n",_argv[1],ret);
      ret=EXIT_FAILURE;
      break;
    }
    if(li!=prev_li){
      const OpusHead *head;
      const OpusTags *tags;
      int             ci;
      /*We found a new link.
        Print out some information.*/
      fprintf(stderr,"Decoding link %i:\n",li);
      head=op_head(of,li);
      fprintf(stderr,"  Channels: %i\n",head->channel_count);
      if(op_seekable(of)){
        ogg_int64_t duration;
        duration=op_pcm_total(of,li);
        fprintf(stderr,"  Duration: %f seconds (%li samples @ 48 kHz).\n",
         duration/48000.0,(long)duration);
      }
      if(head->input_sample_rate){
        fprintf(stderr,"  Original sampling rate: %lu Hz\n",
         (unsigned long)head->input_sample_rate);
      }
      tags=op_tags(of,li);
      fprintf(stderr,"  Encoded by: %s\n",tags->vendor);
      for(ci=0;ci<tags->comments;ci++){
        fprintf(stderr,"  %s\n",tags->user_comments[ci]);
      }
      fprintf(stderr,"\n");
      if(!op_seekable(of)){
        pcm_offset=op_pcm_tell(of)-ret;
        if(pcm_offset!=0){
          fprintf(stderr,"Non-zero starting PCM offset in link %i: %li\n",
           li,(long)pcm_offset);
        }
      }
    }
    next_pcm_offset=op_pcm_tell(of);
    if(pcm_offset+ret!=next_pcm_offset){
      fprintf(stderr,"PCM offset gap! %li+%i!=%li\n",
       (long)pcm_offset,ret,(long)next_pcm_offset);
    }
    pcm_offset=next_pcm_offset;
    if(ret<=0){
      ret=EXIT_SUCCESS;
      break;
    }
    /*Downmix to stereo so we can have a consistent output format.*/
    nchannels=op_channel_count(of,li);
    if(nchannels<0||nchannels>8){
      fprintf(stderr,"Unsupported channel count: %i\n",nchannels);
      ret=EXIT_FAILURE;
      break;
    }
    for(i=0;i<ret;i++){
      float l;
      float r;
      int   ci;
      l=r=0.F;
      for(ci=0;ci<nchannels;ci++){
        l+=DOWNMIX_MATRIX[nchannels-1][ci][0]*pcm[i*nchannels+ci];
        r+=DOWNMIX_MATRIX[nchannels-1][ci][1]*pcm[i*nchannels+ci];
      }
      stereo_pcm[2*i+0]=l;
      stereo_pcm[2*i+1]=r;
    }
    if(!fwrite(stereo_pcm,sizeof(*stereo_pcm)*2,ret,stdout)){
      fprintf(stderr,"Error writing decoded audio data: %s\n",strerror(errno));
      ret=EXIT_FAILURE;
      break;
    }
    nsamples+=ret;
    prev_li=li;
  }
  op_free(of);
  if(ret==EXIT_SUCCESS){
    fprintf(stderr,"Done (played %li samples).\n",(long)nsamples);
  }
  return ret;
}
