/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE libopusfile SOFTWARE CODEC SOURCE CODE. *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE libopusfile SOURCE CODE IS (C) COPYRIGHT 1994-2012           *
 * by the Xiph.Org Foundation and contributors http://www.xiph.org/ *
 *                                                                  *
 ********************************************************************

 function: stdio-based convenience library for opening/seeking/decoding
 last mod: $Id: vorbisfile.h 17182 2010-04-29 03:48:32Z xiphmont $

 ********************************************************************/
#if !defined(_opusfile_h)
# define _opusfile_h (1)

/**\mainpage
   \section Introduction

   This is the documentation for the <tt>libopusfile</tt> C API.

   The <tt>libopusfile</tt> package provides a convenient high-level API for
    decoding and basic manipulation of all Ogg Opus audio streams.
   <tt>libopusfile</tt> is implemented as a layer on top of Xiph.Org's
    reference
    <tt><a href="https://www.xiph.org/ogg/doc/libogg/reference.html">libogg</a></tt>
    and
    <tt><a href="https://mf4.xiph.org/jenkins/view/opus/job/opus/ws/doc/html/index.html">libopus</a></tt>
    libraries.

   <tt>libopusfile</tt> provides serveral sets of built-in routines for
    file/stream access, and may also use custom stream I/O routines provided by
    the embedded environment.
   There are built-in I/O routines provided for ANSI-compliant
    <code>stdio</code> (<code>FILE *</code>), memory buffers, and URLs
    (including "file:" URLs, plus optionally "http:" and "https:" URLs).

   \section Organization

   The main API is divided into several sections:
   - \ref stream_open_close
   - \ref stream_info
   - \ref stream_decoding
   - \ref stream_seeking

   Several additional sections are not tied to the main API.
   - \ref stream_callbacks
   - \ref header_info
   - \ref error_codes*/


