#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <sys/time.h>
extern "C" {
#	include <x264.h>
}
#include "vencoder.h"

struct Ctx
{
	x264_t *x264;
	x264_picture_t picture;
	x264_param_t param;
	void *output;		// 用于保存编码后的完整帧
	int output_bufsize, output_datasize;
	int64_t pts; 		// 输入 pts
	int64_t (*get_pts)(struct Ctx *);

	int64_t info_pts, info_dts;
	int info_key_frame;
	int info_valid;

	int force_keyframe;
};

// return currsec * 90000
static int64_t curr ()
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	double n = tv.tv_sec + 0.000001*tv.tv_usec;
	n *= 90000.0;
	return (int64_t)n;
}

static int64_t next_pts (struct Ctx *ctx)
{
	int64_t now = curr();
	return now - ctx->pts;
}

static int64_t first_pts (struct Ctx *ctx)
{
	ctx->pts = curr();
	ctx->get_pts = next_pts;
	return 0;
}

x264_param_t *get_x264_param_t(void* ctx)
{
	struct Ctx *tmp = (struct Ctx *)ctx;
	return &tmp->param;
}

x264_t *get_x264_handle(void* ctx)
{
	struct Ctx *tmp = (struct Ctx *)ctx;
	return tmp->x264;
}

void *vencoder_open (int width, int height, double fps)
{
	Ctx *ctx = new Ctx;

	ctx->force_keyframe = 0;

	// 设置编码属性
	//x264_param_default(&ctx->param);
	x264_param_default_preset(&ctx->param, "fast", "zerolatency");
	x264_param_apply_profile(&ctx->param, "baseline");

#if 1
        ctx->param.i_threads = 2;
	ctx->param.i_width = width;
	ctx->param.i_height = height;
	ctx->param.i_fps_num = (int)fps;
	ctx->param.i_fps_den       = 1; /*用两个整型的数的比值，来表示帧率*/
	ctx->param.i_keyint_max = 50;
	ctx->param.i_keyint_min = 25;
	ctx->param.i_level_idc=21;

	ctx->param.b_repeat_headers = 1;  // 重复SPS/PPS 放到关键帧前面
	int m_bitRate = 1024*200;
	ctx->param.rc.i_bitrate = (int)m_bitRate/1000;
	ctx->param.rc.i_vbv_max_bitrate = (int)((m_bitRate*1.2)/1000);
	ctx->param.rc.i_rc_method = X264_RC_ABR; 
	fprintf(stderr, "DBG: X264_RC_CQP = %d\n", X264_RC_CQP);
	fprintf(stderr, "DBG: X264_RC_CRF = %d\n", X264_RC_CRF);
	fprintf(stderr, "DBG: X264_RC_ABR = %d\n", X264_RC_ABR);

	ctx->param.rc.i_qp_constant = 26; //qp的初始值，最大最小的qp值，
#else	
	//CPU 自动检测
	//ctx->param.cpu = x264_cpu_detect();
        //ctx->param.i_threads = 1;
	ctx->param.i_threads = X264_THREADS_AUTO; /* 并行编码线程为0 */
	//ctx->param.b_deterministic = 1; /*允许非确定性时线程优化*/
	//ctx->param.i_sync_lookahead = X264_SYNC_LOOKAHEAD_AUTO; /* 自动选择线程超前缓冲大小-1 */

	//视频属性
	ctx->param.i_csp	= X264_CSP_I420; /*设置输入的视频采样的格式0x0001yuv 4:2:0 planar*/
	ctx->param.i_width = width;
	ctx->param.i_height = height;
	ctx->param.i_fps_num = (int)fps;
	ctx->param.i_fps_den       = 1; /*用两个整型的数的比值，来表示帧率*/
	ctx->param.i_level_idc     = -1; /* level值的设置*/
	ctx->param.i_slice_max_size = 0; /* 每片字节的最大数，包括预计的NAL开销. */
	ctx->param.i_slice_max_mbs = 0; /* 每片宏块的最大数，重写 i_slice_count */
	ctx->param.i_slice_count = 0; /* 每帧的像条数目: 设置矩形像条. */

	//编码属性
	ctx->param.b_repeat_headers = 1;  // 重复SPS/PPS 放到关键帧前面
	ctx->param.b_cabac = 1;
	ctx->param.i_frame_reference = 3; /*参考帧的最大帧数。*/
	ctx->param.i_keyint_max = ctx->param.i_fps_num;
	//ctx->param.i_keyint_max = ctx->param.i_fps_num*100; /* 在此间隔设置IDR关键帧 */
	ctx->param.i_keyint_min = 1;
	//ctx->param.i_keyint_min = ctx->param.i_fps_num; /* 场景切换少于次值编码位I, 而不是 IDR. */
	ctx->param.i_bframe = 3; /*两个参考帧之间的B帧数目*/
	ctx->param.i_scenecut_threshold = 40; /*如何积极地插入额外的I帧 */
	ctx->param.i_bframe_adaptive = X264_B_ADAPT_FAST; /*自适应B帧判定1*/
	ctx->param.i_bframe_bias = 0; /*控制插入B帧判定，范围-100~+100，越高越容易插入B帧*/
	//ctx->param.b_bframe_pyramid = 0; /*允许部分B为参考帧 */
	ctx->param.b_deblocking_filter = 1; /*去块效应相关*/
	ctx->param.i_deblocking_filter_alphac0 = 0; /* [-6, 6] -6 亮度滤波器, 6 强 */
	ctx->param.i_deblocking_filter_beta = 0; /* [-6, 6]  同上 */
	ctx->param.b_cabac = 1; /*cabac的开关*/
	ctx->param.i_cabac_init_idc = 0;
	// ctx->param.b_intra_refresh = 1;
	//ctx->param.b_vfr_input = 1;

	// rc
	// ctx->param.rc.i_rc_method = X264_RC_CRF;
	ctx->param.rc.i_bitrate = 150;
	//ctx->param.rc.f_rate_tolerance = 0.1;
	//ctx->param.rc.i_vbv_max_bitrate = ctx->param.rc.i_bitrate * 1.3;
	//ctx->param.rc.f_rf_constant = 600;
	//ctx->param.rc.f_rf_constant_max = ctx->param.rc.f_rf_constant * 1.3;
	/*码率控制*/
	ctx->param.rc.i_rc_method = X264_RC_CRF;;/*恒定码率*/
	ctx->param.rc.i_bitrate = 0;/*设置平均码率大小*/
	ctx->param.rc.f_rate_tolerance = 1.0;
	ctx->param.rc.i_vbv_max_bitrate = 0; /*平均码率模式下，最大瞬时码率，默认0(与-B设置相同) */
	ctx->param.rc.i_vbv_buffer_size = 0; /*码率控制缓冲区的大小，单位kbit，默认0 */
	ctx->param.rc.f_vbv_buffer_init = 0.9; /* <=1: fraction of buffer_size. >1: kbit码率控制缓冲区数据保留的最大数据量与缓冲区大小之比，范围0~1.0，默认0.9*/

	ctx->param.rc.i_qp_constant = 23;;/*最小qp值*/
	ctx->param.rc.f_rf_constant = 23;
	ctx->param.rc.i_qp_min = 10; /*允许的最小量化值 */
	ctx->param.rc.i_qp_max = 51; /*允许的最大量化值*/
	ctx->param.rc.i_qp_step = 4; /*帧间最大量化步长 */
	ctx->param.rc.f_ip_factor = 1.4;
	ctx->param.rc.f_pb_factor = 1.3;
	ctx->param.rc.i_aq_mode = X264_AQ_VARIANCE; /* psy adaptive QP. (X264_AQ_*) */
	ctx->param.rc.f_aq_strength = 1.0;
	ctx->param.rc.i_lookahead = 40;
	ctx->param.rc.b_stat_write = 0; /* Enable stat writing in psz_stat_out */
	ctx->param.rc.psz_stat_out = "x264_2pass.log";
	ctx->param.rc.b_stat_read = 0;
	ctx->param.rc.psz_stat_in = "x264_2pass.log";
	ctx->param.rc.f_qcompress = 0.6; /* 0.0 => cbr, 1.0 => constant qp */  
	ctx->param.rc.f_qblur = 0.5;   /*时间上模糊量化 */
	ctx->param.rc.f_complexity_blur = 20; /* 时间上模糊复杂性 */
	ctx->param.rc.i_zones = 0; /* number of zone_t's */
	ctx->param.rc.b_mb_tree = 1; /* Macroblock-tree ratecontrol. */

	/*分析 */

	ctx->param.analyse.intra = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8;

	ctx->param.analyse.inter = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8

		         | X264_ANALYSE_PSUB16x16 | X264_ANALYSE_BSUB16x16;

	ctx->param.analyse.i_direct_mv_pred = X264_DIRECT_PRED_SPATIAL;/*空间预测模式*/

	ctx->param.analyse.i_me_method = X264_ME_HEX;/*运动估计算法HEX*/

	ctx->param.analyse.f_psy_rd = 1.0;

	ctx->param.analyse.b_psy = 1;

	ctx->param.analyse.f_psy_trellis = 0;

	ctx->param.analyse.i_me_range = 16;/*运动估计范围*/

	ctx->param.analyse.i_subpel_refine = 7; /* 亚像素运动估计质量 */

	ctx->param.analyse.b_mixed_references = 1; /*允许每个宏块的分区在P帧有它自己的参考号*/

	ctx->param.analyse.b_chroma_me = 1; /* 亚像素色度运动估计和P帧的模式选择 */

	ctx->param.analyse.i_mv_range_thread = -1; /* 线程之间的最小空间. -1 = auto, based on number of threads. */

	ctx->param.analyse.i_mv_range = -1; /*运动矢量最大长度set from level_idc*/

	ctx->param.analyse.i_chroma_qp_offset = 0; /*色度量化步长偏移量 */

	ctx->param.analyse.b_fast_pskip = 1; /*快速P帧跳过检测*/

	ctx->param.analyse.b_weighted_bipred = 1; /*为b帧隐式加权 */

	ctx->param.analyse.b_dct_decimate = 1; /* 在P-frames转换参数域 */

	ctx->param.analyse.b_transform_8x8 = 1; /* 帧间分区*/

	ctx->param.analyse.i_trellis = 1; /* Trellis量化，对每个8x8的块寻找合适的量化值，需要CABAC，默认0 0：关闭1：只在最后编码时使用2：一直使用*/

	ctx->param.analyse.i_luma_deadzone[0] = 21; /*帧间亮度量化中使用的无效区大小*/

	ctx->param.analyse.i_luma_deadzone[1] = 11; /*帧内亮度量化中使用的无效区大小*/

	ctx->param.analyse.b_psnr = 0;/*是否显示PSNR*/

	ctx->param.analyse.b_ssim = 0;/*是否显示SSIM*/



	/*量化*/

	ctx->param.i_cqm_preset = X264_CQM_FLAT; /*自定义量化矩阵(CQM),初始化量化模式为flat 0*/

	memset( ctx->param.cqm_4iy, 16, 16 );

	memset( ctx->param.cqm_4ic, 16, 16 );

	memset( ctx->param.cqm_4py, 16, 16 );

	memset( ctx->param.cqm_4pc, 16, 16 );

	memset( ctx->param.cqm_8iy, 16, 64 );

	memset( ctx->param.cqm_8py, 16, 64 );/*开辟空间*/

	/*muxing*/

	ctx->param.b_repeat_headers = 1; /* 在每个关键帧前放置SPS/PPS*/

	ctx->param.b_aud = 0; /*生成访问单元分隔符*/
#endif

#ifdef DEBUG
	ctx->param.i_log_level = X264_LOG_WARNING;
#else
	ctx->param.i_log_level = X264_LOG_NONE;
#endif // release

	ctx->x264 = x264_encoder_open(&ctx->param);
	if (!ctx->x264) {
		fprintf(stderr, "%s: x264_encoder_open err\n", __func__);
		delete ctx;
		return 0;
	}

	x264_picture_init(&ctx->picture);
	ctx->picture.img.i_csp = X264_CSP_I420;
	ctx->picture.img.i_plane = 3;

	ctx->output = malloc(128*1024);
	ctx->output_bufsize = 128*1024;
	ctx->output_datasize = 0;

	ctx->get_pts = first_pts;
	ctx->info_valid = 0;

	return ctx;
}

