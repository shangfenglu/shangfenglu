#include "encoder.h"
#include "bitmap.h"
#include "debug_msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

//写入一帧
static int write_frame(struct encoder* encoder,int type)
{
	int ret = 0;
	AVCodecContext* codec_ctx = encoder->codec_ctx;
	AVFrame* frame = (1 == type) ? encoder->yuv_frame : encoder->last_frame;
	ret = avcodec_send_frame(codec_ctx, frame);
	if (ret < 0) {
		printf("avcodec_send_frame failed!\n");
		return -1;
	}

	while (ret >= 0) {
		ret = avcodec_receive_packet(codec_ctx, &(encoder->packet));
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			return 0;
		}
		else if (ret < 0) {
			printf("receiving packet failed\n");
			return -1;
		}
		av_packet_rescale_ts(&(encoder->packet), encoder->codec_ctx->time_base, encoder->video_stream->time_base);
		encoder->packet.stream_index = encoder->video_stream->index;
		av_interleaved_write_frame(encoder->ofmt_ctx, &(encoder->packet));
	}

	return 0;
}

struct encoder* get_encoder()
{
    struct encoder* encoder = (struct encoder*)calloc(1, sizeof(struct encoder));
    if (!encoder) {
        DB_WAR("calloc encoder failed!");
        return NULL;
    }
    encoder->codec = NULL;
    encoder->codec_ctx = NULL;
    encoder->ofmt_ctx = NULL;
    encoder->sws_ctx = NULL;
    encoder->yuv_frame = NULL;
    encoder->last_frame = NULL;
    encoder->begin_timestamp = 0;
    encoder->timestamp = 0;
    encoder->fps = 0;

    return encoder;
}

void free_encoder(struct encoder* encoder)
{
    if (encoder) {
        free(encoder);
        encoder = NULL;
    }
}

int init_codec(struct encoder* encoder,int width, int height, int fps)
{
    DB_INFO("init_codec width = %d, height = %d, fps = %d", width, height, fps);
    int ret = 0;
    encoder->codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!encoder->codec) {
        DB_WAR("avcodec_find_encoder AV_CODEC_ID_H264 failed!");
        return -1;
    }

    encoder->codec_ctx = avcodec_alloc_context3(encoder->codec);
    if (!encoder->codec_ctx) {
        DB_WAR("avcodec_alloc_context3 failed!");
        return -1;
    }

    encoder->codec_ctx->bit_rate = 4000000;
    encoder->codec_ctx->width = width;
    encoder->codec_ctx->height = height;
    encoder->codec_ctx->time_base.num = 1;
    encoder->codec_ctx->time_base.den = fps;
    encoder->codec_ctx->gop_size = 50;
    encoder->codec_ctx->max_b_frames = 0;
    encoder->codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    encoder->codec_ctx->codec_id = AV_CODEC_ID_H264;
    encoder->codec_ctx->thread_count = 1;
    encoder->codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    encoder->pts = 0;
    encoder->fps = fps;
    
    if(encoder->codec_ctx->priv_data){
        av_opt_set(encoder->codec_ctx->priv_data, "tune", "fastdecode", 0);
        av_opt_set(encoder->codec_ctx->priv_data, "tune", "zerolatency", 0);
        av_opt_set(encoder->codec_ctx->priv_data, "qp", "30", 0);
        av_opt_set(encoder->codec_ctx->priv_data, "preset", "ultrafast", 0);
    }else{
        DB_INFO("av_opt_set failed!");
    }

    ret = avcodec_open2(encoder->codec_ctx, encoder->codec, NULL);
    if (ret < 0) {
        DB_WAR("avcodec_open2 failed!");
        avcodec_free_context(&encoder->codec_ctx);
        return -1;
    }

    return 0;
}

void free_codec(struct encoder* encoder)
{
    if(encoder) {
        // 关闭编码器
        avcodec_close(encoder->codec_ctx);

        // 释放 AVCodecContext
        avcodec_free_context(&encoder->codec_ctx);
        encoder->codec_ctx = NULL;
    }
}

int init_output_context(struct encoder* encoder, char* filename,int fps)
{
    int ret = 0;
    ret = avformat_alloc_output_context2(&(encoder->ofmt_ctx), 0, 0, filename);
    if(ret < 0) {
        DB_WAR("avformat_alloc_output_context2 failed!");
        return ret;
    }

    encoder->video_stream = avformat_new_stream(encoder->ofmt_ctx, NULL);
    if (!encoder->video_stream) {
        DB_WAR("avformat_new_stream failed!");
        return -1;
    }
    encoder->video_stream->id = 0;
    encoder->video_stream->codecpar->codec_tag = 0;
    encoder->video_stream->time_base.num = 1;
    encoder->video_stream->time_base.den = fps;
    ret = avcodec_parameters_from_context(encoder->video_stream->codecpar, encoder->codec_ctx);
    if (ret < 0) {
        DB_WAR("avcodec_parameters_from_context failed!");
        avformat_free_context(encoder->ofmt_ctx);
        return -1;
    }

    return 0;
}

