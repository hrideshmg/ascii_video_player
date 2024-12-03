#include <stdio.h>
#include <stdlib.h>
#include <libavformat/avformat.h> // handle formats
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h> // scaling and converting pixel formats
#include <sys/stat.h> // for using mkdir

#define STB_IMAGE_WRITE_IMPLEMENTATION // include the image writing functions from stb_image_write.h
#include "stb_image_write.h"

int main(int argc, char *argv[]) // array of character pointers as arguments
{
    char *input_filename = "epic_video.mp4";
    char *output_directory = "extracted_frames";

    mkdir(output_directory, 0777);

    AVFormatContext *format_ctx = NULL;
    avformat_open_input(&format_ctx, input_filename, NULL, NULL); // automatically detects the format
    avformat_find_stream_info(format_ctx, NULL); // info about audio and video streams

    int video_stream_index = -1; // to tell that no video stream is found when initialized
    AVCodecParameters *codec_params = NULL; // structure to store info about encoded data
    for (unsigned int i = 0; i < format_ctx->nb_streams; i++){
        if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream_index = i;
            codec_params = format_ctx->streams[i]->codecpar;
            break;
        }
    }
    const AVCodec *codec = avcodec_find_decoder(codec_params->codec_id); // find the codec based on id
    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec); // codec context which tells how to decode the video
    avcodec_parameters_to_context(codec_ctx, codec_params);// copy from ctx to params
    avcodec_open2(codec_ctx, codec, NULL);// opens the codec to decode

    int total_frames = 0;
    AVRational frame_rate = format_ctx->streams[video_stream_index]->avg_frame_rate;
    int64_t duration = format_ctx->duration; //duration in AV_TIME_BASE (microseconds)

    if(frame_rate.num > 0 && frame_rate.den > 0){
        double duration_in_seconds = (double)duration / AV_TIME_BASE; //converting to seconds
        total_frames = (int)(duration_in_seconds * (double)frame_rate.num / (double)frame_rate.den);//num = no. of frames in a time period, den = time period in which frames occur
    }else{
        total_frames = (int)(duration / AV_TIME_BASE * 30);
    }
    printf("Total frames in video: %d\n", total_frames);

    AVFrame *frame = av_frame_alloc();// frame struct to store the decoded frame data
    AVPacket *packet = av_packet_alloc(); // same but encoded frame data
    int frame_number = 0;
    while (av_read_frame(format_ctx, packet) >= 0){
        if (packet->stream_index == video_stream_index){
            avcodec_send_packet(codec_ctx, packet);
            while (avcodec_receive_frame(codec_ctx, frame) == 0){
                struct SwsContext *sws_ctx = sws_getContext(
                    frame->width,
                    frame->height,
                    codec_ctx->pix_fmt,
                    frame->width,
                    frame->height,
                    AV_PIX_FMT_RGB24,
                    SWS_BILINEAR,
                    NULL,
                    NULL,
                    NULL);

                uint8_t *rgb_data = malloc(frame->width * frame->height * 3); // allocate memory for rgb data of frame
                sws_scale(sws_ctx,
                          (const uint8_t *const *)frame->data,
                          frame->linesize,
                          0,
                          frame->height,
                          &rgb_data,
                          (int[]){frame->width * 3}); // convert the frame to rgb format using context from earlier

                char filename[100];
                sprintf(filename, "%s/frame_%03d.jpg", output_directory, frame_number++);
                stbi_write_jpg(filename, frame->width, frame->height, 3 /* channels */, rgb_data, 90); // save rgb data as jpg image with 90% quality

                free(rgb_data);
                sws_freeContext(sws_ctx);
            }
        }
        av_packet_unref(packet);
    }
    av_packet_free(&packet);
    av_frame_free(&frame);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&format_ctx);

    return 0;
}