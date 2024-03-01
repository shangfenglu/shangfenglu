#ifndef __FRAMETRANS_
#define __FRAMETRANS_

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/log.h>

typedef enum release_level{
	RELEASE_ENCODER_LEVEL = 0,
	RELEASE_CODEC_LEVEL = 1,
	RELEASE_OUTPUT_LEVEL = 2,
	RELEASE_SWS_LEVEL = 3,
	RELEASE_FRAME_LEVEL1 = 4,
	RELEASE_FRAME_LEVEL2 = 5,
	RELEASE_ALL = 6
}RELEASE_LEVEL;

struct bitmap;
struct encoder {
	AVCodec*         codec;            // 编码器
	AVCodecContext*  codec_ctx;        // 编码容器
	AVFormatContext* ofmt_ctx;         // 输出容器
	AVStream*        video_stream;     // 数据流
	struct SwsContext* sws_ctx;        // 转码容器
	AVFrame*         yuv_frame;        // yuv帧容器
	AVPacket         packet;           // 数据包
	uint8_t*         indata[AV_NUM_DATA_POINTERS]; // 输入数据
	int              inlinesize[AV_NUM_DATA_POINTERS]; // 输入数据长度
	int64_t          pts;				// 时间戳
	int     		 pthreads;          // 线程数
	AVFrame*         last_frame;        // 上一帧容器
	uint64_t         timestamp;		    // 时间戳
	uint64_t         begin_timestamp;   // 开始时间戳
	int              fps;               // 帧率
};

//打开编码器
struct encoder* encoder_open(int width, int height, int fps,int pthreads, int pst_level, char* filename);

//开始编码
int encoder_encode(struct encoder* encoder,struct bitmap* bitmap);

//关闭编码器
int encoder_close(struct encoder* encoder,enum release_level level);

//返回补帧数量
static int get_frame_nums(uint64_t src_timestamp,uint64_t dst_timestamp,int millsecond);

//获取编码器结构体
static struct encoder* get_encoder();

//释放编码器结构体
static void free_encoder(struct encoder* encoder);

//初始化编码器
static int init_codec(struct encoder* encoder,int width, int height, int fps);

//释放编码器
static void free_codec(struct encoder* encoder);

//获取输出上下文
static int init_output_context(struct encoder* encoder, char* filename,int fps);

//释放输出上下文
static void free_output_context(struct encoder* encoder);

//获取一帧
static AVFrame* get_frame(int width,int height);

//释放一帧
static void free_frame(AVFrame* frame);

//写入文件头
static int write_file_header(AVFormatContext* ofmt_ctx,char* filename);

#endif