void free_output_context(struct encoder* encoder)
{
    if (encoder) {
        // 释放 AVFormatContext
        avformat_free_context(encoder->ofmt_ctx);
    }
}

AVFrame* get_frame(int width,int height)
{
    int ret = 0;
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        DB_WAR("av_frame_alloc failed!");
        return NULL;
    }

    frame->format = AV_PIX_FMT_YUV420P;
    frame->width = width;
    frame->height = height;

    ret = av_frame_get_buffer(frame, 32);
    if (ret < 0) {
        DB_WAR("av_frame_get_buffer failed!");
        av_frame_free(&frame);
        return NULL;
    }

    return frame;
}

void free_frame(AVFrame* frame)
{
    if (frame) {
        av_frame_free(&frame);
    }
}

int write_file_header(AVFormatContext* ofmt_ctx,char* filename)
{
    int ret = 0;
    ret = avio_open(&(ofmt_ctx->pb), filename, AVIO_FLAG_WRITE);
    if (ret < 0) {
        DB_WAR("avio_open failed!");
        return ret;
    }

    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        DB_WAR("avformat_write_header failed!");
        // 关闭输出文件
        avio_closep(&(ofmt_ctx->pb));
        return ret;
    }

    return 0;
}

static int is_time_out(uint64_t sre_timestamp,uint64_t dst_timestamp,int minutes)
{
    int ret = 0;
    int minutes_num = 0;
    uint64_t time = dst_timestamp - sre_timestamp;
    if(time < 0){
        time = -time;
    }
    
    minutes_num = (int)time/(60*1000);
    if(minutes_num >= minutes){
        ret = 1;
    }

    return ret;
}

struct encoder* encoder_open(int width, int height, int fps,int pthreads, int pst_level, char* filename)
{
	int ret = 0;
    struct encoder* encoder = get_encoder();
    if (!encoder) {
        DB_WAR("calloc encoder failed!");
        return NULL;
    }

    av_log_set_level(AV_LOG_QUIET);

    //注册封装
	av_register_all();

	//注册解码器
	avcodec_register_all();

    //获取编码器
    ret = init_codec(encoder, width, height, fps);
    if (ret < 0) {
        DB_WAR("get_codec failed!");
        encoder_close(encoder,(RELEASE_LEVEL)1);
        return NULL;
    }
	
    //获取输出上下文
    ret = init_output_context(encoder, filename, fps);
    if (ret < 0) {
        DB_WAR("get_output_context failed!");
        encoder_close(encoder,(RELEASE_LEVEL)2);
        return NULL;
    }

    encoder->sws_ctx = sws_getCachedContext(encoder->sws_ctx,
                               width, height, AV_PIX_FMT_RGB24,
                               width, height, AV_PIX_FMT_YUV420P,
                               SWS_BICUBIC,
                               NULL, NULL, NULL);
    if (!encoder->sws_ctx) {
        DB_WAR("sws_getCachedContext failed!");
        encoder_close(encoder,(RELEASE_LEVEL)2);
        return NULL;
    }

    //开辟yuv帧
    encoder->yuv_frame = get_frame(width, height);
    if (!encoder->yuv_frame) {
        DB_WAR("get_frame failed!");
        encoder_close(encoder,(RELEASE_LEVEL)3);
        return NULL;
    }

    //开辟上一帧
    encoder->last_frame = get_frame(width, height);
    if (!encoder->last_frame) {
        DB_WAR("get_frame failed!");
        encoder_close(encoder,(RELEASE_LEVEL)4);
        return NULL;
    }

    //写入文件头
    ret = write_file_header(encoder->ofmt_ctx, filename);
    if (ret < 0) {
        DB_WAR("write_file_header failed!");
        encoder->ofmt_ctx = NULL;
        encoder_close(encoder,(RELEASE_LEVEL)5);
        return NULL;
    }

    //初始化包
    av_init_packet(&(encoder->packet));
    
    return encoder;
}