int vencoder_close (void *ctx)
{
	Ctx *c = (Ctx*)ctx;
	x264_encoder_close(c->x264);
	free(c->output);
	delete c;
	return 1;
}

int get_last_frame_info (void *ctx, int *key_frame, int64_t *pts, int64_t *dts)
{
	Ctx *c = (Ctx *)ctx;
	if (c->info_valid) {
		*key_frame = c->info_key_frame;
		*pts = c->info_pts;
		*dts = c->info_dts;
		return 1;
	}
	else {
		return -1;
	}
}

static int encode_nals (Ctx *c, x264_nal_t *nals, int nal_cnt)
{
	char *pout = (char*)c->output;
	c->output_datasize = 0;
	for (int i = 0; i < nal_cnt; i++) {
		if (c->output_datasize + nals[i].i_payload > c->output_bufsize) {
			// 扩展
			c->output_bufsize = (c->output_datasize+nals[i].i_payload+4095)/4096*4096;
			c->output = realloc(c->output, c->output_bufsize);
		}
		memcpy(pout+c->output_datasize, nals[i].p_payload, nals[i].i_payload);
		c->output_datasize += nals[i].i_payload;
	}

	return c->output_datasize;
}

static void dumpnal (x264_nal_t *nal)
{
	// 打印前面 10 个字节
	for (int i = 0; i < nal->i_payload && i < 20; i++) {
		fprintf(stderr, "%02x ", (unsigned char)nal->p_payload[i]);
	}
}

