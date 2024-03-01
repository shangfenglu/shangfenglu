#include "config.h"
#include "ini.h"

void g_rgbtrans_init(struct ini_t* cfg_ini, struct xconfig* cfg)
{
    //初始化值
    cfg->ffmpeg_threads = 1;
    
    //读取配置
    ini_sget(cfg_ini, "thread", "ffmpeg_threads", "%d",&cfg->ffmpeg_threads);
	ini_sget(cfg_ini, "frame", "frame_rate", "%d",&cfg->fps);

    //释放资源
    ini_free(cfg_ini);
}



#if 0
static struct xconfig config;
#define filename "conf/cfg.ini"

int main()
{
	struct ini_t* cfg_ini = NULL;
	if ((cfg_ini = ini_load(filename)) == NULL) {
		printf("ini_load failed\n");
	}

	g_rgbtrans_init(cfg_ini, &config);
	printf("config.ffmpeg_threads:%d\n", config.ffmpeg_threads);


	printf("encoder susscess\n");

	return 0;
}


#endif