int encoder_encode(struct encoder* encoder, struct bitmap* bitmap)
{
    int ret = 0;
    int frame_nums = 0;
    if (!encoder || !bitmap)
        return -1;
    
    //记录视频起始时间戳
    if(0 == encoder->begin_timestamp){
        encoder->begin_timestamp = bitmap->time;
    }

    //计算当前需要写入多少帧
    if(encoder->timestamp != 0){
        frame_nums = get_frame_nums(encoder->timestamp,bitmap->time,1000/encoder->fps);
        //DB_INFO("frame_nums = %d",frame_nums);
        for(int i = 0; i < frame_nums; i++){
            encoder->last_frame->pts = encoder->pts++;
            ret = write_frame(encoder,0);
            if (ret < 0) {
                DB_WAR("write_frame failed!");
                return -1;
            }
        }
    }

    //数据转换
    encoder->indata[0] = bitmap->buffer;
    encoder->inlinesize[0] = bitmap->resolution->width * 3;
    ret = sws_scale(encoder->sws_ctx, encoder->indata, encoder->inlinesize, 0, bitmap->resolution->height,
            encoder->yuv_frame->data, encoder->yuv_frame->linesize);
    if (ret < 0) {
        DB_WAR("sws_scale failed!");
        return -1;
    }
    encoder->yuv_frame->pts = encoder->pts++;

    //拷贝帧数据
    encoder->timestamp = bitmap->time;
    ret = av_frame_copy(encoder->last_frame,encoder->yuv_frame);
    if (ret < 0) {
        DB_WAR("av_frame_copy failed!");
        return -1;
    }
    
    //写入数据
    ret = write_frame(encoder,1);
    if (ret < 0) {
        DB_WAR("write_frame failed!");
        return -1;
    }

    //判断是否结束编码
    if(1 == is_time_out(encoder->begin_timestamp,bitmap->time,1)){
        DB_INFO("time is over!");
        return 1;
    }
    return 0;
}

int encoder_close(struct encoder* encoder,enum release_level level)  
{
    int ret = 0;
    switch(level){
        case RELEASE_ALL:
            //写文件头
            if(encoder->ofmt_ctx) {
                av_write_trailer(encoder->ofmt_ctx);
            }
            free_codec(encoder);
            DB_INFO("avcodec_free_context ok!");
            free_output_context(encoder);
             DB_INFO("avformat_free_context ok!");
            sws_freeContext(encoder->sws_ctx);
            DB_INFO("sws_freeContext ok!");
            free_frame(encoder->yuv_frame);
            DB_INFO("av_frame_free ok!");
            free_frame(encoder->last_frame);
            av_packet_unref(&(encoder->packet));
            DB_INFO("av_packet_unref ok!");
            free_encoder(encoder);
        break;
        case RELEASE_ENCODER_LEVEL:
            free_encoder(encoder);
            break;
        case RELEASE_CODEC_LEVEL:
            free_codec(encoder);
            free_encoder(encoder);
            break;
        case RELEASE_OUTPUT_LEVEL:
            free_output_context(encoder);
            free_codec(encoder);
            free_encoder(encoder);
            break;
        case RELEASE_SWS_LEVEL:
            free_output_context(encoder);
            free_codec(encoder);
            sws_freeContext(encoder->sws_ctx);
            free_encoder(encoder);
            break;
        case RELEASE_FRAME_LEVEL1:
            free_output_context(encoder);
            free_codec(encoder);
            sws_freeContext(encoder->sws_ctx);
            free_frame(encoder->yuv_frame);
            free_encoder(encoder);
            break;
        case RELEASE_FRAME_LEVEL2:
            free_output_context(encoder);
            free_codec(encoder);
            sws_freeContext(encoder->sws_ctx);
            free_frame(encoder->yuv_frame);
            free_frame(encoder->last_frame);
            free_encoder(encoder);
            break;
        default:
            DB_WAR("release level error!");
            ret = -1;
    }

    return ret;
}

int get_frame_nums(uint64_t src_timestamp,uint64_t dst_timestamp,int millsecond)
{
    // 将时间戳转换为 time_t 类型
    time_t src_time = (time_t)(src_timestamp / 1000);
    time_t dst_time = (time_t)(dst_timestamp / 1000);

    // 计算时间差（秒数）
    time_t diff = dst_time - src_time;

    // 计算时间差对应的毫秒数
    uint64_t diff_ms = (dst_timestamp % 1000) - (src_timestamp % 1000);
    if (diff_ms < 0) {
        diff_ms += 1000;
        diff--;
    }

    // 计算 100 毫秒的个数
    uint64_t count = (diff * 1000 + diff_ms) / millsecond;

    return (int)count;
}