static void dumpnals (int type, x264_nal_t *nal, int nals)
{
	fprintf(stderr, "======= dump nals %d type=%d, ======\n", nals, type);
	for (int i = 0; i < nals; i++) {
		fprintf(stderr, "\t nal  #%d: %dbytes, nal type=%d ", i, nal[i].i_payload, nal[i].i_type);
		dumpnal(&nal[i]);
		fprintf(stderr, "\n");
	}
}

extern int send_video_sps_pps(unsigned char *sps, int sps_len, unsigned char *pps, int pps_len);
extern int send_rtmp_video(unsigned char * buf,int len);
int vencoder (void *ctx, unsigned char *data[4], int stride[4], const void **out, int *len)
{
	Ctx *c = (Ctx*)ctx;
	
	// 设置 picture 数据
	for (int i = 0; i < 4; i++) {
		c->picture.img.plane[i] = data[i];
		c->picture.img.i_stride[i] = stride[i];
	}

	// encode
	x264_nal_t *nals;
	int nal_cnt;
	x264_picture_t pic_out;
	
	c->picture.i_pts = c->get_pts(c);

	x264_picture_t *pic = &c->picture;

	if (c->force_keyframe) {
		c->picture.i_type = X264_TYPE_IDR;
		c->force_keyframe = 0;
	}
	else {
		c->picture.i_type = X264_TYPE_AUTO;
	}

	do {
		// 这里努力消耗掉 delayed frames ???
		// 实际使用 zerolatency preset 时, 效果足够好了
		int rc_size = x264_encoder_encode(c->x264, &nals, &nal_cnt, pic, &pic_out);
		if (rc_size < 0) return -1;
		#if 0
		if (pic_out.b_keyframe) {
//			dumpnals(pic_out.i_type, nals, nal_cnt);
		}
		encode_nals(c, nals, nal_cnt);
		#endif

		//////////////////test by wyq
		int i,last;
		unsigned char sps[1024];
		unsigned char pps[1024];
		int sps_len = 0, pps_len = 0;
		for (i = 0,last = 0;i < nal_cnt;i++)
		{
			if (nals[i].i_type == NAL_SPS)
			{

				sps_len = nals[i].i_payload-4;
				memcpy(sps,nals[i].p_payload+4,sps_len);
				fprintf(stderr, "DBG: sps len = %d\n", sps_len);

			}
			else if (nals[i].i_type == NAL_PPS)
			{

				pps_len = nals[i].i_payload-4;
				memcpy(pps,nals[i].p_payload+4,pps_len);
				fprintf(stderr, "DBG: pps len = %d\n", pps_len);

				/*发送sps pps*/
				send_video_sps_pps(sps, sps_len, pps, pps_len);    

			} else
			{

				/*发送普通帧(剩下全部)*/
				send_rtmp_video(nals[i].p_payload, rc_size-last);
				//if(sps_len != 0 && pps_len != 0)
				{
					fprintf(stderr, "DBG: send_rtmp_video = (rc_size : %d, sps_len : %d, pps_len : %d, last_size : %d)\n",
						rc_size, sps_len, pps_len, rc_size-last);
				}
				break;
			}
			last += nals[i].i_payload;
		}		
		//////////////////end test
	} while (0);

	*out = c->output;
	*len = c->output_datasize;

	if (nal_cnt > 0) {
		c->info_valid = 1;
		c->info_key_frame = pic_out.b_keyframe;
		c->info_pts = pic_out.i_pts;
		c->info_dts = pic_out.i_dts;
	}
	else {
		fprintf(stderr, ".");
		return 0; // 继续
	}

#ifdef DEBUG_MORE
	static int64_t _last_pts = c->info_pts;

	fprintf(stderr, "DBG: pts delta = %lld\n", c->info_pts - _last_pts);
	_last_pts = c->info_pts;
#endif //


#ifdef DEBUG_MORE
	static size_t _seq = 0;

	fprintf(stderr, "#%lu: [%c] frame type=%d, size=%d\n", _seq, 
			pic_out.b_keyframe ? '*' : '.', 
			pic_out.i_type, c->output_datasize);

	_seq++;
#endif // debug

	return 1;
}

int ve_force_keyframe (void *ctx)
{
	Ctx *c = (Ctx*)ctx;
	c->force_keyframe = 1;
	return 1;
}

