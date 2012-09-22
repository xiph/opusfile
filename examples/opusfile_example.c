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
#include <opusfile.h>

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
    of=op_open_callbacks(op_fdopen(&cb,fileno(stdin),"rb"),&cb,NULL,0,&ret);
  }
  else{
    /*Try to treat the argument as a URL.*/
    of=op_open_url(_argv[1],OP_SSL_SKIP_CERTIFICATE_CHECK,&ret);
#if 0
    if(of==NULL){
        OpusFileCallbacks  cb={NULL,NULL,NULL,NULL};
        void              *fp;
        /*For debugging: force a file to not be seekable.*/
        fp=op_fopen(&cb,_argv[1],"rb");
        cb.seek=NULL;
        cb.tell=NULL;
        of=op_open_callbacks(fp,&cb,NULL,0,NULL);
      }
#else
    if(of==NULL)of=op_open_file(_argv[1],&ret);
#endif
  }
  if(of==NULL){
    fprintf(stderr,"Failed to open file '%s': %i\n",_argv[1],ret);
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
    float       pcm[120*48*2];
    int         li;
    ret=op_read_float_stereo(of,pcm,sizeof(pcm)/sizeof(*pcm));
    if(ret<0){
      fprintf(stderr,"Error decoding '%s': %i\n",_argv[1],ret);
      ret=EXIT_FAILURE;
      break;
    }
    li=op_current_link(of);
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
    if(!fwrite(pcm,sizeof(*pcm)*2,ret,stdout)){
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