# if defined(__cplusplus)
extern "C" {
# endif

# include <stdio.h>
# include <ogg/ogg.h>
# include <opus_multistream.h>

/*Enable special features for gcc and gcc-compatible compilers.*/
# if !defined(OP_GNUC_PREREQ)
#  if defined(__GNUC__)&&defined(__GNUC_MINOR__)
#   define OP_GNUC_PREREQ(_maj,_min) \
 ((__GNUC__<<16)+__GNUC_MINOR__>=((_maj)<<16)+(_min))
#  else
#   define OP_GNUC_PREREQ(_maj,_min) 0
#  endif
# endif

# if OP_GNUC_PREREQ(4,0)
#  pragma GCC visibility push(default)
# endif

typedef struct OpusHead    OpusHead;
typedef struct OpusTags    OpusTags;
typedef struct OggOpusFile OggOpusFile;

/*Warning attributes for libopusfile functions.*/
# if OP_GNUC_PREREQ(3,4)
#  define OP_WARN_UNUSED_RESULT __attribute__((__warn_unused_result__))
# else
#  define OP_WARN_UNUSED_RESULT
# endif
# if OP_GNUC_PREREQ(3,4)
#  define OP_ARG_NONNULL(_x) __attribute__((__nonnull__(_x)))
# else
#  define OP_ARG_NONNULL(_x)
# endif

/**\defgroup error_codes Error Codes*/
/*@{*/
/**\name List of possible error codes
   Many of the functions in this library return a negative error code when a
    function fails.
   This list provides a brief explanation of the common errors.
   See each individual function for more details on what a specific error code
    means in that context.*/
/*@{*/

/**A request did not succeed.*/
#define OP_FALSE         (-1)
/*Currently not used externally.*/
#define OP_EOF           (-2)
/**There was a hole in the page sequence numbers (e.g., a page was corrupt or
    missing).*/
#define OP_HOLE          (-3)
/**An underlying read, seek, or tell operation failed when it should have
    succeeded.*/
#define OP_EREAD         (-128)
/**A <code>NULL</code> pointer was passed where one was unexpected, or an
    internal memory allocation failed, or an internal library error was
    encountered.*/
#define OP_EFAULT        (-129)
/**The stream used a feature that is not implemented, such as an unsupported
    channel family.*/
#define OP_EIMPL         (-130)
/**One or more parameters to a function were invalid.*/
#define OP_EINVAL        (-131)
/**A purported Ogg Opus stream did not begin with an Ogg page, a purported
    header packet did not start with one of the required strings, "OpusHead" or
    "OpusTags", or a link in a chained file was encountered that did not
    contain any logical Opus streams.*/
#define OP_ENOTFORMAT    (-132)
/**A required header packet was not properly formatted, contained illegal
    values, or was missing altogether.*/
#define OP_EBADHEADER    (-133)
/**The ID header contained an unrecognized version number.*/
#define OP_EVERSION      (-134)
/*Currently not used at all.*/
#define OP_ENOTAUDIO     (-135)
/**An audio packet failed to decode properly.
   This is usually caused by a multistream Ogg packet where the durations of
    the individual Opus packets contained in it are not all the same.*/
#define OP_EBADPACKET    (-136)
/**We failed to find data we had seen before, or the bitstream structure was
    sufficiently malformed that seeking to the target destination was
    impossible.*/
#define OP_EBADLINK      (-137)
/**An operation that requires seeking was requested on an unseekable stream.*/
#define OP_ENOSEEK       (-138)
/**The first or last granule position of a link failed basic validity checks.*/
#define OP_EBADTIMESTAMP (-139)

/*@}*/
/*@}*/

/**\defgroup header_info Header Information*/
/*@{*/

/**The maximum number of channels in an Ogg Opus stream.*/
#define OPUS_CHANNEL_COUNT_MAX (255)

/**Ogg Opus bitstream information.
   This contains the basic playback parameters for a stream, and corresponds to
    the initial ID header packet of an Ogg Opus stream.*/
struct OpusHead{
  /**The Ogg Opus format version, in the range 0...255.
     The top 4 bits represent a "major" version, and the bottom four bits
      represent backwards-compatible "minor" revisions.
     The current specification describes version 1.
     This library will recognize versions up through 15 as backwards compatible
      with the current specification.
     An earlier draft of the specification described a version 0, but the only
      difference between version 1 and version 0 is that version 0 did
      not specify the semantics for handling the version field.*/
  int           version;
  /**The number of channels, in the range 1...255.*/
  int           channel_count;
  /**The number of samples that should be discarded from the beginning of the
      stream.*/
  unsigned      pre_skip;
  /**The sampling rate of the original input.
     All Opus audio is coded at 48 kHz, and should also be decoded at 48 kHz
      for playback (unless the target hardware does not support this sampling
      rate).
     However, this field may be used to resample the audio back to the original
      sampling rate, for example, when saving the output to a file.*/
  opus_uint32   input_sample_rate;
  /**The gain to apply to the decoded output, in dB, as a Q8 value in the range
      -32768...32767.
     The decoder will automatically scale the output by
      pow(10,output_gain/(20.0*256)).*/
  int           output_gain;
  /**The channel mapping family, in the range 0...255.
     Channel mapping family 0 covers mono or stereo in a single stream.
     Channel mapping family 1 covers 1 to 8 channels in one or more streams,
      using the Vorbis speaker assignments.
     Channel mapping family 255 covers 1 to 255 channels in one or more
      streams, but without any defined speaker assignment.*/
  int           mapping_family;
  /**The number of Opus streams in each Ogg packet, in the range 1...255.*/
  int           stream_count;
  /**The number of coupled Opus streams in each Ogg packet, in the range
      0...127.
     This must satisfy <code>0 <= coupled_count <= stream_count</code> and
      <code>coupled_count + stream_count <= 255</code>.
     The coupled streams appear first, before all uncoupled streams, in an Ogg
      Opus packet.*/
  int           coupled_count;
  /**The mapping from coded stream channels to output channels.
     Let <code>index=mapping[k]</code> be the value for channel <code>k</code>.
     If <code>index<2*coupled_count</code>, then it refers to the left channel
      from stream <code>(index/2)</code> if even, and the right channel from
      stream <code>(index/2)</code> if odd.
     Otherwise, it refers to the output of the uncoupled stream
      <code>(index-coupled_count)</code>.*/
  unsigned char mapping[OPUS_CHANNEL_COUNT_MAX];
};

/**The metadata from an Ogg Opus stream.

   This structure holds the in-stream metadata corresponding to the 'comment'
    header packet of an Ogg Opus stream.
   The comment header is meant to be used much like someone jotting a quick
    note on the label of a CD.
   It should be a short, to the point text note that can be more than a couple
    words, but not more than a short paragraph.

   The metadata is stored as a series of (tag, value) pairs, in length-encoded
    string vectors, using the same format as Vorbis (without the final "framing
    bit"), Theora, and Speex, except for the packet header.
   The first occurrence of the '=' character delimits the tag and value.
   A particular tag may occur more than once, and order is significant.
   The character set encoding for the strings is always UTF-8, but the tag
    names are limited to ASCII, and treated as case-insensitive.
   See <a href="http://www.xiph.org/vorbis/doc/v-comment.html">the Vorbis
    comment header specification</a> for details.

   In filling in this structure, \a libopsfile will null-terminate the
    #user_comments strings for safety.
   However, the bitstream format itself treats them as 8-bit clean vectors,
    possibly containing NUL characters, so the #comment_lengths array should be
    treated as their authoritative length.

   This structure is binary and source-compatible with a
    <code>vorbis_comment</code>, and pointers to it may be freely cast to
    <code>vorbis_comment</code> pointers, and vice versa.
   It is provided as a separate type to avoid introducing a compile-time
    dependency on the libvorbis headers.*/
struct OpusTags{
  /**The array of comment string vectors.*/
  char **user_comments;
  /**An array of the corresponding length of each vector, in bytes.*/
  int   *comment_lengths;
  /**The total number of comment streams.*/
  int    comments;
  /**The null-terminated vendor string.
     This identifies the software used to encode the stream.*/
  char  *vendor;
};

/**\name Functions for manipulating header data

   These functions manipulate the #OpusHead and #OpusTags structures,
    which describe the audio parameters and tag-value metadata, respectively.
   These can be used to query the headers returned by <tt>libopusfile</tt>, or
    to parse Opus headers from sources other than an Ogg Opus stream, provided
    they use the same format.*/
/*@{*/

/**Parses the contents of the ID header packet of an Ogg Opus stream.
   \param[out] _head Returns the contents of the parsed packet.
                     The contents of this structure are untouched on error.
                     This may be <code>NULL</code> to merely test the header
                      for validity.
   \param[in]  _data The contents of the ID header packet.
   \param      _len  The number of bytes of data in the ID header packet.
   \return 0 on success or a negative value on error.
   \retval #OP_ENOTFORMAT If the data does not start with the "OpusHead"
                           string.
   \retval #OP_EVERSION   If the version field signaled a version this library
                           does not know how to parse.
   \retval #OP_EIMPL      If the channel mapping family was 255, which general
                           purpose players should not attempt to play.
   \retval #OP_EBADHEADER If the contents of the packet otherwise violate the
                           Ogg Opus specification:
                          <ul>
                           <li>Insufficient data,</li>
                           <li>Too much data for the known minor versions,</li>
                           <li>An unrecognized channel mapping family,</li>
                           <li>Zero channels or too many channels,</li>
                           <li>Zero coded streams,</li>
                           <li>Too many coupled streams, or</li>
                           <li>An invalid channel mapping index.</li>
                          </ul>*/
OP_WARN_UNUSED_RESULT int opus_head_parse(OpusHead *_head,
 const unsigned char *_data,size_t _len) OP_ARG_NONNULL(2);

/**Converts a granule position to a sample offset for a given Ogg Opus stream.
   The sample offset is simply <code>_gp-_head->pre_skip</code>.
   Granule position values smaller than OpusHead#pre_skip correspond to audio
    that should never be played, and thus have no associated sample offset.
   This function returns -1 for such values.
   This function also correctly handles extremely large granule positions,
    which may have wrapped around to a negative number when stored in a signed
    ogg_int64_t value.
   \param _head The #OpusHead information from the ID header of the stream.
   \param _gp   The granule position to convert.
   \return The sample offset associated with the given granule position
            (counting at a 48 kHz sampling rate), or the special value -1 on
            error (i.e., the granule position was smaller than the pre-skip
            amount).*/
ogg_int64_t opus_granule_sample(const OpusHead *_head,ogg_int64_t _gp)
 OP_ARG_NONNULL(1);

/**Parses the contents of the 'comment' header packet of an Ogg Opus stream.
   \param[out] _tags An uninitialized #OpusTags structure.
                     This returns the contents of the parsed packet.
                     The contents of this structure are untouched on error.
                     This may be <code>NULL</code> to merely test the header
                      for validity.
   \param[in]  _data The contents of the 'comment' header packet.
   \param      _len  The number of bytes of data in the 'info' header packet.
   \retval 0             Success.
   \retval #OP_ENOTFORMAT If the data does not start with the "OpusTags"
                           string.
   \retval #OP_EBADHEADER If the contents of the packet otherwise violate the
                           Ogg Opus specification.
   \retval #OP_EFAULT     If there wasn't enough memory to store the tags.*/
OP_WARN_UNUSED_RESULT int opus_tags_parse(OpusTags *_tags,
 const unsigned char *_data,size_t _len) OP_ARG_NONNULL(2);

/**Initializes an #OpusTags structure.
   This should be called on a freshly allocated #OpusTags structure before
    attempting to use it.
   \param _tags The #OpusTags structure to initialize.*/
void opus_tags_init(OpusTags *_tags) OP_ARG_NONNULL(1);

/**Add a (tag, value) pair to an initialized #OpusTags structure.
   \note Neither opus_tags_add() nor opus_tags_add_comment() support values
    containing embedded NULs, although the bitstream format does support them.
   To add such tags, you will need to manipulate the #OpusTags structure
    directly.
   \param _tags  The #OpusTags structure to add the (tag, value) pair to.
   \param _tag   A NUL-terminated, case-insensitive, ASCII string containing
                  the tag to add (without an '=' character).
   \param _value A NUL-terminated UTF-8 containing the corresponding value.
   \return 0 on success, or a negative value on failure.
   \retval OP_EFAULT An internal memory allocation failed.*/
int opus_tags_add(OpusTags *_tags,const char *_tag,const char *_value)
 OP_ARG_NONNULL(1) OP_ARG_NONNULL(2) OP_ARG_NONNULL(3);

/**Add a comment to an initialized #OpusTags structure.
   \note Neither opus_tags_add_comment() nor opus_tags_add() support comments
    containing embedded NULs, although the bitstream format does support them.
   To add such tags, you will need to manipulate the #OpusTags structure
    directly.
   \param _tags    The #OpusTags structure to add the comment to.
   \param _comment A NUL-terminated UTF-8 string containing the comment in
                    "TAG=value" form.
   \return 0 on success, or a negative value on failure.
   \retval OP_EFAULT An internal memory allocation failed.*/
int opus_tags_add_comment(OpusTags *_tags,const char *_comment)
 OP_ARG_NONNULL(1) OP_ARG_NONNULL(2);

/**Look up a comment value by its tag.
   \param _tags  An initialized #OpusTags structure.
   \param _tag   The tag to look up.
   \param _count The instance of the tag.
                 The same tag can appear multiple times, each with a distinct
                  value, so an index is required to retrieve them all.
                 The order in which these values appear is significant and
                  should be preserved.
                 Use opus_tags_query_count() to get the legal range for the
                  \a _count parameter.
   \return A pointer to the queried tag's value.
           This points directly to data in the #OpusTags structure.
           It should not be modified or freed by the application, and
            modifications to the structure may invalidate the pointer.
   \retval <code>NULL</code> if no matching tag is found.*/
const char *opus_tags_query(const OpusTags *_tags,const char *_tag,int _count)
 OP_ARG_NONNULL(1) OP_ARG_NONNULL(2);

/**Look up the number of instances of a tag.
   Call this first when querying for a specific tag and then iterate over the
    number of instances with separate calls to opus_tags_query() to retrieve
    all the values for that tag in order.
   \param _tags An initialized #OpusTags structure.
   \param _tag  The tag to look up.
   \return The number of instances of this particular tag.*/
int opus_tags_query_count(const OpusTags *_tags,const char *_tag)
 OP_ARG_NONNULL(1) OP_ARG_NONNULL(2);

/**Clears the #OpusTags structure.
   This should be called on an #OpusTags structure after it is no longer
    needed.
   It will free all memory used by the structure members.
   \param _tags The #OpusTags structure to clear.*/
void opus_tags_clear(OpusTags *_tags) OP_ARG_NONNULL(1);

/*@}*/

/*@}*/

/**\defgroup url_flags URL Reading Flags*/
/*@{*/
/**\name URL reading flags
   Flags for op_url_create_with_proxy() and associated functions.
   These may be expanded in the future.*/
/*@{*/

/**Skip the certificate check when connecting via TLS/SSL (https).*/
#define OP_SSL_SKIP_CERTIFICATE_CHECK (1)

/*@}*/
/*@}*/

/**\defgroup stream_callbacks Abstract Stream Reading Interface*/
/*@{*/
/**\name Functions for reading from streams
   These functions define the interface used to read from and seek in a stream
    of data.
   A stream does not need to implement seeking, but the decoder will not be
    able to seek if it does not do so.
   These functions also include some convenience routines for working with
    standard <code>FILE</code> pointers, complete streams stored in a single
    block of memory, or URLs.*/
/*@{*/

typedef struct OpusFileCallbacks OpusFileCallbacks;

/**Reads \a _nmemb elements of data, each \a _size bytes long, from
    \a _stream.
   \return The number of items successfully read (i.e., not the number of
            characters).
           Unlike normal <code>fread()</code>, this function is allowed to
            return fewer items than requested (e.g., if reading more would
            block), as long as <em>some</em> data is returned when no error
            occurs and EOF has not been reached.
           If an error occurs, or the end-of-file is reached, the return
            value is zero.
           <code>errno</code> need not be set.*/
typedef size_t (*op_read_func)(void *_ptr,size_t _size,size_t _nmemb,
 void *_stream);

/**Sets the position indicator for \a _stream.
   The new position, measured in bytes, is obtained by adding \a _offset
    bytes to the position specified by \a _whence.
   If \a _whence is set to <code>SEEK_SET</code>, <code>SEEK_CUR</code>, or
    <code>SEEK_END</code>, the offset is relative to the start of the stream,
    the current position indicator, or end-of-file, respectively.
   \retval 0  Success.
   \retval -1 Seeking is not supported or an error occurred.
              <code>errno</code> need not be set.*/
typedef int (*op_seek_func)(void *_stream,opus_int64 _offset,int _whence);

/**Obtains the current value of the position indicator for \a _stream.
   \return The current position indicator.*/
typedef opus_int64 (*op_tell_func)(void *_stream);

/**Closes the underlying stream.
   \retval 0   Success.
   \retval EOF An error occurred.
               <code>errno</code> need not be set.*/
typedef int (*op_close_func)(void *_stream);

/**The callbacks used to access non-<code>FILE</code> stream resources.
   The function prototypes are basically the same as for the stdio functions
    <code>fread()</code>, <code>fseek()</code>, <code>ftell()</code>, and
    <code>fclose()</code>.
   The differences are that the <code>FILE *</code> arguments have been
    replaced with a <code>void *</code>, which is to be used as a pointer to
    whatever internal data these functions might need, that #seek_func and
    #tell_func take and return 64-bit offsets, and that #seek_func *must*
    return -1 if the stream is unseekable.*/
struct OpusFileCallbacks{
  /**Used to read data from the stream.
     This must not be <code>NULL</code>.*/
  op_read_func  read;
  /**Used to seek in the stream.
     This may be <code>NULL</code> if seeking is not implemented.*/
  op_seek_func  seek;
  /**Used to return the current read position in the stream.
     This may be <code>NULL</code> if seeking is not implemented.*/
  op_tell_func  tell;
  /**Used to close the stream when the decoder is freed.
     This may be <code>NULL</code> to leave the stream open.*/
  op_close_func close;
};

/**Opens a stream with <code>fopen()</code> and fills in a set of callbacks
    that can be used to access it.
   This is useful to avoid writing your own portable 64-bit seeking wrappers,
    and also avoids cross-module linking issues on Windows, where a
    <code>FILE *</code> must be accessed by routines defined in the same module
    that opened it.
   \param[out] _cb   The callbacks to use for this file.
                     If there is an error opening the file, nothing will be
                      filled in here.
   \param      _path The path to the file to open.
   \param      _mode The mode to open the file in.
   \return A stream handle to use with the callbacks, or <code>NULL</code> on
            error.*/
OP_WARN_UNUSED_RESULT void *op_fopen(OpusFileCallbacks *_cb,
 const char *_path,const char *_mode) OP_ARG_NONNULL(1) OP_ARG_NONNULL(2)
 OP_ARG_NONNULL(3);

/**Opens a stream with <code>fdopen()</code> and fills in a set of callbacks
    that can be used to access it.
   This is useful to avoid writing your own portable 64-bit seeking wrappers,
    and also avoids cross-module linking issues on Windows, where a
    <code>FILE *</code> must be accessed by routines defined in the same module
    that opened it.
   \param[out] _cb   The callbacks to use for this file.
                     If there is an error opening the file, nothing will be
                      filled in here.
   \param      _fd   The file descriptor to open.
   \param      _mode The mode to open the file in.
   \return A stream handle to use with the callbacks, or <code>NULL</code> on
            error.*/
OP_WARN_UNUSED_RESULT void *op_fdopen(OpusFileCallbacks *_cb,
 int _fd,const char *_mode) OP_ARG_NONNULL(1) OP_ARG_NONNULL(3);

/**Opens a stream with <code>freopen()</code> and fills in a set of callbacks
    that can be used to access it.
   This is useful to avoid writing your own portable 64-bit seeking wrappers,
    and also avoids cross-module linking issues on Windows, where a
    <code>FILE *</code> must be accessed by routines defined in the same module
    that opened it.
   \param[out] _cb     The callbacks to use for this file.
                       If there is an error opening the file, nothing will be
                        filled in here.
   \param      _path   The path to the file to open.
   \param      _mode   The mode to open the file in.
   \param      _stream A stream previously returned by op_fopen(), op_fdopen(),
                        or op_freopen().
   \return A stream handle to use with the callbacks, or <code>NULL</code> on
            error.*/
OP_WARN_UNUSED_RESULT void *op_freopen(OpusFileCallbacks *_cb,
 const char *_path,const char *_mode,void *_stream) OP_ARG_NONNULL(1)
 OP_ARG_NONNULL(2) OP_ARG_NONNULL(3) OP_ARG_NONNULL(4);

/**Creates a stream that reads from the given block of memory.
   This block of memory must contain the complete stream to decode.
   This is useful for caching small streams (e.g., sound effects) in RAM.
   \param[out] _cb   The callbacks to use for this stream.
                     If there is an error creating the stream, nothing will be
                      filled in here.
   \param      _data The block of memory to read from.
   \param      _size The size of the block of memory.
   \return A stream handle to use with the callbacks, or <code>NULL</code> on
            error.*/
OP_WARN_UNUSED_RESULT void *op_mem_stream_create(OpusFileCallbacks *_cb,
 const unsigned char *_data,size_t _size) OP_ARG_NONNULL(1);

/**Creates a stream that reads from the given URL.
   This is equivalent to calling op_url_stream_create_with_proxy() with
    <code>NULL</code> for \a _proxy_host.
   \param[out] _cb    The callbacks to use for this stream.
                      If there is an error creating the stream, nothing will be
                       filled in here.
   \param      _url   The URL to read from.
                      Currently only the "file:", "http:", and "https:" schemes
                       are supported.
                      Both "http:" and "https:" may be disabled at compile
                       time, in which case opening such URLs will fail.
   \param      _flags The \ref url_flags "optional flags" to use.
   \return A stream handle to use with the callbacks, or <code>NULL</code> on
            error.*/
OP_WARN_UNUSED_RESULT void *op_url_stream_create(OpusFileCallbacks *_cb,
 const char *_url,int _flags) OP_ARG_NONNULL(1) OP_ARG_NONNULL(2);

/**Creates a stream that reads from the given URL using the specified proxy.
   \param[out] _cb         The callbacks to use for this stream.
                           If there is an error creating the stream, nothing
                            will be filled in here.
   \param      _url        The URL to read from.
                           Currently only the "file:", "http:", and "https:"
                            schemes are supported.
                           Both "http:" and "https:" may be disabled at compile
                            time, in which case opening such URLs will fail.
   \param      _flags      The \ref url_flags "optional flags" to use.
   \param      _proxy_host The host of the proxy to connect to.
                           This may be <code>NULL</code> if you do not wish to
                            use a proxy.
                           The proxy information is ignored if \a _url is a
                            <file:> URL.
   \param      _proxy_port The port of the proxy to connect to.
                           This is ignored if \a _proxy_host is
                            <code>NULL</code>.
   \param      _proxy_user The username to use with the specified proxy.
                           This may be <code>NULL</code> if no authorization is
                            required.
                           This is ignored if \a _proxy_host is
                            <code>NULL</code>.
   \param      _proxy_pass The password to use with the specified proxy.
                           This may be <code>NULL</code> if no authorization is
                            required.
                           This is ignored if either \a _proxy_host or
                            \a _proxy_user are <code>NULL</code>.
   \return A stream handle to use with the callbacks, or <code>NULL</code> on
            error.*/
OP_WARN_UNUSED_RESULT void *op_url_stream_create_with_proxy(
 OpusFileCallbacks *_cb,const char *_url,int _flags,
  const char *_proxy_host,unsigned _proxy_port,
  const char *_proxy_user,const char *_proxy_pass) OP_ARG_NONNULL(1)
  OP_ARG_NONNULL(2);

/*@}*/
/*@}*/

/**\defgroup stream_open_close Opening and Closing*/
/*@{*/
/**\name Functions for opening and closing streams

   These functions allow you to test a stream to see if it is Opus, open it,
    and close it.
   Several flavors are provided for each of the built-in stream types, plus a
    more general version which takes a set of application-provided callbacks.*/
/*@{*/

/**Test to see if this is an Opus stream.
   For good results, you will need at least 57 bytes (for a pure Opus-only
    stream).
   Something more like 512 bytes will give more reliable results for
    multiplexed streams.
   This function is meant to be a quick-rejection filter.
   Its purpose is not to guarantee that a stream is a valid Opus stream, but to
    ensure that it looks enough like Opus that it isn't going to be recognized
    as some other format (except possibly an Opus stream that is also
    multiplexed with other codecs, such as video).
   If you need something that gives a much better guarantee that this stream
    can be opened successfully, use op_test_callbacks() or one of the
    associated convenience functions.
   \param[out] _head     The parsed ID header contents.
                         You may pass <code>NULL</code> if you do not need
                          this information.
                         If the function fails, the contents of this structure
                          remain untouched.
   \param _initial_data  An initial buffer of data from the start of the
                          stream.
   \param _initial_bytes The number of bytes in \a _initial_data.
   \return 0 if the data appears to be Opus, or a negative value on error.
   \retval #OP_FALSE      There was not enough data to tell if this was an Opus
                           stream or not.
   \retval #OP_EFAULT     An internal memory allocation failed.
   \retval #OP_EIMPL      The stream used a feature that is not implemented,
                           such as an unsupported channel family.
   \retval #OP_ENOTFORMAT If the data did not contain a recognizable ID
                           header for an Opus stream.
   \retval #OP_EVERSION   If the version field signaled a version this library
                           does not know how to parse.
   \retval #OP_EBADHEADER A required header packet was not properly formatted,
                           contained illegal values, or was missing
                           altogether.*/
int op_test(OpusHead *_head,
 const unsigned char *_initial_data,size_t _initial_bytes);

/**Open a stream from the given file path.
   \param      _path  The path to the file to open.
   \param[out] _error Returns 0 on success, or a failure code on error.
                      You may pass in <code>NULL</code> if you don't want the
                       failure code.
                      The failure code will be #OP_EFAULT if the file could not
                       be opened, or one of the other failure codes from
                       op_open_callbacks() otherwise.
   \return A freshly opened #OggOpusFile, or <code>NULL</code> on error.*/
OP_WARN_UNUSED_RESULT OggOpusFile *op_open_file(const char *_path,int *_error)
 OP_ARG_NONNULL(1);

/**Open a stream from a memory buffer.
   \param      _data  The memory buffer to open.
   \param      _size  The number of bytes in the buffer.
   \param[out] _error Returns 0 on success, or a failure code on error.
                      You may pass in <code>NULL</code> if you don't want the
                       failure code.
                      See op_open_callbacks() for a full list of failure codes.
   \return A freshly opened #OggOpusFile, or <code>NULL</code> on error.*/
OP_WARN_UNUSED_RESULT OggOpusFile *op_open_memory(const unsigned char *_data,
 size_t _size,int *_error);

/**Open a stream from a URL.
   \param      _url   The URL to open.
                      Currently only the <file:>, <http:>, and <https:> schemes
                       are supported.
                      Both "http:" and "https:" may be disabled at compile
                       time, in which case opening such URLs will fail.
   \param      _flags The \ref url_flags "optional flags" to use.
   \param[out] _error Returns 0 on success, or a failure code on error.
                      You may pass in <code>NULL</code> if you don't want the
                       failure code.
                      See op_open_callbacks() for a full list of failure codes.
   \return A freshly opened #OggOpusFile, or <code>NULL</code> on error.*/
OP_WARN_UNUSED_RESULT OggOpusFile *op_open_url(const char *_url,
 int _flags,int *_error) OP_ARG_NONNULL(1);

/**Open a stream from a URL using the specified proxy.
   \param      _url        The URL to open.
                           Currently only the <file:>, <http:>, and <https:>
                            schemes are supported.
                           Both "http:" and "https:" may be disabled at compile
                            time, in which case opening such URLs will fail.
   \param      _flags      The \ref url_flags "optional flags" to use.
   \param      _proxy_host The host of the proxy to connect to.
                           This may be <code>NULL</code> if you do not wish to
                            use a proxy.
                           The proxy information is ignored if \a _url is a
                            <file:> URL.
   \param      _proxy_port The port of the proxy to connect to.
                           This is ignored if \a _proxy_host is
                            <code>NULL</code>.
   \param      _proxy_user The username to use with the specified proxy.
                           This may be <code>NULL</code> if no authorization is
                            required.
                           This is ignored if \a _proxy_host is
                            <code>NULL</code>.
   \param      _proxy_pass The password to use with the specified proxy.
                           This may be <code>NULL</code> if no authorization is
                            required.
                           This is ignored if either \a _proxy_host or
                            \a _proxy_user are <code>NULL</code>.
   \param[out] _error      Returns 0 on success, or a failure code on error.
                           You may pass in <code>NULL</code> if you don't want
                            the failure code.
                           See op_open_callbacks() for a full list of failure
                            codes.
   \return A freshly opened #OggOpusFile, or <code>NULL</code> on error.*/
OP_WARN_UNUSED_RESULT OggOpusFile *op_open_url_with_proxy(const char *_url,
 int _flags,const char *_proxy_host,unsigned _proxy_port,
 const char *_proxy_user,const char *_proxy_pass,int *_error)
 OP_ARG_NONNULL(1);

/**Open a stream using the given set of callbacks to access it.
   \param _source        The stream to read from (e.g., a <code>FILE *</code>).
   \param _cb            The callbacks with which to access the stream.
                         <code><a href="#op_read_func">read()</a></code> must
                          be implemented.
                         <code><a href="#op_seek_func">seek()</a></code> and
                          <code><a href="#op_tell_func">tell()</a></code> may
                          be <code>NULL</code>, or may always return -1 to
                          indicate a source is unseekable, but if
                          <code><a href="#op_seek_func">seek()</a></code> is
                          implemented and succeeds on a particular source, then
                          <code><a href="#op_tell_func">tell()</a></code> must
                          also.
                         <code><a href="#op_close_func">close()</a></code> may
                          be <code>NULL</code>, but if it is not, it will be
                          called when the #OggOpusFile is destroyed by
                          op_free().
                         It will not be called if op_open_callbacks() fails
                          with an error.
   \param _initial_data  An initial buffer of data from the start of the
                          stream.
                         Applications can read some number of bytes from the
                          start of the stream to help identify this as an Opus
                          stream, and then provide them here to allow the
                          stream to be opened, even if it is unseekable.
   \param _initial_bytes The number of bytes in \a _initial_data.
                         If the stream is seekable, its current position (as
                          reported by
                          <code><a href="#opus_tell_func">tell()</a></code>
                          at the start of this function) must be equal to
                          \a _initial_bytes.
                         Otherwise, seeking to absolute positions will
                          generate inconsistent results.
   \param[out] _error    Returns 0 on success, or a failure code on error.
                         You may pass in <code>NULL</code> if you don't want
                          the failure code.
                         The failure code will be one of
                         <dl>
                           <dt>#OP_EREAD</dt>
                           <dd>An underlying read, seek, or tell operation
                            failed when it should have succeeded, or we failed
                            to find data in the stream we had seen before.</dd>
                           <dt>#OP_EFAULT</dt>
                           <dd>There was a memory allocation failure, or an
                            internal library error.</dd>
                           <dt>#OP_EIMPL</dt>
                           <dd>The stream used a feature that is not
                            implemented, such as an unsupported channel
                            family.</dd>
                           <dt>#OP_EINVAL</dt>
                           <dd><code><a href="#op_seek_func">seek()</a></code>
                            was implemented and succeeded on this source, but
                            <code><a href="#op_tell_func">tell()</a></code>
                            did not, or the starting position indicator was
                            not equal to \a _initial_bytes.</dd>
                           <dt>#OP_ENOTFORMAT</dt>
                           <dd>The stream contained a link that did not have
                            any logical Opus streams in it.</dd>
                           <dt>#OP_EBADHEADER</dt>
                           <dd>A required header packet was not properly
                            formatted, contained illegal values, or was missing
                            altogether.</dd>
                           <dt>#OP_EVERSION</dt>
                           <dd>An ID header contained an unrecognized version
                            number.</dd>
                           <dt>#OP_EBADLINK</dt>
                           <dd>We failed to find data we had seen before after
                            seeking.</dd>
                           <dt>#OP_EBADTIMESTAMP</dt>
                           <dd>The first or last timestamp in a link failed
                            basic validity checks.</dd>
                         </dl>
   \return A freshly opened #OggOpusFile, or <code>NULL</code> on error.*/
OP_WARN_UNUSED_RESULT OggOpusFile *op_open_callbacks(void *_source,
 const OpusFileCallbacks *_cb,const unsigned char *_initial_data,
 size_t _initial_bytes,int *_error) OP_ARG_NONNULL(2);

/**Partially open a stream from the given file path.
   \see op_test_callbacks
   \param      _path  The path to the file to open.
   \param[out] _error Returns 0 on success, or a failure code on error.
                      You may pass in <code>NULL</code> if you don't want the
                       failure code.
                      The failure code will be #OP_EFAULT if the file could not
                       be opened, or one of the other failure codes from
                       op_open_callbacks() otherwise.
   \return An #OggOpusFile pointer on success, or <code>NULL</code> on error.*/
OP_WARN_UNUSED_RESULT OggOpusFile *op_test_file(const char *_path,int *_error)
 OP_ARG_NONNULL(1);

/**Partially open a stream from a memory buffer.
   \see op_test_callbacks
   \param      _data  The memory buffer to open.
   \param      _size  The number of bytes in the buffer.
   \param[out] _error Returns 0 on success, or a failure code on error.
                      You may pass in <code>NULL</code> if you don't want the
                       failure code.
                      See op_open_callbacks() for a full list of failure codes.
   \return An #OggOpusFile pointer on success, or <code>NULL</code> on error.*/
OP_WARN_UNUSED_RESULT OggOpusFile *op_test_memory(const unsigned char *_data,
 size_t _size,int *_error);

/**Partially open a stream from a URL.
   \see op_test_callbacks
   \param      _url   The URL to open.
                      Currently only the <file:>, <http:>, and <https:> schemes
                       are supported.
                      Both "http:" and "https:" may be disabled at compile
                       time, in which case opening such URLs will fail.
   \param      _flags The <a href="#url_flags">optional flags</a> to use.
   \param[out] _error Returns 0 on success, or a failure code on error.
                      You may pass in <code>NULL</code> if you don't want the
                       failure code.
                      See op_open_callbacks() for a full list of failure codes.
   \return An #OggOpusFile pointer on success, or <code>NULL</code> on error.*/
OP_WARN_UNUSED_RESULT OggOpusFile *op_test_url(const char *_url,int _flags,
 int *_error) OP_ARG_NONNULL(1);

/**Partially open a stream from a URL using the specified proxy.
   \see op_test_callbacks
   \param      _url        The URL to open.
                           Currently only the <file:>, <http:>, and <https:>
                            schemes are supported.
                           Both "http:" and "https:" may be disabled at compile
                            time, in which case opening such URLs will fail.
   \param      _flags      The <a href="#url_flags">optional flags</a> to use.
   \param      _proxy_host The host of the proxy to connect to.
                           This may be <code>NULL</code> if you do not wish to
                            use a proxy.
                           The proxy information is ignored if \a _url is a
                            <file:> URL.
   \param      _proxy_port The port of the proxy to connect to.
                           This is ignored if \a _proxy_host is
                            <code>NULL</code>.
   \param      _proxy_user The username to use with the specified proxy.
                           This may be <code>NULL</code> if no authorization is
                            required.
                           This is ignored if \a _proxy_host is
                            <code>NULL</code>.
   \param      _proxy_pass The password to use with the specified proxy.
                           This may be <code>NULL</code> if no authorization is
                            required.
                           This is ignored if either \a _proxy_host or
                            \a _proxy_user are <code>NULL</code>.
   \param[out] _error      Returns 0 on success, or a failure code on error.
                           You may pass in <code>NULL</code> if you don't want
                            the failure code.
                           See op_open_callbacks() for a full list of failure
                            codes.
   \return An #OggOpusFile pointer on success, or <code>NULL</code> on error.*/
OP_WARN_UNUSED_RESULT OggOpusFile *op_test_url_with_proxy(const char *_url,
 int _flags,const char *_proxy_host,unsigned _proxy_port,
 const char *_proxy_user,const char *_proxy_pass,int *_error)
 OP_ARG_NONNULL(1);

/**Partially open a stream using the given set of callbacks to access it.
   This tests for Opusness and loads the headers for the first link.
   It does not seek (although it tests for seekability).
   Use op_test_open() to finish opening the stream, or op_free() to dispose of
    it.
   \param _source        The stream to read from (e.g., a <code>FILE *</code>).
   \param _cb            The callbacks with which to access the stream.
                         <code><a href="#op_read_func">read()</a></code> must
                          be implemented.
                         <code><a href="#op_seek_func">seek()</a></code> and
                          <code><a href="#op_tell_func">tell()</a></code> may
                          be <code>NULL</code>, or may always return -1 to
                          indicate a source is unseekable, but if
                          <code><a href="#op_seek_func">seek()</a></code> is
                          implemented and succeeds on a particular source, then
                          <code><a href="#op_tell_func">tell()</a></code> must
                          also.
                         <code><a href="#op_close_func">close()</a></code> may
                          be <code>NULL</code>, but if it is not, it will be
                          called when the #OggOpusFile is destroyed by
                          op_free().
                         It will not be called if op_open_callbacks() fails
                          with an error.
   \param _initial_data  An initial buffer of data from the start of the
                          stream.
                         Applications can read some number of bytes from the
                          start of the stream to help identify this as an Opus
                          stream, and then provide them here to allow the
                          stream to be tested more thoroughly, even if it is
                          unseekable.
   \param _initial_bytes The number of bytes in \a _initial_data.
                         If the stream is seekable, its current position (as
                          reported by
                          <code><a href="#opus_tell_func">tell()</a></code>
                          at the start of this function) must be equal to
                          \a _initial_bytes.
                         Otherwise, seeking to absolute positions will
                          generate inconsistent results.
   \param[out] _error    Returns 0 on success, or a failure code on error.
                         You may pass in <code>NULL</code> if you don't want
                          the failure code.
                         See op_open_callbacks() for a full list of failure
                          codes.
   \return A freshly opened #OggOpusFile, or <code>NULL</code> on error.*/
OP_WARN_UNUSED_RESULT OggOpusFile *op_test_callbacks(void *_source,
 const OpusFileCallbacks *_cb,const unsigned char *_initial_data,
 size_t _initial_bytes,int *_error) OP_ARG_NONNULL(2);

/**Finish opening a stream partially opened with op_test_callbacks() or one of
    the associated convenience functions.
   If this function fails, you are still responsible for freeing the
    #OggOpusFile with op_free().
   \param _of The #OggOpusFile to finish opening.
   \return 0 on success, or a negative value on error.
   \retval #OP_EREAD         An underlying read, seek, or tell operation failed
                              when it should have succeeded.
   \retval #OP_EFAULT        There was a memory allocation failure, or an
                              internal library error.
   \retval #OP_EIMPL         The stream used a feature that is not implemented,
                              such as an unsupported channel family.
   \retval #OP_EINVAL        The stream was not partially opened with
                              op_test_callbacks() or one of the associated
                              convenience functions.
   \retval #OP_ENOTFORMAT    The stream contained a link that did not have any
                              logical Opus streams in it.
   \retval #OP_EBADHEADER    A required header packet was not properly
                              formatted, contained illegal values, or was
                              missing altogether.
   \retval #OP_EVERSION      An ID header contained an unrecognized version
                              number.
   \retval #OP_EBADLINK      We failed to find data we had seen before after
                              seeking.
   \retval #OP_EBADTIMESTAMP The first or last timestamp in a link failed basic
                              validity checks.*/
int op_test_open(OggOpusFile *_of) OP_ARG_NONNULL(1);

/**Release all memory used by an #OggOpusFile.
   \param _of The #OggOpusFile to free.*/
void op_free(OggOpusFile *_of);

/*@}*/
/*@}*/

/**\defgroup stream_info Stream Information*/
/*@{*/
/**\name Functions for obtaining information about streams

   These functions allow you to get basic information about a stream, including
    seekability, the number of links (for chained streams), plus the size,
    duration, bitrate, header parameters, and meta information for each link
    (or, where available, the stream as a whole).
   Some of these (size, duration) are only available for seekable streams.
   You can also query the current stream position, link, and playback time,
    and instantaneous bitrate during playback.

   Some of these functions may be used successfully on the partially open
    streams returned by op_test_callbacks() or one of the associated
    convenience functions.
   Their documention will indicate so explicitly.*/
/*@{*/

/**Returns the number of links in this chained stream.
   This function may be called on partially-opened streams, but it will always
    return 1.
   The actual number of links is not known until the stream is fully opened.
   \param _of The #OggOpusFile from which to retrieve the link count.
   \return For seekable sources, this returns the total number of links in the
            whole stream.
           For unseekable sources, this always returns 1.*/
int op_link_count(OggOpusFile *_of) OP_ARG_NONNULL(1);

/**Returns whether or not the data source being read is seekable.
   This is true if
   <ol>
   <li>The <code><a href="#op_seek_func">seek()</a></code> and
    <code><a href="#op_tell_func">tell()</a></code> callbacks are both
    non-<code>NULL</code>,</li>
   <li>The <code><a href="#op_seek_func">seek()</a></code> callback was
    successfully executed at least once, and</li>
   <li>The <code><a href="#op_tell_func">tell()</a></code> callback was
    successfully able to report the position indicator afterwards.</li>
   </ol>
   This function may be called on partially-opened streams.
   \param _of The #OggOpusFile whose seekable status is to be returned.
   \return A non-zero value if seekable, and 0 if unseekable.*/
int op_seekable(OggOpusFile *_of) OP_ARG_NONNULL(1);

/**Get the serial number of the given link in a (possibly-chained) Ogg Opus
    stream.
   This function may be called on partially-opened streams, but it will always
    return the serial number of the Opus stream in the first link.
   \param _of The #OggOpusFile from which to retrieve the serial number.
   \param _li The index of the link whose serial number should be retrieved.
              Use a negative number to get the serial number of the current
               link.
   \return The serial number of the given link.
           If \a _li is greater than the total number of links, this returns
            the serial number of the last link.
           If the source is not seekable, this always returns the serial number
            of the current link.*/
opus_uint32 op_serialno(OggOpusFile *_of,int _li) OP_ARG_NONNULL(1);

/**Get the channel count of the given link in a (possibly-chained) Ogg Opus
    stream.
   This is equivalent to <code>op_head(_of,_li)->channel_count</code>, but
    is provided for convenience.
   This function may be called on partially-opened streams, but it will always
    return the serial number of the Opus stream in the first link.
   \param _of The #OggOpusFile from which to retrieve the channel count.
   \param _li The index of the link whose channel count should be retrieved.
              Use a negative number to get the channel count of the current
               link.
   \return The channel count of the given link.
           If \a _li is greater than the total number of links, this returns
            the channel count of the last link.
           If the source is not seekable, this always returns the channel count
            of the current link.*/
int op_channel_count(OggOpusFile *_of,int _li) OP_ARG_NONNULL(1);

/**Get the total (compressed) size of the stream, or of an individual link in
    a (possibly-chained) Ogg Opus stream, including all headers and Ogg muxing
    overhead.
   \param _of The #OggOpusFile from which to retrieve the compressed size.
   \param _li The index of the link whose compressed size should be computed.
              Use a negative number to get the compressed size of the entire
               stream.
   \return The compressed size of the entire stream if \a _li is negative, the
            compressed size of link \a _li if it is non-negative, or a negative
            value on error.
           The compressed size of the entire stream may be smaller than that
            of the underlying source if trailing garbage was detected in the
            file.
   \retval #OP_EINVAL The source is not seekable (so we can't know the length),
                       \a _li wasn't less than the total number of links in
                       the stream, or the stream was only partially open.*/
opus_int64 op_raw_total(OggOpusFile *_of,int _li) OP_ARG_NONNULL(1);

/**Get the total PCM length (number of samples at 48 kHz) of the stream, or of
    an individual link in a (possibly-chained) Ogg Opus stream.
   Users looking for <code>op_time_total()</code> should use op_pcm_total()
    instead.
   Because timestamps in Opus are fixed at 48 kHz, there is no need for a
    separate function to convert this to seconds (and leaving it out avoids
    introducing floating point to the API, for those that wish to avoid it).
   \param _of The #OggOpusFile from which to retrieve the PCM offset.
   \param _li The index of the link whose PCM length should be computed.
              Use a negative number to get the PCM length of the entire stream.
   \return The PCM length of the entire stream if \a _li is negative, the PCM
            length of link \a _li if it is non-negative, or a negative value on
            error.
   \retval #OP_EINVAL The source is not seekable (so we can't know the length),
                       \a _li wasn't less than the total number of links in
                       the stream, or the stream was only partially open.*/
ogg_int64_t op_pcm_total(OggOpusFile *_of,int _li) OP_ARG_NONNULL(1);

/**Get the ID header information for the given link in a (possibly chained) Ogg
    Opus stream.
   This function may be called on partially-opened streams, but it will always
    return the ID header information of the Opus stream in the first link.
   \param _of The #OggOpusFile from which to retrieve the ID header
               information.
   \param _li The index of the link whose ID header information should be
               retrieved.
              Use a negative number to get the ID header information of the
               current link.
              For an unseekable stream, \a _li is ignored, and the ID header
               information for the current link is always returned, if
               available.
   \return The contents of the ID header for the given link.*/
const OpusHead *op_head(OggOpusFile *_of,int _li) OP_ARG_NONNULL(1);

/**Get the comment header information for the given link in a (possibly
    chained) Ogg Opus stream.
   \param _of The #OggOpusFile from which to retrieve the comment header
               information.
   \param _li The index of the link whose comment header information should be
               retrieved.
              Use a negative number to get the comment header information of
               the current link.
              For an unseekable stream, \a _li is ignored, and the comment
               header information for the current link is always returned, if
               available.
   \return The contents of the comment header for the given link, or
            <code>NULL</code> if either the stream was only partially open or
            this is an unseekable stream that encountered an invalid link.*/
const OpusTags *op_tags(OggOpusFile *_of,int _li) OP_ARG_NONNULL(1);

/**Retrieve the index of the current link.
   This is the link that produced the data most recently read by
    op_read_float() or its associated functions, or, after a seek, the link
    that the seek target landed in.
   Reading more data may advance the link index (even on the first read after a
    seek).
   \param _of The #OggOpusFile from which to retrieve the current link index.
   \return The index of the current link on success, or a negative value on
            failture.
           For seekable streams, this is a number between 0 and the value
            returned by op_link_count().
           For unseekable streams, this value starts at 0 and increments by one
            each time a new link is encountered (even though op_link_count()
            always returns 1).
   \retval #OP_EINVAL The stream was not fully open.*/
int op_current_link(OggOpusFile *_of) OP_ARG_NONNULL(1);

/**Computes the bitrate for a given link in a (possibly chained) Ogg Opus
    stream.
   The stream must be seekable to compute the bitrate.
   For unseekable streams, use op_bitrate_instant() to get periodic estimates.
   \param _of The #OggOpusFile from which to retrieve the bitrate.
   \param _li The index of the link whose bitrate should be computed.
              USe a negative number to get the bitrate of the whole stream.
   \return The bitrate on success, or a negative value on error.
   \retval #OP_EINVAL The stream was not fully open, the stream was not
                       seekable, or \a _li was larger than the number of
                       links.*/
opus_int32 op_bitrate(OggOpusFile *_of,int _li) OP_ARG_NONNULL(1);

/**Compute the instantaneous bitrate, measured as the ratio of bits to playable
    samples decoded since a) the last call to op_bitrate_instant(), b) the last
    seek, or c) the start of playback, whichever was most recent.
   This will spike somewhat after a seek or at the start/end of a chain
    boundary, as pre-skip, pre-roll, and end-trimming causes samples to be
    decoded but not played.
   \param _of The #OggOpusFile from which to retrieve the bitrate.
   \return The bitrate, in bits per second, or a negative value on error.
   \retval #OP_EFALSE No data has been decoded since any of the events
                       described above.
   \retval #OP_EINVAL The stream was not fully open.*/
opus_int32 op_bitrate_instant(OggOpusFile *_of) OP_ARG_NONNULL(1);

/**Obtain the current value of the position indicator for \a _of.
   \param _of The #OggOpusFile from which to retrieve the position indicator.
   \return The byte position that is currently being read from.
   \retval #OP_EINVAL The stream was not fully open.*/
opus_int64 op_raw_tell(OggOpusFile *_of) OP_ARG_NONNULL(1);

/**Obtain the PCM offset of the next sample to be read.
   If the stream is not properly timestamped, this might not increment by the
    proper amount between reads, or even return monotonically increasing
    values.
   \param _of The #OggOpusFile from which to retrieve the PCM offset.
   \return The PCM offset of the next sample to be read.
   \retval #OP_EINVAL The stream was not fully open.*/
ogg_int64_t op_pcm_tell(OggOpusFile *_of) OP_ARG_NONNULL(1);

/*@}*/
/*@}*/

/**\defgroup stream_seeking Seeking*/
/*@{*/
/**\name Functions for seeking in Opus streams

   These functions let you seek in Opus streams, if the underlying source
    support it.
   Seeking is implemented for all built-in stream I/O routines, though some
    individual sources may not be seekable (pipes, live HTTP streams, or HTTP
    streams from a server that does not support <code>Range</code> requests).

   op_raw_seek() is the fastest: it is guaranteed to perform at most one
    physical seek, but, since the target is a byte position, makes no guarantee
    how close to a given time it will come.
   op_pcm_seek() provides sample-accurate seeking.
   The number of physical seeks it requires is still quite small (often 1 or
    2, even in highly variable bitrate streams).

   Seeking in Opus requires decoding some pre-roll amount before playback to
    allow the internal state to converge (as if recovering from packet loss).
   This is handled internally by <tt>libopusfile</tt>, but means there is
    little extra overhead for decoding up to the exact position requested
    (since it must decode some amount of audio anyway).
   It also means that decoding after seeking may not return exactly the same
    values as would be obtained by decoding the stream straight through.
   However, such differences are expected to be smaller than the loss
    introduced by Opus's lossy compression.*/
/*@{*/

/**Seek to a byte offset relative to the <b>compressed</b> data.
   This also scans packets to update the PCM cursor.
   It will cross a logical bitstream boundary, but only if it can't get any
    packets out of the tail of the link to which it seeks.
   \param _of          The #OggOpusFile in which to seek.
   \param _byte_offset The byte position to seek to.
   \return 0 on success, or a negative error code on failure.
   \retval #OP_EREAD    The seek failed.
   \retval #OP_EINVAL   The stream was not fully open, or the target was
                         outside the valid range for the stream.
   \retval #OP_ENOSEEK  This stream is not seekable.
   \retval #OP_EBADLINK Failed to initialize a decoder for a stream for an
                         unknown reason.*/
int op_raw_seek(OggOpusFile *_of,opus_int64 _byte_offset) OP_ARG_NONNULL(1);

/**Seek to a page preceding the specified PCM offset, such that decoding will
    quickly arrive at the requested position.
   This is faster than sample-granularity seeking because it doesn't do the
    last bit of decode to find a specific sample.
   \param _of         The #OggOpusFile in which to seek.
   \param _pcm_offset The PCM offset to seek to.
                      This is in samples at 48 kHz relative to the start of the
                       stream.
   \return 0 on success, or a negative value on error.
   \retval #OP_EREAD   The seek failed.
   \retval #OP_EINVAL  The stream was not fully open, or the target was outside
                        the valid range for the stream.
   \retval #OP_ENOSEEK This stream is not seekable.*/
int op_pcm_seek_page(OggOpusFile *_of,ogg_int64_t _pcm_offset)
 OP_ARG_NONNULL(1);

/**Seek to the specified PCM offset, such that decoding will begin at exactly
    the requested position.
   \param _of         The #OggOpusFile in which to seek.
   \param _pcm_offset The PCM offset to seek to.
                      This is in samples at 48 kHz relative to the start of the
                       stream.
   \return 0 on success, or a negative value on error.
   \retval #OP_EREAD   The seek failed.
   \retval #OP_EINVAL  The stream was not fully open, or the target was outside
                        the valid range for the stream.
   \retval #OP_ENOSEEK This stream is not seekable.*/
int op_pcm_seek(OggOpusFile *_of,ogg_int64_t _pcm_offset) OP_ARG_NONNULL(1);

/*@}*/
/*@}*/

/**\defgroup stream_decoding Decoding*/
/*@{*/
/**\name Functions for decoding audio data

   These functions retrieve actual decoded audio data from the stream.
   The general functions, op_read() and op_read_float() return 16-bit or
    floating-point output, both using native endian ordering.
   The number of channels returned can change from link to link in a chained
    stream.
   There are special functions, op_read_stereo() and op_read_float_stereo(),
    which always output two channels, to simplify applications which do not
    wish to handle multichannel audio.
   These downmix multichannel files to two channels, so they can always return
    samples in the same format for every link in a chained file.

   If the rest of your audio processing chain can handle floating point, those
    routines should be preferred, as floating point output avoids introducing
    clipping and other issues which might be avoided entirely if, e.g., you
    scale down the volume at some other stage.
   However, if you intend to direct consume 16-bit samples, the conversion in
    <tt>libopusfile</tt> provides noise-shaping dithering API.

   <tt>libopusfile</tt> can also be configured at compile time to use the
    fixed-point <tt>libopus</tt> API.
   If so, the floating-point API may also be disabled.
   In that configuration, nothing in <tt>libopusfile</tt> will use any
    floating-point operations, to simplify support on devices without an
    adequate FPU.*/
/*@{*/

/**Reads more samples from the stream.
   \note Although \a _buf_size must indicate the total number of values that
    can be stored in \a _pcm, the return value is the number of samples
    <em>per channel</em>.
   This is done because
   <ol>
   <li>The channel count cannot be known a prior (reading more samples might
        advance us into the next link, with a different channel count), so
        \a _buf_size cannot also be in units of samples per channel,</li>
   <li>Returning the samples per channel matches the <code>libopus</code> API
        as closely as we're able,</li>
   <li>Returning the total number of values instead of samples per channel
        would mean the caller would need a division to compute the samples per
        channel, and might worry about the possibility of getting back samples
        for some channels and not others, and</li>
   <li>This approach is relatively fool-proof: if an application passes too
        small a value to \a _buf_size, they will simply get fewer samples back,
        and if they assume the return value is the total number of values, then
        they will simply read too few (rather than reading too many and going
        off the end of the buffer).</li>
   </ol>
   \param      _of       The #OggOpusFile from which to read.
   \param[out] _pcm      A buffer in which to store the output PCM samples, as
                          signed native-endian 16-bit values with a nominal
                          range of <code>[-32768,32767)</code>.
                         Multiple channels are interleaved using the
                          <a href="http://www.xiph.org/vorbis/doc/Vorbis_I_spec.html#x1-800004.3.9">Vorbis
                          channel ordering</a>.
                         This must have room for at least \a _buf_size values.
   \param      _buf_size The number of values that can be stored in \a _pcm.
                         It is reccommended that this be large enough for at
                          least 120 ms of data at 48 kHz per channel (5760
                          values per channel).
                         Smaller buffers will simply return less data, possibly
                          consuming more memory to buffer the data internally.
                         If less than \a _buf_size values are returned,
                          <tt>libopusfile</tt> makes no guarantee that the
                          remaining data in \a _pcm will be unmodified.
   \param[out] _li       The index of the link this data was decoded from.
                         You may pass <code>NULL</code> if you do not need this
                          information.
                         If this function fails (returning a negative value),
                          this parameter is left unset.
   \return The number of samples read per channel on success, or a negative
            value on failure.
           The channel count can be retrieved on success by calling
            <code>op_head(_of,*_li)</code>.
           The number of samples returned may be 0 if the buffer was too small
            to store even a single sample for all channels, or if end-of-file
            was reached.
           The list of possible failure codes follows.
           Most of them can only be returned by unseekable, chained streams
            that encounter a new link.
   \retval #OP_EFAULT        An internal memory allocation failed.
   \retval #OP_EIMPL         An unseekable stream encountered a new link that
                              used a feature that is not implemented, such as
                              an unsupported channel family.
   \retval #OP_EINVAL        The stream was not fully open.
   \retval #OP_ENOTFORMAT    An unseekable stream encountered a new link that
                              did not have any logical Opus streams in it.
   \retval #OP_EBADHEADER    An unseekable stream encountered a new link with a
                              required header packet that was not properly
                              formatted, contained illegal values, or was
                              missing altogether.
   \retval #OP_EVERSION      An unseekable stream encountered a new link with
                              an ID header that contained an unrecognized
                              version number.
   \retval #OP_EBADPACKET    Failed to properly decode the next packet.
   \retval #OP_EBADTIMESTAMP An unseekable stream encountered a new link with
                              a starting timestamp that failed basic validity
                              checks.*/
OP_WARN_UNUSED_RESULT int op_read(OggOpusFile *_of,
 opus_int16 *_pcm,int _buf_size,int *_li) OP_ARG_NONNULL(1);

/**Reads more samples from the stream.
   \note Although \a _buf_size must indicate the total number of values that
    can be stored in \a _pcm, the return value is the number of samples
    <em>per channel</em>.
   <ol>
   <li>The channel count cannot be known a prior (reading more samples might
        advance us into the next link, with a different channel count), so
        \a _buf_size cannot also be in units of samples per channel,</li>
   <li>Returning the samples per channel matches the <code>libopus</code> API
        as closely as we're able,</li>
   <li>Returning the total number of values instead of samples per channel
        would mean the caller would need a division to compute the samples per
        channel, and might worry about the possibility of getting back samples
        for some channels and not others, and</li>
   <li>This approach is relatively fool-proof: if an application passes too
        small a value to \a _buf_size, they will simply get fewer samples back,
        and if they assume the return value is the total number of values, then
        they will simply read too few (rather than reading too many and going
        off the end of the buffer).</li>
   </ol>
   \param      _of       The #OggOpusFile from which to read.
   \param[out] _pcm      A buffer in which to store the output PCM samples as
                          signed floats with a nominal range of
                          <code>[-1.0,1.0]</code>.
                         Multiple channels are interleaved using the
                          <a href="http://www.xiph.org/vorbis/doc/Vorbis_I_spec.html#x1-800004.3.9">Vorbis
                          channel ordering</a>.
                         This must have room for at least \a _buf_size floats.
   \param      _buf_size The number of floats that can be stored in \a _pcm.
                         It is reccommended that this be large enough for at
                          least 120 ms of data at 48 kHz per channel (5760
                          samples per channel).
                         Smaller buffers will simply return less data, possibly
                          consuming more memory to buffer the data internally.
                         If less than \a _buf_size values are returned,
                          <tt>libopusfile</tt> makes no guarantee that the
                          remaining data in \a _pcm will be unmodified.
   \param[out] _li       The index of the link this data was decoded from.
                         You may pass <code>NULL</code> if you do not need this
                          information.
                         If this function fails (returning a negative value),
                          this parameter is left unset.
   \return The number of samples read per channel on success, or a negative
            value on failure.
           The channel count can be retrieved on success by calling
            <code>op_head(_of,*_li)</code>.
           The number of samples returned may be 0 if the buffer was too small
            to store even a single sample for all channels, or if end-of-file
            was reached.
           The list of possible failure codes follows.
           Most of them can only be returned by unseekable, chained streams
            that encounter a new link.
   \retval #OP_EFAULT        An internal memory allocation failed.
   \retval #OP_EIMPL         An unseekable stream encountered a new link that
                              used a feature that is not implemented, such as
                              an unsupported channel family.
   \retval #OP_EINVAL        The stream was not fully open.
   \retval #OP_ENOTFORMAT    An unseekable stream encountered a new link that
                              did not have any logical Opus streams in it.
   \retval #OP_EBADHEADER    An unseekable stream encountered a new link with a
                              required header packet that was not properly
                              formatted, contained illegal values, or was
                              missing altogether.
   \retval #OP_EVERSION      An unseekable stream encountered a new link with
                              an ID header that contained an unrecognized
                              version number.
   \retval #OP_EBADPACKET    Failed to properly decode the next packet.
   \retval #OP_EBADTIMESTAMP An unseekable stream encountered a new link with
                              a starting timestamp that failed basic validity
                              checks.*/
OP_WARN_UNUSED_RESULT int op_read_float(OggOpusFile *_of,
 float *_pcm,int _buf_size,int *_li) OP_ARG_NONNULL(1);

/**Reads more samples from the stream and downmixes to stereo, if necessary.
   This function is intended for simple players that want a uniform output
    format, even if the channel count changes between links in a chained
    stream.
   \note \a _buf_size indicates the total number of values that can be stored
    in \a _pcm, while the return value is the number of samples <em>per
    channel</em>, even though the channel count is known, for consistency with
    op_read().
   \param      _of       The #OggOpusFile from which to read.
   \param[out] _pcm      A buffer in which to store the output PCM samples, as
                          signed native-endian 16-bit values with a nominal
                          range of <code>[-32768,32767)</code>.
                         The left and right channels are interleaved in the
                          buffer.
                         This must have room for at least \a _buf_size values.
   \param      _buf_size The number of values that can be stored in \a _pcm.
                         It is reccommended that this be large enough for at
                          least 120 ms of data at 48 kHz per channel (11520
                          values total).
                         Smaller buffers will simply return less data, possibly
                          consuming more memory to buffer the data internally.
                         If less than \a _buf_size values are returned,
                          <tt>libopusfile</tt> makes no guarantee that the
                          remaining data in \a _pcm will be unmodified.
   \return The number of samples read per channel on success, or a negative
            value on failure.
           The number of samples returned may be 0 if the buffer was too small
            to store even a single sample for both channels, or if end-of-file
            was reached.
           The list of possible failure codes follows.
           Most of them can only be returned by unseekable, chained streams
            that encounter a new link.
   \retval #OP_EFAULT        An internal memory allocation failed.
   \retval #OP_EIMPL         An unseekable stream encountered a new link that
                              used a feature that is not implemented, such as
                              an unsupported channel family.
   \retval #OP_EINVAL        The stream was not fully open.
   \retval #OP_ENOTFORMAT    An unseekable stream encountered a new link that
                              did not have any logical Opus streams in it.
   \retval #OP_EBADHEADER    An unseekable stream encountered a new link with a
                              required header packet that was not properly
                              formatted, contained illegal values, or was
                              missing altogether.
   \retval #OP_EVERSION      An unseekable stream encountered a new link with
                              an ID header that contained an unrecognized
                              version number.
   \retval #OP_EBADPACKET    Failed to properly decode the next packet.
   \retval #OP_EBADTIMESTAMP An unseekable stream encountered a new link with
                              a starting timestamp that failed basic validity
                              checks.*/
OP_WARN_UNUSED_RESULT int op_read_stereo(OggOpusFile *_of,
 opus_int16 *_pcm,int _buf_size) OP_ARG_NONNULL(1);

/**Reads more samples from the stream and downmixes to stereo, if necessary.
   This function is intended for simple players that want a uniform output
    format, even if the channel count changes between links in a chained
    stream.
   \note \a _buf_size indicates the total number of values that can be stored
    in \a _pcm, while the return value is the number of samples <em>per
    channel</em>, even though the channel count is known, for consistency with
    op_read_float().
   \param      _of       The #OggOpusFile from which to read.
   \param[out] _pcm      A buffer in which to store the output PCM samples, as
                          signed floats with a nominal range of
                          <code>[-1.0,1.0]</code>.
                         The left and right channels are interleaved in the
                          buffer.
                         This must have room for at least \a _buf_size values.
   \param      _buf_size The number of values that can be stored in \a _pcm.
                         It is reccommended that this be large enough for at
                          least 120 ms of data at 48 kHz per channel (11520
                          values total).
                         Smaller buffers will simply return less data, possibly
                          consuming more memory to buffer the data internally.
                         If less than \a _buf_size values are returned,
                          <tt>libopusfile</tt> makes no guarantee that the
                          remaining data in \a _pcm will be unmodified.
   \return The number of samples read per channel on success, or a negative
            value on failure.
           The number of samples returned may be 0 if the buffer was too small
            to store even a single sample for both channels, or if end-of-file
            was reached.
           The list of possible failure codes follows.
           Most of them can only be returned by unseekable, chained streams
            that encounter a new link.
   \retval #OP_EFAULT        An internal memory allocation failed.
   \retval #OP_EIMPL         An unseekable stream encountered a new link that
                              used a feature that is not implemented, such as
                              an unsupported channel family.
   \retval #OP_EINVAL        The stream was not fully open.
   \retval #OP_ENOTFORMAT    An unseekable stream encountered a new link that
                              that did not have any logical Opus streams in it.
   \retval #OP_EBADHEADER    An unseekable stream encountered a new link with a
                              required header packet that was not properly
                              formatted, contained illegal values, or was
                              missing altogether.
   \retval #OP_EVERSION      An unseekable stream encountered a new link with
                              an ID header that contained an unrecognized
                              version number.
   \retval #OP_EBADPACKET    Failed to properly decode the next packet.
   \retval #OP_EBADTIMESTAMP An unseekable stream encountered a new link with
                              a starting timestamp that failed basic validity
                              checks.*/
OP_WARN_UNUSED_RESULT int op_read_float_stereo(OggOpusFile *_of,
 float *_pcm,int _buf_size) OP_ARG_NONNULL(1);

/*@}*/
/*@}*/

# if OP_GNUC_PREREQ(4,0)
#  pragma GCC visibility pop
# endif

# if defined(__cplusplus)
}
# endif

#endif
