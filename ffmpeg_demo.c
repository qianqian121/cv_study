#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>

int img_convert(AVPicture* dst, enum AVPixelFormat dst_pix_fmt, AVPicture* src, enum AVPixelFormat pix_fmt, int width, int height)
{
   int av_log = av_log_get_level();
   av_log_set_level(AV_LOG_QUIET);
   struct SwsContext *img_convert_ctx = sws_getContext(width, height, pix_fmt, width, height, dst_pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
   int result = sws_scale(img_convert_ctx, src->data, src->linesize, 0, height, dst->data, dst->linesize);
   sws_freeContext(img_convert_ctx);
   av_log_set_level(av_log);
   return result;
}

int main(int argc, char *argv[]) {
    av_register_all();

    AVFormatContext *pFormatCtx;

    // Open Video file
    if (avformat_open_input(&pFormatCtx, argv[1], NULL, NULL) != 0) {
        return -1;  // Couldn't open file
    }

    // Retrieve stream information
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        return -1;  // Couldn't find stream information
    }

    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, argv[1], 0);

    int i;
    AVCodecContext *pCodecCtx;

    // Find the first video stream
    int videoStream = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }

    if (videoStream == -1) {
        return -1;  // Didn't find a video stream
    }

    // Get a pointer to the codec context for the video stream
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;

    AVCodec *pCodec;

    // Find the decoder for the video stream
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1;  // Codec not found
    }

    // Open codec
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        return -1;  // Could not open codec
    }

    AVFrame *pFrame;
    AVFrame *pFrameRGB;

    // Allocate video frame
    pFrame = av_frame_alloc();

    // Allocate an AVFrame structure
    pFrameRGB = av_frame_alloc();
    if (pFrameRGB == NULL) {
        return -1;
    }

    uint8_t *buffer;
    int numBytes;
    // Determine required buffer size and allocate buffer
    numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width,
                                  pCodecCtx->height);
    buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
    avpicture_fill((AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_RGB24,
                    pCodecCtx->width, pCodecCtx->height);

    int frameFinished;
    AVPacket packet;

    i=0;
    while(av_read_frame(pFormatCtx, &packet)>=0) {
      // Is this a packet from the video stream?
      if(packet.stream_index==videoStream) {
        // Decode video frame
        avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished,
                             &packet);

        // Did we get a video frame?
        if(frameFinished) {
        // Convert the image from its native format to RGB
            img_convert((AVPicture *)pFrameRGB, AV_PIX_FMT_RGB24,
                (AVPicture*)pFrame, pCodecCtx->pix_fmt,
                pCodecCtx->width, pCodecCtx->height);

            // Save the frame to disk
//            if(++i<=5)
//              SaveFrame(pFrameRGB, pCodecCtx->width,
//                        pCodecCtx->height, i);
        }
      }

      // Free the packet that was allocated by av_read_frame
      av_free_packet(&packet);
    }

    // Free the RGB image
    av_free(buffer);
    av_free(pFrameRGB);

    // Free the YUV frame
    av_free(pFrame);

    // Close the codec
    avcodec_close(pCodecCtx);

    // Close the video file
    avformat_close_input(&pFormatCtx);
    return 0;
}